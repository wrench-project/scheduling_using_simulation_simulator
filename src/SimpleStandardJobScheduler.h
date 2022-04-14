/**
 * Copyright (c) 2020. <ADD YOUR HEADER INFORMATION>.
 * Generated with the wrench-init.in tool.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
#ifndef MY_SIMPLESCHEDULER_H
#define MY_SIMPLESCHEDULER_H

#include <wrench-dev.h>

class SimpleStandardJobScheduler {

public:

    SimpleStandardJobScheduler();

    void scheduleTasks(std::vector<std::shared_ptr<wrench::WorkflowTask> > tasks);

    void init(
            std::shared_ptr<wrench::JobManager> job_manager,
            std::set<std::shared_ptr<wrench::BareMetalComputeService>> compute_services,
            std::set<std::shared_ptr<wrench::StorageService>> storage_services,
            std::shared_ptr<wrench::FileRegistryService> file_registry_service,
            std::string wms_host);

    unsigned long getNumAvailableSchedulingAlgorithms() { return this->scheduling_algorithms_index_to_tuple.size(); }
    void enableSchedulingAlgorithm(unsigned long index) { this->enabled_scheduling_algorithms.emplace_back(index); }
    std::vector<unsigned long> getEnabledSchedulingAlgorithms() { return this->enabled_scheduling_algorithms; }
    void useSchedulingAlgorithm(unsigned long scheduler_index) { this->current_scheduling_algorithm = scheduler_index; }
    unsigned long getUsedSchedulingAlgorithm() const { return this->current_scheduling_algorithm; }
    std::string schedulingAlgorithmToString(unsigned long index);

    std::string getTaskPrioritySchemeDocumentation();
    std::string getServiceSelectionSchemeDocumentation();
    std::string getCoreSelectionSchemeDocumentation();
    void printAllSchemes();

    static std::vector<std::string> stringSplit(std::string str, char sep);

    void computeBottomLevels(std::shared_ptr<wrench::Workflow> workflow);

    void setRandomAlgorithmSeed(int seed) { this->rng_for_random_algorithm.seed(seed);}

    std::unordered_map<std::shared_ptr<wrench::BareMetalComputeService>, std::map<std::string, unsigned long>> idle_cores_map;

private:

    void computeTaskBottomLevel(std::shared_ptr<wrench::WorkflowTask> task);

    void initTaskPrioritySchemes();
    void initServiceSelectionSchemes();
    void initCoreSelectionSchemes();

    void prioritizeTasks(std::vector<std::shared_ptr<wrench::WorkflowTask>> &tasks);
    bool scheduleTask(std::shared_ptr<wrench::WorkflowTask> task,
                      std::shared_ptr<wrench::BareMetalComputeService> *picked_service,
                      std::string &picked_host,
                      unsigned long *picked_num_cores);


    std::shared_ptr<wrench::FileLocation> pick_location(const std::shared_ptr<wrench::BareMetalComputeService>& compute_service,
                                                        std::shared_ptr<wrench::DataFile> file);

    bool taskCanRunOn(std::shared_ptr<wrench::WorkflowTask> task, std::shared_ptr<wrench::BareMetalComputeService> service);

    std::vector<unsigned long> enabled_scheduling_algorithms;
    std::
    map<unsigned long, std::tuple<std::string, std::string, std::string>> scheduling_algorithms_index_to_tuple;

    unsigned long current_scheduling_algorithm = 0;

    std::unordered_map<std::string, std::function<bool(const std::shared_ptr<wrench::WorkflowTask> a, const std::shared_ptr<wrench::WorkflowTask> b)>> task_priority_schemes;
    std::unordered_map<std::string, std::function<std::shared_ptr<wrench::BareMetalComputeService> (const std::shared_ptr<wrench::WorkflowTask> task, const std::set<std::shared_ptr<wrench::BareMetalComputeService>> services)>> service_selection_schemes;
    std::unordered_map<std::string, std::function<unsigned long(const std::shared_ptr<wrench::WorkflowTask> a, const std::shared_ptr<wrench::BareMetalComputeService> service)>> core_selection_schemes;


    std::shared_ptr<wrench::FileRegistryService> file_registry_service;
    std::set<std::shared_ptr<wrench::StorageService>> storage_services;
    std::set<std::shared_ptr<wrench::BareMetalComputeService>> compute_services;
    std::unordered_map<std::shared_ptr<wrench::BareMetalComputeService>, std::shared_ptr<wrench::StorageService>> map_compute_to_storage;

    std::shared_ptr<wrench::JobManager> job_manager;
    std::string wms_host;

    std::map<std::shared_ptr<wrench::WorkflowTask>, double> bottom_levels;

    std::uniform_int_distribution<unsigned long> random_dist_for_random_algorithm;
    std::mt19937 rng_for_random_algorithm;

};

#endif //MY_SIMPLESCHEDULER_H

