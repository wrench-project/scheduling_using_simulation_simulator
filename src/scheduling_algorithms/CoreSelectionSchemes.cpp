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

/************************************************/
/** Setting/Defining the core selection scheme **/
/************************************************/
void SimpleStandardJobScheduler::initCoreSelectionSchemes() {

    this->core_selection_schemes["minimum"] = [] (const wrench::WorkflowTask* task, const std::shared_ptr<wrench::BareMetalComputeService> service) -> unsigned long {
        return (task->getMinNumCores());
    };
    this->core_selection_schemes["as_many_as_possible"] = [] (const wrench::WorkflowTask* task, const std::shared_ptr<wrench::BareMetalComputeService> service) -> unsigned long {
        auto idle_cores = service->getPerHostNumIdleCores();
        unsigned long max = 0;
        for (auto const &h : idle_cores) {
            max = std::max<unsigned long>(max, h.second);
        }
        return std::min<unsigned long>(max, task->getMaxNumCores());
    };

}
