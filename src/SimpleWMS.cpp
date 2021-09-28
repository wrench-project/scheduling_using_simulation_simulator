/**
* Copyright (c) 2020. <ADD YOUR HEADER INFORMATION>.
* Generated with the wrench-init.in tool.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*/
#include <iostream>
#include <sys/wait.h>

#include "SimpleWMS.h"
#include "SimpleStandardJobScheduler.h"

XBT_LOG_NEW_DEFAULT_CATEGORY(simple_wms, "Log category for Simple WMS");

/**
* @brief Create a Simple WMS with a workflow instance, a scheduler implementation, and a list of compute services
*/
SimpleWMS::SimpleWMS(SimpleStandardJobScheduler *scheduler,
                     double first_scheduler_change_trigger,
                     double periodic_scheduler_change_trigger,
                     double speculative_work_fraction,
                     const std::set<std::shared_ptr<wrench::ComputeService>> &compute_services,
                     const std::set<std::shared_ptr<wrench::StorageService>> &storage_services,
                     const std::shared_ptr<wrench::FileRegistryService> &file_registry_service,
                     const std::string &hostname) : wrench::WMS(
        nullptr,
        nullptr,
        compute_services,
        storage_services,
        {}, file_registry_service,
        hostname,
        "simple"),
                                                    scheduler(scheduler),
                                                    first_scheduler_change_trigger(first_scheduler_change_trigger),
                                                    periodic_scheduler_change_trigger(periodic_scheduler_change_trigger),
                                                    speculative_work_fraction(speculative_work_fraction) {
}

/**
* @brief main method of the SimpleWMS daemon
*/
int SimpleWMS::main() {

    wrench::TerminalOutput::setThisProcessLoggingColor(wrench::TerminalOutput::COLOR_GREEN);

// Check whether the WMS has a deferred start time
    checkDeferredStart();

    WRENCH_INFO("About to execute a workflow with %lu tasks", this->getWorkflow()->getNumberOfTasks());

// Create a job manager
    this->job_manager = this->createJobManager();
    this->data_movement_manager = this->createDataMovementManager();
    this->file_registry_service = this->getAvailableFileRegistryService();

// Initialize the scheduler

    this->scheduler->init(this->job_manager,
                          this->getAvailableComputeServices<wrench::BareMetalComputeService>(),
                          this->getAvailableStorageServices(),
                          this->file_registry_service,
                          wrench::Simulation::getHostName());

// Compute the total work
    double total_work = 0.0;
    for (auto const &t : this->getWorkflow()->getTasks()) {
        total_work += t->getFlops();
    }

// Pipe used for communication with child
    int pipefd[2];

    while (true) {

        // Scheduler change?
        if (((not this->i_am_speculative) and (this->scheduler->getNumSchedulingAlgorithms() > 1)) and
            (((not this->one_schedule_change_has_happened) and (this->work_done_since_last_scheduler_change > this->first_scheduler_change_trigger * total_work)) or
             (this->one_schedule_change_has_happened and (this->work_done_since_last_scheduler_change > this->periodic_scheduler_change_trigger * total_work)))) {
            this->one_schedule_change_has_happened = true;
            this->work_done_since_last_scheduler_change = 0.0;
            std::cerr << "Exploring scheduling algorithm futures speculatively... ";
            std::cerr.flush();
//            std::cerr << "SHOULD BE LOOKING AT RESCHEDULING!\n";
            std::vector<double> makespans;
            for (int i=0; i < this->scheduler->getNumSchedulingAlgorithms(); i++) {
                pipe(pipefd);
//                std::cerr << "STARTING A CHILD TO LOOK AT ALGORITHM " << i << "\n";
                auto pid = fork();
                if (!pid) {
                    // Child
                    // Make the child mute
                    close(STDOUT_FILENO);
                    // Close the read end of the pipe
                    close(pipefd[0]);
//                    std::cerr << "   I AM A CHILD DOING ALGO " << i << "\n";
                    this->scheduler->setSchedulingAlgorithm(i);
                    this->i_am_speculative = true;
                    break;
                } else {
                    // Parent
                    close(pipefd[1]);
                    int stat_loc;
                    double child_time;
                    read(pipefd[0], &child_time, sizeof(double));
//                    std::cerr << "CHILD THAT LOOKED AT ALGO " << i << " GAVE MAKESPAN: " << child_time << "\n";
                    makespans.push_back(child_time);
                    waitpid(pid, &stat_loc, 0);
//                    std::cerr << "CHILD EXITED: " << WEXITSTATUS(stat_loc) << "\n";
//                    break;
                }
            }
            if (not this->i_am_speculative) {
                int minElementIndex = std::min_element(makespans.begin(), makespans.end()) - makespans.begin();
                std::cerr << "Switching to algorithm " << minElementIndex << "\n";
                this->scheduler->setSchedulingAlgorithm(minElementIndex);
            }
        }

        if (this->work_done_since_last_scheduler_change > this->speculative_work_fraction * total_work) {
            break;
        }

        // Get the ready tasks
        std::vector<wrench::WorkflowTask *> ready_tasks = this->getWorkflow()->getReadyTasks();

        // Get the available compute services
        auto compute_services = this->getAvailableComputeServices<wrench::ComputeService>();

        if (compute_services.empty()) {
            WRENCH_INFO("Aborting - No compute services available!");
            break;
        }

        // Run ready tasks with defined scheduler implementation
        this->scheduler->scheduleTasks(ready_tasks);

        // Wait for a workflow execution event, and process it
        try {
            this->waitForAndProcessNextEvent();
        } catch (wrench::WorkflowExecutionException &e) {
            WRENCH_INFO("Error while getting next execution event (%s)... ignoring and trying again",
                        (e.getCause()->toString().c_str()));
            continue;
        }

        if (this->getWorkflow()->isDone()) {
            break;
        }
    }

    if (this->i_am_speculative) {
        double now = wrench::Simulation::getCurrentSimulatedDate();
        write(pipefd[1], &now, sizeof(double));
        close(pipefd[1]);
//        std::cerr << "   CHILD RETURNING TO MAIN AFTER SENDING MAKESPAN " << now << " TO PARENT\n";
    }
    this->job_manager.reset();

    return 0;
}

void SimpleWMS::processEventStandardJobCompletion(std::shared_ptr<wrench::StandardJobCompletedEvent> event) {
    auto task = event->standard_job->getTasks().at(0);
    this->work_done_since_last_scheduler_change += task->getFlops();
    auto created_files = task->getOutputFiles();
    auto cs = event->compute_service;
    std::shared_ptr<wrench::StorageService> target_ss;
    for (auto const &ss : this->getAvailableStorageServices()) {
        if (ss->getHostname() == cs->getHostname()) {
            target_ss = ss;
            break;
        }
    }
    for (auto const &f : created_files) {
        this->file_registry_service->addEntry(f, wrench::FileLocation::LOCATION(target_ss));
    }
}

void SimpleWMS::processEventStandardJobFailure(std::shared_ptr<wrench::StandardJobFailedEvent> event) {
    throw std::runtime_error("Job Failure shouldn't happen!");
}