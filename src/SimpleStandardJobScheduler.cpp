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

    this->initTaskPrioritySchemes();
    this->initServiceSelectionSchemes();
    this->initCoreSelectionSchemes();

    unsigned int index = 0;
    // Create all combination algorithms
    for (auto const &e : this->task_priority_schemes) {
        if (e.first == "random") continue;
        for (auto const &f : this->service_selection_schemes) {
            if (f.first == "random") continue;
            for (auto const &g : this->core_selection_schemes) {
                if (g.first == "random") continue;
                this->scheduling_algorithms_index_to_tuple[index] =  std::make_tuple(e.first, f.first, g.first);
                index++;
            }
        }
    }
    // Add the random:random:random one
    this->scheduling_algorithms_index_to_tuple[index] = std::make_tuple("random", "random", "random");

    // Create the random dist / rng for the random:random:random
    this->random_dist_for_random_algorithm = std::uniform_int_distribution<unsigned long>(0, 1000);
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

    // Create compute/storage map
    for (auto const &cs : this->compute_services) {
        for (auto const &ss : this->storage_services) {
            if (ss->getHostname() == cs->getHostname()) {
                this->map_compute_to_storage[cs] = ss;
                break;
            }
        }
    }
    // Create idle core map
    for (auto const &cs : this->compute_services) {
        auto cores_available = cs->getPerHostNumCores();
        this->idle_cores_map[cs] = cores_available;
    }

}

