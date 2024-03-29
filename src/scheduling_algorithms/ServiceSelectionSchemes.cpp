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

XBT_LOG_NEW_DEFAULT_CATEGORY(simple_scheduler_cluster_selection_schemes, "Log category for service selection schemes");

/***************************************************/
/** Setting/Defining the cluster selection scheme **/
/***************************************************/
void SimpleStandardJobScheduler::initClusterSelectionSchemes() {

    this->cluster_selection_schemes["fastest_cores"] = [this] (const std::shared_ptr<wrench::WorkflowTask>& task, const std::set<std::shared_ptr<wrench::BareMetalComputeService>>& services) -> std::shared_ptr<wrench::BareMetalComputeService> {
        std::shared_ptr<wrench::BareMetalComputeService> picked = nullptr;
        for (auto const &s : services) {
            if (picked == nullptr) {
                picked = s;
            } else if (this->core_flop_rate_map[s] > this->core_flop_rate_map[picked]) {
                picked = s;
            } else if ((std::abs<double>(this->core_flop_rate_map[s] - this->core_flop_rate_map[picked]) < 0.00001)
                       and (s->getHostname() > picked->getHostname())) {
                picked = s;
            }
        }
        return picked;
    };

    this->cluster_selection_schemes["slowest_cores"] = [this] (const std::shared_ptr<wrench::WorkflowTask>& task, const std::set<std::shared_ptr<wrench::BareMetalComputeService>>& services) -> std::shared_ptr<wrench::BareMetalComputeService> {
        std::shared_ptr<wrench::BareMetalComputeService> picked = nullptr;
        for (auto const &s : services) {
            if (picked == nullptr) {
                picked = s;
            } else if (this->core_flop_rate_map[s] < this->core_flop_rate_map[picked]) {
                picked = s;
            } else if ((std::abs<double>(this->core_flop_rate_map[s] - this->core_flop_rate_map[picked]) < 0.00001)
                       and (s->getHostname() > picked->getHostname())) {
                picked = s;
            }
        }
        return picked;
    };

    this->cluster_selection_schemes["most_idle_cores"] = [this] (const std::shared_ptr<wrench::WorkflowTask>& task, const std::set<std::shared_ptr<wrench::BareMetalComputeService>>& services) -> std::shared_ptr<wrench::BareMetalComputeService> {
        std::shared_ptr<wrench::BareMetalComputeService> picked = nullptr;
        for (auto const &s : services) {
            if (picked == nullptr) {
                picked = s;
            } else {
                unsigned long s_total_cores = 0;
                unsigned long picked_total_cores = 0;
                for (auto const &entry : this->idle_cores_map[s]) {
                    s_total_cores += entry.second;
                }
                for (auto const &entry : this->idle_cores_map[picked]) {
                    picked_total_cores += entry.second;
                }
                if (s_total_cores > picked_total_cores) {
                    picked = s;
                } else if ((s_total_cores == picked_total_cores) and (s->getHostname() > picked->getHostname())) {
                    picked = s;
                }
            }
        }
        return picked;
    };

    this->cluster_selection_schemes["least_idle_cores"] = [this] (const std::shared_ptr<wrench::WorkflowTask>& task, const std::set<std::shared_ptr<wrench::BareMetalComputeService>>& services) -> std::shared_ptr<wrench::BareMetalComputeService> {
        std::shared_ptr<wrench::BareMetalComputeService> picked = nullptr;
        for (auto const &s : services) {
            if (picked == nullptr) {
                picked = s;
            } else {
                unsigned long s_total_cores = 0;
                unsigned long picked_total_cores = 0;
                for (auto const &entry : this->idle_cores_map[s]) {
                    s_total_cores += entry.second;
                }
                for (auto const &entry : this->idle_cores_map[picked]) {
                    picked_total_cores += entry.second;
                }
                if (s_total_cores < picked_total_cores) {
                    picked = s;
                } else if ((s_total_cores == picked_total_cores) and (s->getHostname() > picked->getHostname())) {
                    picked = s;
                }
            }
        }
        return picked;
    };

    this->cluster_selection_schemes["most_local_data"] = [this] (const std::shared_ptr<wrench::WorkflowTask>& task, const std::set<std::shared_ptr<wrench::BareMetalComputeService>>& services) -> std::shared_ptr<wrench::BareMetalComputeService> {
        std::shared_ptr<wrench::BareMetalComputeService> picked = nullptr;
        double max_data_bytes = 0;
        for (auto const &s : services) {
            double data_bytes = 0;
            auto storage_service = this->map_compute_to_storage[s];
            for (auto const &f : task->getInputFiles()) {
                if (wrench::StorageService::lookupFileAtLocation(wrench::FileLocation::LOCATION(storage_service, f))) {
                    data_bytes += f->getSize();
                }
            }
            if ((picked == nullptr) or (data_bytes > max_data_bytes)) {
                picked = s;
                max_data_bytes = data_bytes;
            } else if ((std::abs<double>(data_bytes - max_data_bytes) < 0.00001) and (s->getHostname() > picked->getHostname())) {
                picked = s;
            }
        }
        return picked;
    };

    this->cluster_selection_schemes["least_local_data"] = [this] (const std::shared_ptr<wrench::WorkflowTask>& task, const std::set<std::shared_ptr<wrench::BareMetalComputeService>>& services) -> std::shared_ptr<wrench::BareMetalComputeService> {
        std::shared_ptr<wrench::BareMetalComputeService> picked = nullptr;
        double max_data_bytes = 0;
        for (auto const &s : services) {
            double data_bytes = 0;
            auto storage_service = this->map_compute_to_storage[s];
            for (auto const &f : task->getInputFiles()) {
                if (wrench::StorageService::lookupFileAtLocation(wrench::FileLocation::LOCATION(storage_service, f))) {
                    data_bytes += f->getSize();
                }
            }
            if ((picked == nullptr) or (data_bytes < max_data_bytes)) {
                picked = s;
                max_data_bytes = data_bytes;
            } else if ((std::abs<double>(data_bytes - max_data_bytes) < 0.00001) and (s->getHostname() > picked->getHostname())) {
                picked = s;
            }
        }
        return picked;
    };

    this->cluster_selection_schemes["lowest_watts"] = [] (const std::shared_ptr<wrench::WorkflowTask>& task, const std::set<std::shared_ptr<wrench::BareMetalComputeService>>& services) -> std::shared_ptr<wrench::BareMetalComputeService> {
        std::shared_ptr<wrench::BareMetalComputeService> picked = nullptr;
        double picked_watts = -1.0;
        for (auto const &s : services) {
            auto s4u_host = simgrid::s4u::Host::by_name(s->getPhysicalHostname());
            auto wattage_per_state = s4u_host->get_property("wattage_per_state");
            auto tokens = SimpleStandardJobScheduler::stringSplit(wattage_per_state, ',');
            double watts = std::atof(SimpleStandardJobScheduler::stringSplit(tokens.at(0), ':').at(1).c_str());

            if ((picked == nullptr) or (watts < picked_watts)) {
                picked = s;
                picked_watts = watts;
            } else if ((std::abs<double>(picked_watts - watts) < 0.0001) and (s->getHostname() > picked->getHostname())) {
                picked = s;
            }
        }
        return picked;
    };

    this->cluster_selection_schemes["lowest_watts_per_flops"] = [this] (const std::shared_ptr<wrench::WorkflowTask>& task, const std::set<std::shared_ptr<wrench::BareMetalComputeService>>& services) -> std::shared_ptr<wrench::BareMetalComputeService> {
        std::shared_ptr<wrench::BareMetalComputeService> picked = nullptr;
        double picked_watts_per_flops = -1.0;
        for (auto const &s : services) {
            auto s4u_host = simgrid::s4u::Host::by_name(s->getPhysicalHostname());
            auto wattage_per_state = s4u_host->get_property("wattage_per_state");
            auto tokens = SimpleStandardJobScheduler::stringSplit(wattage_per_state, ',');
            double watts = std::atof(SimpleStandardJobScheduler::stringSplit(tokens.at(0), ':').at(1).c_str());
            double flops = this->core_flop_rate_map[s];
            double flops_per_watts = flops / watts;

            if ((picked == nullptr) or (flops_per_watts < picked_watts_per_flops)) {
                picked = s;
                picked_watts_per_flops = flops_per_watts;
            } else if ((std::abs<double>(picked_watts_per_flops - flops_per_watts) < 0.0001) and (s->getHostname() > picked->getHostname())) {
                picked = s;
            }
        }
        return picked;
    };

    this->cluster_selection_schemes["random"] = [this] (const std::shared_ptr<wrench::WorkflowTask>& task, const std::set<std::shared_ptr<wrench::BareMetalComputeService>>& services) -> std::shared_ptr<wrench::BareMetalComputeService> {
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

    this->cluster_selection_schemes["most_idle_cpu_resources"] = [this] (const std::shared_ptr<wrench::WorkflowTask>& task, const std::set<std::shared_ptr<wrench::BareMetalComputeService>>& services) -> std::shared_ptr<wrench::BareMetalComputeService> {
        std::shared_ptr<wrench::BareMetalComputeService> picked = nullptr;
        double best=0;
        for (auto const &s : services) {
            double power=0;
            auto idleHosts= this->idle_cores_map[s];
            auto cores= this->core_flop_rate_map[s];
            for(auto const& host : idleHosts){
                power+=((double)host.second * cores);
            }
            if(picked == nullptr or power > best){
                best = power;
                picked = s;
            } else if ((std::abs<double>(power - best) < 0.00001) and (s->getHostname() > picked->getHostname())) {
                picked = s;
            }
        }
        return picked;
    };

}
