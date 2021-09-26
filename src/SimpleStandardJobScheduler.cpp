/**
 * Copyright (c) 2020. <ADD YOUR HEADER INFORMATION>.
 * Generated with the wrench-init.in tool.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
#include "SimpleStandardJobScheduler.h"

#include <utility>
#include <algorithm>

XBT_LOG_NEW_DEFAULT_CATEGORY(simple_scheduler, "Log category for Simple Scheduler");

SimpleStandardJobScheduler::SimpleStandardJobScheduler() {

    /** Setting/Defining the task priority scheme **/
    this->task_priority_schemes["max_flops"] = [](const wrench::WorkflowTask *a,
                                                      const wrench::WorkflowTask *b) -> bool {
        if (a->getFlops() < b->getFlops()) {
            return true;
        } else if (a->getFlops() > b->getFlops()) {
            return false;
        } else {
            return ((unsigned long) a < (unsigned long) b);
        }
    };

    /** Setting/Defining the service selection scheme **/
    this->service_selection_schemes["fastest_cores"] = [] (const wrench::WorkflowTask* task, const std::set<std::shared_ptr<wrench::BareMetalComputeService>> services) -> std::shared_ptr<wrench::BareMetalComputeService> {
        std::shared_ptr<wrench::BareMetalComputeService> picked = nullptr;
        for (auto const &s : services) {
            if ((picked == nullptr) or (s->getCoreFlopRate().begin()->second > picked->getCoreFlopRate().begin()->second)) {
                picked = s;
            }
        }
        return picked;
    };

    /** Setting/Defining the core selection scheme **/
    this->core_selection_schemes["minimum"] = [] (const wrench::WorkflowTask* task, const std::shared_ptr<wrench::BareMetalComputeService> service) -> int {
        return (int)(task->getMinNumCores());
    };


}


void SimpleStandardJobScheduler::init(
        std::shared_ptr<wrench::JobManager> _job_manager,
        std::set<std::shared_ptr<wrench::BareMetalComputeService>> _compute_services,
        std::set<std::shared_ptr<wrench::StorageService>> _storage_services,
        std::shared_ptr<wrench::FileRegistryService> _file_registry_service,
        std::string wms_host) {
    this->job_manager = std::move(_job_manager);
    this->storage_services = std::move(_storage_services);
    this->compute_services = std::move(_compute_services);
    this->file_registry_service = std::move(_file_registry_service);
    this->wms_host = wms_host;

}

std::shared_ptr<wrench::FileLocation>  SimpleStandardJobScheduler::pick_location(
        const std::shared_ptr<wrench::BareMetalComputeService>& compute_service,
        wrench::WorkflowFile *file) {

    auto entries = this->file_registry_service->lookupEntry(file);
    if (entries.empty()) {
        throw std::runtime_error("FILE " + file->getID() + " is nowhere to be found!");
    }

    std::shared_ptr<wrench::FileLocation> picked_local_location = nullptr;
    std::shared_ptr<wrench::FileLocation> picked_wms_location = nullptr;
    std::shared_ptr<wrench::FileLocation> picked_other_location = nullptr;

    // Pick the WMS host if file is there
    for (auto const &location : entries) {
        if (location->getStorageService()->getHostname() == compute_service->getHostname()) {
            picked_local_location = location;
            continue;
        }
        if (location->getStorageService()->getHostname() == this->wms_host) {
            picked_wms_location = location;
            continue;
        }
        if (picked_other_location == nullptr) {
            picked_other_location = location;
            continue;
        }
    }

    if (picked_local_location) return picked_local_location;
    if (picked_wms_location) return picked_wms_location;
    if (picked_other_location) return picked_other_location;

    throw std::runtime_error("A location should have been returned");

}

bool SimpleStandardJobScheduler::taskCanRunOn(wrench::WorkflowTask *task, const std::shared_ptr<wrench::BareMetalComputeService> service) {


    auto idle_cores = service->getPerHostNumIdleCores();
    for (auto const &spec : idle_cores) {
        if (spec.second >= task->getMinNumCores()) {
            return true;
        }
    }
    return false;
}

void SimpleStandardJobScheduler::prioritizeTasks(std::vector<wrench::WorkflowTask *> &tasks) {

    std::sort(tasks.begin(), tasks.end(),
              this->task_priority_schemes[std::get<0>(this->scheduling_algorithms.at(this->current_scheduler))]);

}

/** Returns true if found something **/
bool SimpleStandardJobScheduler::scheduleTask(wrench::WorkflowTask *task,
                                              std::shared_ptr<wrench::BareMetalComputeService> *picked_service,
                                              int *picked_num_cores) {


    // Weed out impossible services
    std::set<std::shared_ptr<wrench::BareMetalComputeService>>  possible_services;
    for (auto const &s : this->compute_services) {
        if (this->taskCanRunOn(task, s)) {
            possible_services.insert(s);
        }
    }

    if (possible_services.empty()) {
        *picked_service = nullptr;
        *picked_num_cores = 0;
        return false;
    }

    *picked_service = this->service_selection_schemes[std::get<1>(this->scheduling_algorithms.at(this->current_scheduler))](task, possible_services);
    *picked_num_cores = this->core_selection_schemes[std::get<2>(this->scheduling_algorithms.at(this->current_scheduler))](task, *picked_service);

    return true;
}

void SimpleStandardJobScheduler::scheduleTasks(std::vector<wrench::WorkflowTask *> tasks) {

    prioritizeTasks(tasks);

    for (auto task : tasks) {

        WRENCH_INFO("Scheduling ready task %s", task->getID().c_str());
        std::shared_ptr<wrench::BareMetalComputeService> picked_service;
        int picked_num_cores;

        if (not scheduleTask(task, &picked_service, &picked_num_cores)) {
            WRENCH_INFO("Wasn't able to schedule task %s", task->getID().c_str());
            continue;
        }

        WRENCH_INFO("Submitting task %s for execution", task->getID().c_str());

        // Submitting the task as a simple job
        std::map<wrench::WorkflowFile *, std::shared_ptr<wrench::FileLocation>> file_locations;

        // Input files are read from the "best" location
        for (auto file : task->getInputFiles()) {
            // Pick a location
            std::shared_ptr<wrench::FileLocation> picked_location = pick_location(picked_service, file);
            file_locations.insert(std::make_pair(file, picked_location));
        }

        // Output file are all written locally
        std::shared_ptr<wrench::StorageService> target_ss = nullptr;
        for (const auto &ss : this->storage_services) {
            if (ss->getHostname() == picked_service->getHostname()) {
                target_ss = ss;
                break;
            }
        }
        if (target_ss == nullptr) {
            throw std::runtime_error("Couldn't find SS associated to CS on host " + picked_service->getHostname());
        }

        for (auto f : task->getOutputFiles()) {
            file_locations.insert(std::make_pair(f, wrench::FileLocation::LOCATION(target_ss)));
        }

        auto job = this->job_manager->createStandardJob(task, file_locations);
        this->job_manager->submitJob(job, picked_service, {});

    }
    WRENCH_INFO("Done with scheduling tasks as standard jobs");
}

void SimpleStandardJobScheduler::addSchedulingAlgorithm(std::string spec) {

    stringstream ss(spec);
    std::vector<std::string> tokens;
    string item;
    while (getline(ss, item, ':')) {
        tokens.push_back (item);
    }
    if (tokens.size() != 3) {
        throw std::invalid_argument("Invalid scheduler specification '" + spec + "'");
    }
    this->scheduling_algorithms.push_back(std::make_tuple(tokens.at(0), tokens.at(1), tokens.at(2)));

}


