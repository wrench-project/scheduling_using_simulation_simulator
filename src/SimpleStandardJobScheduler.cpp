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
    this->initClusterSelectionSchemes();
    this->initCoreSelectionSchemes();

    // Create the random dist / rng for the random:random:random
    this->random_dist_for_random_algorithm = std::uniform_int_distribution<unsigned long>(0, 1000);
}


void SimpleStandardJobScheduler::init(
        std::shared_ptr<wrench::JobManager> _job_manager,
        std::set<std::shared_ptr<wrench::BareMetalComputeService>> _compute_services,
        std::set<std::shared_ptr<wrench::StorageService>> _storage_services,
        std::string wms_host) {
    this->job_manager = std::move(_job_manager);
    this->storage_services = std::move(_storage_services);
    this->compute_services = std::move(_compute_services);
    this->wms_host = std::move(wms_host);

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
    // Create core flop rate map
    for (auto const &cs: this->compute_services) {
        auto core_flop_rates = cs->getCoreFlopRate();
        this->core_flop_rate_map[cs] = (*(core_flop_rates.begin())).second;
    }

}

std::shared_ptr<wrench::FileLocation>  SimpleStandardJobScheduler::pick_location(
        const std::shared_ptr<wrench::BareMetalComputeService>& compute_service,
        const std::shared_ptr<wrench::DataFile>& file) {

    if (this->file_replica_locations.find(file) == this->file_replica_locations.end()) {
        throw std::runtime_error("FILE " + file->getID() + " is nowhere to be found!");
    }

    std::shared_ptr<wrench::FileLocation> picked_local_location = nullptr;
    std::shared_ptr<wrench::FileLocation> picked_wms_location = nullptr;
    std::shared_ptr<wrench::FileLocation> picked_other_location = nullptr;

    // Pick the WMS host if file is there
    for (auto const &ss : this->file_replica_locations[file]) {
        if (ss == this->map_compute_to_storage[compute_service]) {
            picked_local_location = wrench::FileLocation::LOCATION(ss, file);
            continue;
        }
        if (ss->getHostname() == this->wms_host) {
            picked_wms_location = wrench::FileLocation::LOCATION(ss, file);
            continue;
        }
        if (picked_other_location == nullptr) {
            picked_other_location = wrench::FileLocation::LOCATION(ss, file);;
            continue;
        }
    }

    if (picked_local_location) return picked_local_location;
    if (picked_wms_location) return picked_wms_location;
    if (picked_other_location) return picked_other_location;

    throw std::runtime_error("A location should have been returned");

}

bool SimpleStandardJobScheduler::taskCanRunOn(const std::shared_ptr<wrench::WorkflowTask>& task, const std::shared_ptr<wrench::BareMetalComputeService>& service) {

#if 0
    auto idle_cores = service->getPerHostNumIdleCores();
    for (auto const &spec : idle_cores) {
        if (spec.second >= TASK_MIN_NUM_CORES(task)) {
            return true;
        }
    }
    std::cerr << service->getName() << " IS NOT OK\n";
    return false;
#else
    for (auto const &entry : this->idle_cores_map[service]) {
        if (entry.second >= TASK_MIN_NUM_CORES(task)) {
            return true;
        }
    }
    return false;
#endif
}

void SimpleStandardJobScheduler::prioritizeTasks(std::vector<std::shared_ptr<wrench::WorkflowTask> > &tasks) {

    std::sort(tasks.begin(), tasks.end(),
              this->task_selection_schemes[std::get<0>(this->enabled_scheduling_algorithms.at(this->current_scheduling_algorithm))]);

}

