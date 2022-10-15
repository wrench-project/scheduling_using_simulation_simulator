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

/**
 *  @brief A simple WMS implementation
 */
class SimpleWMS : public wrench::ExecutionController {
public:
    SimpleWMS(SimpleStandardJobScheduler *scheduler,
              std::shared_ptr<wrench::Workflow> workflow,
              double first_scheduler_change_trigger,
              double periodic_scheduler_change_trigger,
              double speculative_work_fraction,
              std::string &simulation_noise_scheme,
              double simulation_noise,
              int noise_seed,
              double simulation_noise_reduction,
              double energy_bound,
              std::string &algorithm_selection_scheme,
              double simulation_overhead,
              bool disable_contention,
              bool disable_adaptation_if_noise_has_not_changed,
              bool at_most_one_noise_reduction,
              std::set<std::shared_ptr<wrench::BareMetalComputeService>> compute_services,
              std::set<std::shared_ptr<wrench::StorageService>> storage_services,
              const std::string &hostname);

    std::vector<unsigned long> getAlgorithmSequence() { return this->algorithm_sequence; }

    bool i_am_speculative = false;

private:

    int main() override;
    void processEventStandardJobCompletion(std::shared_ptr<wrench::StandardJobCompletedEvent> event) override;
    void processEventStandardJobFailure(std::shared_ptr<wrench::StandardJobFailedEvent> event) override;

    std::shared_ptr<wrench::Workflow> workflow;
    SimpleStandardJobScheduler *scheduler;
    double first_scheduler_change_trigger;
    double periodic_scheduler_change_trigger;
    double speculative_work_fraction;
    std::string simulation_noise_scheme;
    double simulation_noise;
    int noise_seed;
    double simulation_noise_reduction;
    double energy_bound;
    std::string algorithm_selection_scheme;
    double simulation_overhead;
    bool disable_contention;
    bool disable_adaptation_if_noise_has_not_changed;
    bool at_most_one_noise_reduction;

    double work_done_since_last_scheduler_change = 0.0;
    bool one_schedule_change_has_happened = false;

    std::shared_ptr<wrench::JobManager> job_manager;
    std::shared_ptr<wrench::DataMovementManager> data_movement_manager;


    std::set<std::shared_ptr<wrench::BareMetalComputeService>> compute_services;
    std::set<std::shared_ptr<wrench::StorageService>> storage_services;

    std::vector<unsigned long> algorithm_sequence;


};

#endif //MY_SIMPLEWMS_H

