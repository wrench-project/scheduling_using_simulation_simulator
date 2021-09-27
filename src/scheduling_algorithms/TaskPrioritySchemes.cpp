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

XBT_LOG_NEW_DEFAULT_CATEGORY(simple_scheduler_task_priority_schemes, "Log category for task priority schemes");

/***********************************************/
/** Setting/Defining the task priority scheme **/
/***********************************************/
void SimpleStandardJobScheduler::initTaskPrioritySchemes() {

    this->task_priority_schemes["most_flops"] = [](const wrench::WorkflowTask *a,
                                                      const wrench::WorkflowTask *b) -> bool {
        if (a->getFlops() < b->getFlops()) {
            return true;
        } else if (a->getFlops() > b->getFlops()) {
            return false;
        } else {
            return ((unsigned long) a < (unsigned long) b);
        }
    };
    this->task_priority_schemes["most_data"] = [](const wrench::WorkflowTask *a,
                                                   const wrench::WorkflowTask *b) -> bool {
        double a_bytes = 0.0, b_bytes = 0.0;
        for (auto const &f : a->getInputFiles()) {
            a_bytes += f->getSize();
        }
        for (auto const &f : b->getInputFiles()) {
            b_bytes += f->getSize();
        }

        if (a_bytes < b_bytes) {
            return true;
        } else if (a_bytes > b_bytes) {
            return false;
        } else {
            return ((unsigned long) a < (unsigned long) b);
        }
    };

}
