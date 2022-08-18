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

XBT_LOG_NEW_DEFAULT_CATEGORY(scheduling_algorithms_core_selection_schemes, "Log category for core selection schemes");

unsigned long pickNumCoresWithBoundedEfficiency(SimpleStandardJobScheduler *scheduler, const std::shared_ptr<wrench::WorkflowTask>& task, const std::shared_ptr<wrench::BareMetalComputeService>& service, double efficiency_bound) {
//    auto idle_cores = service->getPerHostNumIdleCores();
    auto idle_cores = scheduler->idle_cores_map[service];
    unsigned long max = 0;
    for (auto const &h : idle_cores) {
        max = std::max<unsigned long>(max, h.second);
    }
    auto model = std::dynamic_pointer_cast<wrench::AmdahlParallelModel>(task->getParallelModel());
    for (unsigned long num_cores=max; num_cores >= 1; num_cores--) {
        double efficiency = (1.0 / (1.0 * (1.0 - model->getAlpha()) + 1.0 * model->getAlpha() / (double)num_cores)) / (double)num_cores;
//        std::cerr << "   num_cores=" << num_cores << "  eff=" << efficiency << "\n";
        if (efficiency >= efficiency_bound) {
            return num_cores;
        }
    }
    return 1; // just in case
}

/************************************************/
/** Setting/Defining the core selection scheme **/
/************************************************/
void SimpleStandardJobScheduler::initCoreSelectionSchemes() {

//    this->core_selection_schemes["minimum"] = [] (const wrench::WorkflowTask* task, const std::shared_ptr<wrench::BareMetalComputeService> service) -> unsigned long {
//        return TASK_MIN_NUM_CORES(task);
//    };

    this->core_selection_schemes["as_many_as_possible"] = [this] (const std::shared_ptr<wrench::WorkflowTask>& task, const std::shared_ptr<wrench::BareMetalComputeService>& service) -> unsigned long {
//        auto idle_cores = service->getPerHostNumIdleCores();
        auto idle_cores = this->idle_cores_map[service];

        unsigned long max = 0;
        for (auto const &h : idle_cores) {
            max = std::max<unsigned long>(max, h.second);
        }
        if (max < TASK_MIN_NUM_CORES(task)) {
            throw std::runtime_error("Core selection scheme (AS MANY AS POSSIBLE): A potential compute service doesn't have enough cores - this shouldn't happen");
        }
        return std::min<unsigned long>(max, TASK_MAX_NUM_CORES(task));
    };

    this->core_selection_schemes["parallel_efficiency_fifty_percent"] = [this] (const std::shared_ptr<wrench::WorkflowTask>& task, const std::shared_ptr<wrench::BareMetalComputeService>& service) -> unsigned long {
        return pickNumCoresWithBoundedEfficiency(this, task, service, 0.5);
    };

    this->core_selection_schemes["parallel_efficiency_ninety_percent"] = [this] (const std::shared_ptr<wrench::WorkflowTask>& task, const std::shared_ptr<wrench::BareMetalComputeService>& service) -> unsigned long {
        return pickNumCoresWithBoundedEfficiency(this, task, service, 0.9);
    };

    this->core_selection_schemes["random"] = [this] (const std::shared_ptr<wrench::WorkflowTask>& task, const std::shared_ptr<wrench::BareMetalComputeService>& service) -> unsigned long {
//        auto idle_cores = service->getPerHostNumIdleCores();
        auto idle_cores = this->idle_cores_map[service];
        unsigned long max = 0;
        for (auto const &h : idle_cores) {
            max = std::max<unsigned long>(max, h.second);
        }
        if (max < TASK_MIN_NUM_CORES(task)) {
            throw std::runtime_error("Core selection scheme (RANDOM): A potential compute service doesn't have enough cores - this shouldn't happen");
        }
        max = std::min<unsigned long>(max, TASK_MAX_NUM_CORES(task));
        return 1 + this->random_dist_for_random_algorithm(this->rng_for_random_algorithm) % max;
    };

}
