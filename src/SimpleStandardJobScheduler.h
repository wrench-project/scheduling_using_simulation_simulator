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

    void scheduleTasks(std::vector<wrench::WorkflowTask *> tasks);

    void addSchedulingAlgorithm(std::string spec);

    void init(
            std::shared_ptr<wrench::JobManager> job_manager,
            std::set<std::shared_ptr<wrench::BareMetalComputeService>> compute_services,
            std::set<std::shared_ptr<wrench::StorageService>> storage_services,
            std::shared_ptr<wrench::FileRegistryService> file_registry_service,
            std::string wms_host);

    std::string getTaskPrioritySchemeDocumentation();
    std::string getServiceSelectionSchemeDocumentation();
    std::string getCoreSelectionSchemeDocumentation();

private:
    void prioritizeTasks(std::vector<wrench::WorkflowTask *> &tasks);
    bool scheduleTask(wrench::WorkflowTask *task,
                      std::shared_ptr<wrench::BareMetalComputeService> *picked_service,
                      int *picked_num_cores);

    std::shared_ptr<wrench::FileLocation> pick_location(const std::shared_ptr<wrench::BareMetalComputeService>& compute_service,
                                                        wrench::WorkflowFile *file);

    bool taskCanRunOn(wrench::WorkflowTask *task, const std::shared_ptr<wrench::BareMetalComputeService> service);


        std::vector<std::tuple<std::string, std::string, std::string>> scheduling_algorithms;
    int current_scheduler = 0;

    std::map<std::string, std::function<bool(const wrench::WorkflowTask* a, const wrench::WorkflowTask* b)>> task_priority_schemes;
    std::map<std::string, std::function<std::shared_ptr<wrench::BareMetalComputeService> (const wrench::WorkflowTask* task, const std::set<std::shared_ptr<wrench::BareMetalComputeService>> services)>> service_selection_schemes;
    std::map<std::string, std::function<int(const wrench::WorkflowTask* a, const std::shared_ptr<wrench::BareMetalComputeService> service)>> core_selection_schemes;


    std::shared_ptr<wrench::FileRegistryService> file_registry_service;
    std::set<std::shared_ptr<wrench::StorageService>> storage_services;
    std::set<std::shared_ptr<wrench::BareMetalComputeService>> compute_services;
    std::shared_ptr<wrench::JobManager> job_manager;
    std::string wms_host;

};

#endif //MY_SIMPLESCHEDULER_H

