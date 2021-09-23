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
    SimpleStandardJobScheduler() {}

    void scheduleTasks(const std::vector<wrench::WorkflowTask *> &tasks);

    void init(
            std::shared_ptr<wrench::JobManager> job_manager,
            std::set<std::shared_ptr<wrench::BareMetalComputeService>> compute_services,
            std::set<std::shared_ptr<wrench::StorageService>> storage_services,
            std::shared_ptr<wrench::FileRegistryService> file_registry_service,
            std::string wms_host);

private:
    bool scheduleTask(wrench::WorkflowTask *task,
                      std::shared_ptr<wrench::BareMetalComputeService> *picked_service,
                      int *picked_num_cores);

    std::shared_ptr<wrench::FileLocation> pick_location(const std::shared_ptr<wrench::BareMetalComputeService>& compute_service,
                                                        wrench::WorkflowFile *file);


    std::shared_ptr<wrench::FileRegistryService> file_registry_service;
    std::set<std::shared_ptr<wrench::StorageService>> storage_services;
    std::set<std::shared_ptr<wrench::BareMetalComputeService>> compute_services;
    std::shared_ptr<wrench::JobManager> job_manager;
    std::string wms_host;
};

#endif //MY_SIMPLESCHEDULER_H