std::shared_ptr<wrench::FileLocation>  SimpleStandardJobScheduler::pick_location(
        const std::shared_ptr<wrench::BareMetalComputeService>& compute_service,
        std::shared_ptr<wrench::DataFile> file) {

    auto entries = this->file_registry_service->lookupEntry(file);
    if (entries.empty()) {
        throw std::runtime_error("FILE " + file->getID() + " is nowhere to be found!");
    }

    std::shared_ptr<wrench::FileLocation> picked_local_location = nullptr;
    std::shared_ptr<wrench::FileLocation> picked_wms_location = nullptr;
    std::shared_ptr<wrench::FileLocation> picked_other_location = nullptr;

    // Pick the WMS host if file is there
    for (auto const &location : entries) {
        if (location->getStorageService() == this->map_compute_to_storage[compute_service]) {
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

bool SimpleStandardJobScheduler::taskCanRunOn(std::shared_ptr<wrench::WorkflowTask> task, const std::shared_ptr<wrench::BareMetalComputeService> service) {

#if 0
    auto idle_cores = service->getPerHostNumIdleCores();
    for (auto const &spec : idle_cores) {
        if (spec.second >= task->getMinNumCores()) {
            return true;
        }
    }
    std::cerr << service->getName() << " IS NOT OK\n";
    return false;
#else
    for (auto const &entry : this->idle_cores_map[service]) {
        if (entry.second >= task->getMinNumCores()) {
            return true;
        }
    }
    return false;
#endif
}

void SimpleStandardJobScheduler::prioritizeTasks(std::vector<std::shared_ptr<wrench::WorkflowTask> > &tasks) {

    std::sort(tasks.begin(), tasks.end(),
              this->task_priority_schemes[std::get<0>(this->scheduling_algorithms_index_to_tuple.at(this->current_scheduling_algorithm))]);

}

/** Returns true if found something **/
bool SimpleStandardJobScheduler::scheduleTask(std::shared_ptr<wrench::WorkflowTask> task,
                                              std::shared_ptr<wrench::BareMetalComputeService> *picked_service,
                                              std::string &picked_host,
                                              unsigned long *picked_num_cores) {


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

//    std::cerr << "I HAVE SELECTED " << possible_services.size() << " SERVICES THAT COULD WORK\n";
//    for (auto const &h : possible_services) {
//        std::cerr << "  - " << h->getName() << "\n";
//    }

    picked_host = "";
    *picked_service = this->service_selection_schemes[std::get<1>(this->scheduling_algorithms_index_to_tuple[this->current_scheduling_algorithm])](task, possible_services);
    *picked_num_cores = this->core_selection_schemes[std::get<2>(this->scheduling_algorithms_index_to_tuple[this->current_scheduling_algorithm])](task, *picked_service);
    for (auto const &entry : this->idle_cores_map[*picked_service]) {
        if (entry.second >= *picked_num_cores) {
            picked_host = entry.first;
            break;
        }
    }
    if (picked_host.empty()) {
        throw std::runtime_error("Picked_host is empty in SimpleStandardJobScheduler::scheduleTask(): this should not happen");
    }
    return true;
}

void SimpleStandardJobScheduler::scheduleTasks(std::vector<std::shared_ptr<wrench::WorkflowTask>> tasks) {

//    std::cerr << "GOT A BUNCH OF READY TASKS TO SCEHDULE: \n";
//    for (auto const &rt: tasks) {
//        std::cerr << "READY TASK: " << rt->getID() << ": NC = " << rt->getNumberOfChildren() <<"\n";
//    }

    prioritizeTasks(tasks);
//    std::cerr << "AFTER PRIORITIZATION: \n";
//    for (auto const &rt: tasks) {
//        std::cerr << "READY TASK: " << rt->getID() << ": NC = " << rt->getNumberOfChildren() <<"\n";
//    }

    int num_scheduled_tasks = 0;
    for (const auto &task : tasks) {

//        WRENCH_INFO("Trying to schedule ready task %s", task->getID().c_str());
        std::shared_ptr<wrench::BareMetalComputeService> picked_service;
        std::string picked_host;
        unsigned long picked_num_cores;

        if (not scheduleTask(task, &picked_service, picked_host, &picked_num_cores)) {
//            WRENCH_INFO("Wasn't able to schedule task %s", task->getID().c_str());
            continue;
        }

        WRENCH_INFO("Submitting task %s for execution on service at cluster %s on host %s with %lu cores",
                    task->getID().c_str(),
                    picked_service->getHostname().c_str(),
                    picked_host.c_str(),
                    picked_num_cores);

        // IMPORTANT: Update the idle cores map
        this->idle_cores_map[picked_service][picked_host] -= picked_num_cores;

        num_scheduled_tasks++;

        // Submitting the task as a simple job
        std::map<std::shared_ptr<wrench::DataFile> , std::shared_ptr<wrench::FileLocation>> file_locations;

        // Input files are read from the "best" location
        for (const auto &file : task->getInputFiles()) {
            // Pick a location
            std::shared_ptr<wrench::FileLocation> picked_location = pick_location(picked_service, file);
            file_locations.insert(std::make_pair(file, picked_location));
        }

        // Output file are all written locally
        std::shared_ptr<wrench::StorageService> target_ss = this->map_compute_to_storage[picked_service];

        for (const auto &f : task->getOutputFiles()) {
            file_locations.insert(std::make_pair(f, wrench::FileLocation::LOCATION(target_ss)));
        }

        auto job = this->job_manager->createStandardJob(task, file_locations);
        this->job_manager->submitJob(job, picked_service, {{task->getID(), picked_host + ":" + std::to_string(picked_num_cores)}});

    }
//    std::cerr << "DEBUG SCHEDULED " << num_scheduled_tasks << "\n";
//    WRENCH_INFO("Done with scheduling tasks as standard jobs");
}



std::string SimpleStandardJobScheduler::getTaskPrioritySchemeDocumentation() {
    std::string documentation;

    for (auto const &e : this->task_priority_schemes) {
        documentation += "  - " + e.first + "\n";
    }
    return documentation;
}


std::string SimpleStandardJobScheduler::getServiceSelectionSchemeDocumentation() {
    std::string documentation;

    for (auto const &e : this->service_selection_schemes) {
        documentation += "  - " + e.first + "\n";
    }
    return documentation;
}

std::string SimpleStandardJobScheduler::getCoreSelectionSchemeDocumentation() {
    std::string documentation;

    for (auto const &e : this->core_selection_schemes) {
        documentation += "  - " + e.first + "\n";
    }
    return documentation;
}

void SimpleStandardJobScheduler::printAllSchemes() {

    for (auto const &e : this->scheduling_algorithms_index_to_tuple) {
        unsigned int index = e.first;
        std::cout << "[" << (index < 100 ? "0" : "") << (index < 10 ? "0" : "") << index << "] "
                  << this->schedulingAlgorithmToString(index) << "\n";
    }
}

std::string SimpleStandardJobScheduler::schedulingAlgorithmToString(unsigned long index) {
    auto alg = this->scheduling_algorithms_index_to_tuple[index];
    return std::get<0>(alg) + ":" + std::get<1>(alg) + ":" + std::get<2>(alg);
}


std::vector<std::string> SimpleStandardJobScheduler::stringSplit(const std::string str, char sep) {
    stringstream ss(str);
    std::vector<std::string> tokens;
    string item;
    while (getline(ss, item, sep)) {
        tokens.push_back(item);
    }
    return tokens;
}

void SimpleStandardJobScheduler::computeBottomLevels(std::shared_ptr<wrench::Workflow> workflow) {

    for (auto const &t : workflow->getEntryTasks()) {
        computeTaskBottomLevel(t);
    }
}

void SimpleStandardJobScheduler::computeTaskBottomLevel(std::shared_ptr<wrench::WorkflowTask> task) {

    if (this->bottom_levels.find(task) != this->bottom_levels.end()) {
        return;
    }

    double my_bl = task->getFlops();
    double max = 0.0;
    for (const auto &child : task->getChildren()) {
        computeTaskBottomLevel(child);
        double bl = this->bottom_levels[child];
        max = (bl < max ? max : bl);
    }
    this->bottom_levels[task] = my_bl + max;
//    std::cerr << task->getID() << ": " << this->bottom_levels[task] << "\n";
}

