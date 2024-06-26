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

#define TASK_MIN_NUM_CORES(task) 1
#define TASK_MAX_NUM_CORES(task) 32

//#define TASK_MIN_NUM_CORES(task) task->getMinNumCores()
//#define TASK_MAX_NUM_CORES(task) task->getMaxNumCores()


class SimpleStandardJobScheduler {

public:

    SimpleStandardJobScheduler();

    void scheduleTasks(std::vector<std::shared_ptr<wrench::WorkflowTask> > tasks);

    void init(
            std::shared_ptr<wrench::JobManager> job_manager,
            std::set<std::shared_ptr<wrench::BareMetalComputeService>> compute_services,
            std::set<std::shared_ptr<wrench::StorageService>> storage_services,
            std::string wms_host,
            double initial_load_max_duration,
            int initial_load_duration_seed,
            double initial_load_prob_core_loaded);

    void enableTaskSelectionScheme(const std::string& scheme);
    void enableClusterSelectionScheme(const std::string& scheme);
    void enableCoreSelectionScheme(const std::string& scheme);
    void finalizeEnabledAlgorithmList();

    unsigned long getNumAvailableSchedulingAlgorithms();
    unsigned long getNumEnabledSchedulingAlgorithms();
    void useSchedulingAlgorithmNow(unsigned long scheduler_index);
    void useSchedulingAlgorithmThen(unsigned long scheduler_index, double date);
    unsigned long getUsedSchedulingAlgorithm() const;
    std::string algorithmIndexToString(unsigned long);

    std::string getDocumentation();

    static std::vector<std::string> stringSplit(const std::string& str, char sep);

    void computeBottomLevels(const std::shared_ptr<wrench::Workflow>& workflow);
    void computeNumbersOfChildren(const std::shared_ptr<wrench::Workflow>& workflow);


    void setRandomAlgorithmSeed(int seed) { this->rng_for_random_algorithm.seed(seed);}

    std::unordered_map<std::shared_ptr<wrench::BareMetalComputeService>, double> core_flop_rate_map;
    std::unordered_map<std::shared_ptr<wrench::BareMetalComputeService>, std::map<std::string, unsigned long>> idle_cores_map;
    std::unordered_map<std::shared_ptr<wrench::DataFile>, std::set<std::shared_ptr<wrench::StorageService>>> file_replica_locations;

    void createInitialLoad(int initial_load_max_duration,
                           int initial_load_duration_seed,
                           double initial_load_prob_core_loaded);

private:

    std::string getTaskPrioritySchemeDocumentation();
    std::string getClusterSelectionSchemeDocumentation();
    std::string getCoreSelectionSchemeDocumentation();

    void computeTaskBottomLevel(const std::shared_ptr<wrench::WorkflowTask>& task);

    void initTaskPrioritySchemes();
    void initClusterSelectionSchemes();
    void initCoreSelectionSchemes();

    void prioritizeTasks(std::vector<std::shared_ptr<wrench::WorkflowTask>> &tasks);
    bool scheduleTask(const std::shared_ptr<wrench::WorkflowTask>& task,
                      std::shared_ptr<wrench::BareMetalComputeService> *picked_service,
                      std::string &picked_host,
                      unsigned long *picked_num_cores);


    std::shared_ptr<wrench::FileLocation> pick_location(const std::shared_ptr<wrench::BareMetalComputeService>& compute_service,
                                                        const std::shared_ptr<wrench::DataFile>& file);

    bool taskCanRunOn(const std::shared_ptr<wrench::WorkflowTask>& task, const std::shared_ptr<wrench::BareMetalComputeService>& service);


    std::vector<std::string> enabled_task_selection_schemes;
    std::vector<std::string> enabled_cluster_selection_schemes;
    std::vector<std::string> enabled_core_selection_schemes;
    std::vector<std::tuple<std::string, std::string, std::string>> enabled_scheduling_algorithms;

    unsigned long current_scheduling_algorithm = 0;
    unsigned long upcoming_scheduling_algorithm = 0;
    double upcoming_scheduling_algorithm_activation_date = DBL_MAX;

    std::map<std::string, std::function<bool(const std::shared_ptr<wrench::WorkflowTask> a, const std::shared_ptr<wrench::WorkflowTask> b)>> task_selection_schemes;
    std::map<std::string, std::function<std::shared_ptr<wrench::BareMetalComputeService> (const std::shared_ptr<wrench::WorkflowTask> task, const std::set<std::shared_ptr<wrench::BareMetalComputeService>> services)>> cluster_selection_schemes;
    std::map<std::string, std::function<unsigned long(const std::shared_ptr<wrench::WorkflowTask> a, const std::shared_ptr<wrench::BareMetalComputeService> service)>> core_selection_schemes;


    std::set<std::shared_ptr<wrench::StorageService>> storage_services;
    std::set<std::shared_ptr<wrench::BareMetalComputeService>> compute_services;
    std::map<std::shared_ptr<wrench::BareMetalComputeService>, std::shared_ptr<wrench::StorageService>> map_compute_to_storage;

    std::shared_ptr<wrench::JobManager> job_manager;
    std::string wms_host;

    std::map<std::shared_ptr<wrench::WorkflowTask>, double> bottom_levels;
    std::map<std::shared_ptr<wrench::WorkflowTask>, unsigned long> number_children;

    std::uniform_int_distribution<unsigned long> random_dist_for_random_algorithm;
    std::mt19937 rng_for_random_algorithm;

};

#endif //MY_SIMPLESCHEDULER_H