/** Returns true if found something **/
bool SimpleStandardJobScheduler::scheduleTask(const std::shared_ptr<wrench::WorkflowTask>& task,
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
//    WRENCH_INFO("PICKING SERVICE %s", std::get<1>(this->enabled_scheduling_algorithms[this->current_scheduling_algorithm]).c_str());
    *picked_service = this->cluster_selection_schemes[std::get<1>(this->enabled_scheduling_algorithms[this->current_scheduling_algorithm])](task, possible_services);
//    WRENCH_INFO("PICKING NUM_CORES");
    // If it's a ficticious initial load task, it uses a single core and that's it
    if (task->getID().rfind("initial_load_task_", 0) == 0) {
        *picked_num_cores = 1;
    } else {
        *picked_num_cores = this->core_selection_schemes[std::get<2>(
                this->enabled_scheduling_algorithms[this->current_scheduling_algorithm])](task, *picked_service);
    }
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

    // Update scheduling algorithm if need be
    if (this->upcoming_scheduling_algorithm_activation_date <= wrench::Simulation::getCurrentSimulatedDate()) {
        std::cerr << "[" + std::to_string(wrench::Simulation::getCurrentSimulatedDate()) + "] Actually switching to algorithm " <<
                  "[" << (this->upcoming_scheduling_algorithm < 100 ? "0" : "") << (this->upcoming_scheduling_algorithm < 10 ? "0" : "") << this->upcoming_scheduling_algorithm << "] " <<
                  this->algorithmIndexToString(this->upcoming_scheduling_algorithm) << "\n";
        this->current_scheduling_algorithm = this->upcoming_scheduling_algorithm;
        this->upcoming_scheduling_algorithm_activation_date = DBL_MAX;
    }
//    std::cerr << "GOT A BUNCH OF READY TASKS TO SCHEDULE: \n";
//    for (auto const &rt: tasks) {
//        std::cerr << "READY TASK: " << rt->getID() << ": NC = " << rt->getNumberOfChildren() <<"\n";
//    }

    prioritizeTasks(tasks);
//    std::cerr << "AFTER PRIORITIZATION: \n";
//    for (auto const &rt: tasks) {
//        std::cerr << "READY TASK: " << rt->getID() << ": NC = " << rt->getNumberOfChildren() <<"\n";
//    }

    int num_scheduled_tasks = 0;
//    WRENCH_INFO("SCHEDULING TASKS");
    for (const auto &task : tasks) {

//        WRENCH_INFO("Trying to schedule ready task %s", task->getID().c_str());
        std::shared_ptr<wrench::BareMetalComputeService> picked_service;
        std::string picked_host;
        unsigned long picked_num_cores;

//        WRENCH_INFO("Trying to schedule task %s", task->getID().c_str());
        if (not scheduleTask(task, &picked_service, picked_host, &picked_num_cores)) {
            WRENCH_INFO("Wasn't able to schedule task %s", task->getID().c_str());
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
            file_locations.insert(std::make_pair(f, wrench::FileLocation::LOCATION(target_ss, f)));
        }

        auto job = this->job_manager->createStandardJob(task, file_locations);
//        std::cerr << wrench::Simulation::getCurrentSimulatedDate() << ": SUBMITTING TASK: " << task->getID() << "\n";
        this->job_manager->submitJob(job, picked_service, {{task->getID(), picked_host + ":" + std::to_string(picked_num_cores)}});

    }
//    std::cerr << "DEBUG SCHEDULED " << num_scheduled_tasks << "\n";
//    WRENCH_INFO("Done with scheduling tasks as standard jobs");
}


std::string SimpleStandardJobScheduler::getDocumentation() {

    std::string scheduler_doc;
    scheduler_doc += "* Task selection schemes:\n";
    scheduler_doc += this->getTaskPrioritySchemeDocumentation();
    scheduler_doc += "* Cluster selection schemes:\n";
    scheduler_doc += this->getClusterSelectionSchemeDocumentation();
    scheduler_doc += "* Core selection schemes:\n";
    scheduler_doc += this->getCoreSelectionSchemeDocumentation();
    return scheduler_doc;
}


std::string SimpleStandardJobScheduler::getTaskPrioritySchemeDocumentation() {
    std::string documentation;

    for (auto const &e : this->task_selection_schemes) {
        documentation += "  - " + e.first + "\n";
    }
    return documentation;
}


