/**
 * Copyright (c) 2020. <ADD YOUR HEADER INFORMATION>.
 * Generated with the wrench-init.in tool.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
#ifndef MY_SIMPLEWMS_H
#define MY_SIMPLEWMS_H

#include <wrench-dev.h>

class SimpleStandardJobScheduler;
class Simulation;

/**
 *  @brief A simple WMS implementation
 */
class SimpleWMS : public wrench::WMS {
public:
    SimpleWMS(SimpleStandardJobScheduler *scheduler,
              double first_scheduler_change_trigger,
              double periodic_scheduler_change_trigger,
              double speculative_work_fraction,
              const std::set<std::shared_ptr<wrench::ComputeService>> &compute_services,
              const std::set<std::shared_ptr<wrench::StorageService>> &storage_services,
              const std::shared_ptr<wrench::FileRegistryService> &file_registry_service,
              const std::string &hostname);

private:


    int main() override;
    void processEventStandardJobCompletion(std::shared_ptr<wrench::StandardJobCompletedEvent> event) override;
    void processEventStandardJobFailure(std::shared_ptr<wrench::StandardJobFailedEvent> event) override;

    SimpleStandardJobScheduler *scheduler;
    double first_scheduler_change_trigger;
    double periodic_scheduler_change_trigger;
    double speculative_work_fraction;
    double work_done_since_last_scheduler_change = 0.0;
    bool one_schedule_change_has_happened = false;
    bool i_am_speculative = false;

    std::shared_ptr<wrench::JobManager> job_manager;
    std::shared_ptr<wrench::DataMovementManager> data_movement_manager;
    std::shared_ptr<wrench::FileRegistryService> file_registry_service;
};

#endif //MY_SIMPLEWMS_H

