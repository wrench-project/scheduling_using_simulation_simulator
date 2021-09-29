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
                     double simulation_noise,
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
                                                    speculative_work_fraction(speculative_work_fraction),
                                                    simulation_noise(simulation_noise)
                                                    {
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
        if (((not this->i_am_speculative) and (this->scheduler->getEnabledSchedulingAlgorithms().size() > 1)) and
            (((not this->one_schedule_change_has_happened) and (this->work_done_since_last_scheduler_change > this->first_scheduler_change_trigger * total_work)) or
             (this->one_schedule_change_has_happened and (this->work_done_since_last_scheduler_change > this->periodic_scheduler_change_trigger * total_work)))) {

            this->one_schedule_change_has_happened = true;
            this->work_done_since_last_scheduler_change = 0.0;
            std::cerr << "Exploring scheduling algorithm futures speculatively... ";
            std::cerr.flush();
            std::vector<double> makespans;
            for (auto const &algorithm_index : this->scheduler->getEnabledSchedulingAlgorithms()) {
                pipe(pipefd);
                auto pid = fork();
                if (!pid) {
                    // Make the child mute
                    close(STDOUT_FILENO);
                    // Close the read end of the pipe
                    close(pipefd[0]);
                    this->scheduler->useSchedulingAlgorithm(algorithm_index);
                    this->i_am_speculative = true;
                    break;
                } else {
                    // Parent
                    close(pipefd[1]);
                    int stat_loc;
                    double child_time;
                    read(pipefd[0], &child_time, sizeof(double));
                    makespans.push_back(child_time);
                    waitpid(pid, &stat_loc, 0);
                }
            }
            if (not this->i_am_speculative) {
                auto argmin = std::min_element(makespans.begin(), makespans.end()) - makespans.begin();
                unsigned long algorithm_index = this->scheduler->getEnabledSchedulingAlgorithms().at(argmin);

                std::cerr << "Switching to algorithm " <<
                          "[" << (algorithm_index < 100 ? "0" : "") << (algorithm_index < 10 ? "0" : "") << algorithm_index << "] " <<
                          this->scheduler->schedulingAlgorithmToString(this->scheduler->getEnabledSchedulingAlgorithms().at(argmin)) << "\n";

                this->scheduler->useSchedulingAlgorithm(argmin);
            }
        }

        if ((this->i_am_speculative) and (this->work_done_since_last_scheduler_change > this->speculative_work_fraction * total_work)) {
            break;
        }

        // Get the ready tasks
        std::vector<wrench::WorkflowTask *> ready_tasks = this->getWorkflow()->getReadyTasks();

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
        // Get current time
        double now = wrench::Simulation::getCurrentSimulatedDate();
        // Add noise
        std::uniform_real_distribution<double> random_dist(-simulation_noise, simulation_noise);
        std::mt19937 rng(getpid());
        now = now + now * random_dist(rng);
        // Send it back to the parent
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