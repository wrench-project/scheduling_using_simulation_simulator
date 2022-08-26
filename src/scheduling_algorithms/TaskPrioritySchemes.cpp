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

XBT_LOG_NEW_DEFAULT_CATEGORY(simple_scheduler_task_selection_schemes, "Log category for task priority schemes");

/***********************************************/
/** Setting/Defining the task priority scheme **/
/***********************************************/
void SimpleStandardJobScheduler::initTaskPrioritySchemes() {

    this->task_selection_schemes["most_flops"] = [](const std::shared_ptr<wrench::WorkflowTask> &a,
                                                   const std::shared_ptr<wrench::WorkflowTask> &b) -> bool {
        if (a->getFlops() < b->getFlops()) {
            return false;
        } else if (a->getFlops() > b->getFlops()) {
            return true;
        } else {
            return (a->getID() < b->getID());
        }
    };

    this->task_selection_schemes["most_data"] = [](const std::shared_ptr<wrench::WorkflowTask> &a,
                                                  const std::shared_ptr<wrench::WorkflowTask> &b) -> bool {
        double a_bytes = 0.0, b_bytes = 0.0;
        for (auto const &f : a->getInputFiles()) {
            a_bytes += f->getSize();
        }
        for (auto const &f : b->getInputFiles()) {
            b_bytes += f->getSize();
        }

        if (a_bytes < b_bytes) {
            return false;
        } else if (a_bytes > b_bytes) {
            return true;
        } else {
            return (a->getID() < b->getID());
        }
    };

    this->task_selection_schemes["highest_bottom_level"] = [this](
            const std::shared_ptr<wrench::WorkflowTask>& a,
            const std::shared_ptr<wrench::WorkflowTask>& b) -> bool {

        double a_bl = this->bottom_levels[a];
        double b_bl = this->bottom_levels[b];

        if (a_bl < b_bl) {
            return false;
        } else if (a_bl > b_bl) {
            return true;
        } else {
            return (a->getID() < b->getID());
        }
    };

    this->task_selection_schemes["most_children"] = [this](
            const std::shared_ptr<wrench::WorkflowTask>& a,
            const std::shared_ptr<wrench::WorkflowTask>& b) -> bool {

        unsigned long a_num_children = this->number_children[a];
        unsigned long b_num_children = this->number_children[b];

        if (a_num_children < b_num_children) {
            return false;
        } else if (a_num_children > b_num_children) {
            return true;
        } else {
            return (a->getID() < b->getID());
        }
    };

    this->task_selection_schemes["random"] = [](
            const std::shared_ptr<wrench::WorkflowTask>& a,
            const std::shared_ptr<wrench::WorkflowTask>& b) -> bool {

        // This is not really random, but leads to deterministic sorting.
        return ((17 * (unsigned long)a.get() + 11 * (unsigned long)b.get()) % 2) == 1;
    };

}
