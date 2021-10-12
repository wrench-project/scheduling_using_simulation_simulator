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

XBT_LOG_NEW_DEFAULT_CATEGORY(simple_scheduler_service_selection_schemes, "Log category for service selection schemes");

/***************************************************/
/** Setting/Defining the service selection scheme **/
/***************************************************/
void SimpleStandardJobScheduler::initServiceSelectionSchemes() {

    this->service_selection_schemes["fastest_cores"] = [] (const wrench::WorkflowTask* task, const std::set<std::shared_ptr<wrench::BareMetalComputeService>> services) -> std::shared_ptr<wrench::BareMetalComputeService> {
        std::shared_ptr<wrench::BareMetalComputeService> picked = nullptr;
        for (auto const &s : services) {
            if ((picked == nullptr) or (s->getCoreFlopRate().begin()->second > picked->getCoreFlopRate().begin()->second)) {
                picked = s;
            }
        }
        return picked;
    };
    this->service_selection_schemes["most_idle_cores"] = [] (const wrench::WorkflowTask* task, const std::set<std::shared_ptr<wrench::BareMetalComputeService>> services) -> std::shared_ptr<wrench::BareMetalComputeService> {
        std::shared_ptr<wrench::BareMetalComputeService> picked = nullptr;
        for (auto const &s : services) {
            if ((picked == nullptr) or (s->getTotalNumIdleCores() > picked->getTotalNumIdleCores())) {
                picked = s;
            }
        }
        return picked;
    };

    this->service_selection_schemes["most_local_data"] = [this] (const wrench::WorkflowTask* task, const std::set<std::shared_ptr<wrench::BareMetalComputeService>> services) -> std::shared_ptr<wrench::BareMetalComputeService> {
        std::shared_ptr<wrench::BareMetalComputeService> picked = nullptr;
        double max_data_bytes = 0;
        for (auto const &s : services) {
            double data_bytes = 0;
            auto storage_service = this->map_compute_to_storage[s];
            for (auto const &f : task->getInputFiles()) {
                if (wrench::StorageService::lookupFile(f, wrench::FileLocation::LOCATION(storage_service))) {
                    data_bytes += f->getSize();
                }
            }
            if ((picked == nullptr) or (data_bytes > max_data_bytes)) {
                picked = s;
            }
        }
        return picked;
    };

    this->service_selection_schemes["random"] = [this] (const wrench::WorkflowTask* task, const std::set<std::shared_ptr<wrench::BareMetalComputeService>> services) -> std::shared_ptr<wrench::BareMetalComputeService> {
        auto picked = this->random_dist_for_random_algorithm(this->rng_for_random_algorithm) % services.size();
        for (auto const &s : services) {
            if (picked == 0) {
                return s;
            } else {
                picked--;
            }
        }
        return *(services.begin()); // just in case
    };

}
