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

#include "SimpleWMS.h"
#include "SimpleStandardJobScheduler.h"

XBT_LOG_NEW_DEFAULT_CATEGORY(simple_wms, "Log category for Simple WMS");

/**
 * @brief Create a Simple WMS with a workflow instance, a scheduler implementation, and a list of compute services
 */
SimpleWMS::SimpleWMS(const std::set<std::shared_ptr<wrench::ComputeService>> &compute_services,
                     const std::set<std::shared_ptr<wrench::StorageService>> &storage_services,
                     const std::shared_ptr<wrench::FileRegistryService> &file_registry_service,
                     const std::string &hostname) : wrench::WMS(
        nullptr,
        nullptr,
        compute_services,
        storage_services,
        {}, file_registry_service,
        hostname,
        "simple") {}

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
    std::cerr << "___> " << this->file_registry_service << "\n";

    // Initialize the scheduler
    auto my_scheduler = new SimpleStandardJobScheduler();

    my_scheduler->init(this->job_manager,
                       this->getAvailableComputeServices<wrench::BareMetalComputeService>(),
                       this->getAvailableStorageServices(),
                       this->file_registry_service,
                       wrench::Simulation::getHostName());

    while (true) {
        // Get the ready tasks
        std::vector<wrench::WorkflowTask *> ready_tasks = this->getWorkflow()->getReadyTasks();

        // Get the available compute services
        auto compute_services = this->getAvailableComputeServices<wrench::ComputeService>();

        if (compute_services.empty()) {
            WRENCH_INFO("Aborting - No compute services available!");
            break;
        }

        // Run ready tasks with defined scheduler implementation
        my_scheduler->scheduleTasks(ready_tasks);

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

    wrench::Simulation::sleep(10);

    this->job_manager.reset();

    return 0;
}

void SimpleWMS::processEventStandardJobCompletion(std::shared_ptr<wrench::StandardJobCompletedEvent> event) {
    auto task = event->standard_job->getTasks().at(0);
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
    return;
}