std::string SimpleStandardJobScheduler::getClusterSelectionSchemeDocumentation() {
    std::string documentation;

    for (auto const &e : this->cluster_selection_schemes) {
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

std::vector<std::string> SimpleStandardJobScheduler::stringSplit(const std::string& str, char sep) {
    stringstream ss(str);
    std::vector<std::string> tokens;
    string item;
    while (getline(ss, item, sep)) {
        tokens.push_back(item);
    }
    return tokens;
}

void SimpleStandardJobScheduler::computeNumbersOfChildren(const std::shared_ptr<wrench::Workflow>& workflow) {

    for (auto const &t : workflow->getTasks()) {
        this->number_children[t] =t->getNumberOfChildren();
    }
}

void SimpleStandardJobScheduler::computeBottomLevels(const std::shared_ptr<wrench::Workflow>& workflow) {

    for (auto const &t : workflow->getEntryTasks()) {
        computeTaskBottomLevel(t);
    }
}

void SimpleStandardJobScheduler::computeTaskBottomLevel(const std::shared_ptr<wrench::WorkflowTask>& task) {

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

void SimpleStandardJobScheduler::enableTaskSelectionScheme(const std::string& scheme) {
    if (this->task_selection_schemes.find(scheme) == this->task_selection_schemes.end()) {
        throw std::invalid_argument("Unknown task selection scheme: " + scheme);
    }
    if (std::find(this->enabled_task_selection_schemes.begin(),
                  this->enabled_task_selection_schemes.end(),
                  scheme) ==
        this->enabled_task_selection_schemes.end()) {
        this->enabled_task_selection_schemes.push_back(scheme);
    }
}

void SimpleStandardJobScheduler::enableClusterSelectionScheme(const std::string& scheme) {
    if (this->cluster_selection_schemes.find(scheme) == this->cluster_selection_schemes.end()) {
        throw std::invalid_argument("Unknown cluster selection scheme: " + scheme);
    }
    if (std::find(this->enabled_cluster_selection_schemes.begin(),
                  this->enabled_cluster_selection_schemes.end(),
                  scheme) ==
        this->enabled_cluster_selection_schemes.end()) {
        this->enabled_cluster_selection_schemes.push_back(scheme);
    }

}

void SimpleStandardJobScheduler::enableCoreSelectionScheme(const std::string& scheme) {
    if (this->core_selection_schemes.find(scheme) == this->core_selection_schemes.end()) {
        throw std::invalid_argument("Unknown core selection scheme: " + scheme);
    }
    if (std::find(this->enabled_core_selection_schemes.begin(),
                  this->enabled_core_selection_schemes.end(),
                  scheme) ==
        this->enabled_core_selection_schemes.end()) {
        this->enabled_core_selection_schemes.push_back(scheme);
    }
}

void SimpleStandardJobScheduler::finalizeEnabledAlgorithmList() {
    for (const auto &task_ss : this->enabled_task_selection_schemes) {
        for (const auto &cluster_ss: this->enabled_cluster_selection_schemes) {
            for (const auto &core_ss: this->enabled_core_selection_schemes) {
                this->enabled_scheduling_algorithms.emplace_back(std::make_tuple(task_ss, cluster_ss, core_ss));
            }
        }
    }
//    std::cerr << "NUM_ALGOS = " << this->enabled_scheduling_algorithms.size() << "\n";


}

std::string SimpleStandardJobScheduler::algorithmIndexToString(unsigned long index) {
    auto algorithm = this->enabled_scheduling_algorithms.at(index);
    return std::get<0>(algorithm) + "/" +
           std::get<1>(algorithm) + "/" +
           std::get<2>(algorithm);
}

unsigned long SimpleStandardJobScheduler::getNumAvailableSchedulingAlgorithms() {
    return this->enabled_scheduling_algorithms.size();
}

unsigned long SimpleStandardJobScheduler::getNumEnabledSchedulingAlgorithms() {
    return this->enabled_scheduling_algorithms.size();
}

void SimpleStandardJobScheduler::useSchedulingAlgorithmNow(unsigned long scheduler_index) {
    this->current_scheduling_algorithm = scheduler_index;
    this->upcoming_scheduling_algorithm = scheduler_index;
    this->upcoming_scheduling_algorithm_activation_date = DBL_MAX;
}

unsigned long SimpleStandardJobScheduler::getUsedSchedulingAlgorithm() const {
    return this->current_scheduling_algorithm;
}

void SimpleStandardJobScheduler::useSchedulingAlgorithmThen(unsigned long scheduler_index, double date) {
    // Ignore a change is a previous change hasn't been activated yet.
    if (this->upcoming_scheduling_algorithm_activation_date < DBL_MAX and
        this->upcoming_scheduling_algorithm_activation_date > wrench::Simulation::getCurrentSimulatedDate()) {
        return;
    }
    this->upcoming_scheduling_algorithm = scheduler_index;
    this->upcoming_scheduling_algorithm_activation_date = date;
}
