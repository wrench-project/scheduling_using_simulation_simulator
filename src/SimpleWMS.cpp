/**
* Copyright (c) 2020. <ADD YOUR HEADER INFORMATION>.
* Generated with the wrench-init.in tool.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*/
#include <iostream>
#include <utility>
#include <sys/wait.h>

#include "SimpleWMS.h"
#include "SimpleStandardJobScheduler.h"

XBT_LOG_NEW_DEFAULT_CATEGORY(simple_wms, "Log category for Simple WMS");

/**
* @brief Create a Simple WMS with a workflow instance, a scheduler implementation, and a list of compute services
*/
SimpleWMS::SimpleWMS(SimpleStandardJobScheduler *scheduler,
                     std::shared_ptr<wrench::Workflow> workflow,
                     double first_scheduler_change_trigger,
                     double periodic_scheduler_change_trigger,
                     double speculative_work_fraction,
                     std::string &simulation_noise_scheme,
                     double simulation_noise,
                     int noise_seed,
                     std::string &algorithm_selection_scheme,
                     std::set<std::shared_ptr<wrench::BareMetalComputeService>> compute_services,
                     std::set<std::shared_ptr<wrench::StorageService>> storage_services,
                     std::shared_ptr<wrench::FileRegistryService> file_registry_service,
                     const std::string &hostname) : wrench::ExecutionController(
        hostname,
        "simple"),
                                                    workflow(std::move(workflow)),
                                                    scheduler(scheduler),
                                                    first_scheduler_change_trigger(first_scheduler_change_trigger),
                                                    periodic_scheduler_change_trigger(periodic_scheduler_change_trigger),
                                                    speculative_work_fraction(speculative_work_fraction),
                                                    simulation_noise_scheme(simulation_noise_scheme),
                                                    simulation_noise(simulation_noise),
                                                    noise_seed(noise_seed),
                                                    algorithm_selection_scheme(algorithm_selection_scheme),
                                                    compute_services(std::move(compute_services)),
                                                    storage_services(std::move(storage_services)),
                                                    file_registry_service(std::move(file_registry_service))
{
}

/**
* @brief main method of the SimpleWMS daemon
*/
int SimpleWMS::main() {

    std::uniform_real_distribution<double> random_dist(-simulation_noise, simulation_noise);
    std::mt19937 rng(noise_seed);

    wrench::TerminalOutput::setThisProcessLoggingColor(wrench::TerminalOutput::COLOR_GREEN);

    WRENCH_INFO("About to execute a workflow with %lu tasks", this->workflow->getNumberOfTasks());

    // Set all the pstates to the "middle" value
    for (auto const &h : wrench::Simulation::getHostnameList()) {
        this->simulation->setPstate(h, ((int)wrench::Simulation::getListOfPstates(h).size()-1)/2); // (2*n+1 -1) / 2
    }
    // Create a job manager
    this->job_manager = this->createJobManager();
    this->data_movement_manager = this->createDataMovementManager();

    // Initialize the scheduler
    this->scheduler->init(this->job_manager,
                          this->compute_services,
                          this->storage_services,
                          this->file_registry_service,
                          wrench::Simulation::getHostName());

    // Compute the total work
    double total_work = 0.0;
    for (auto const &t : this->workflow->getTasks()) {
        total_work += t->getFlops();
    }

    // Set of workflow levels that have already become ready
    std::set<unsigned long> already_ready_levels;
    already_ready_levels.insert(0);

    // Pipe used for communication with child
    int pipefd[2];

    // Compute noisy tasks and file sizes that children will use and thus
    // incur simulation errors (for the micro-application scheme)
    std::unordered_map<std::shared_ptr<wrench::DataFile>, double> noisy_file_sizes;
    std::unordered_map<std::shared_ptr<wrench::WorkflowTask>, double> noisy_task_flops;
    if (this->simulation_noise_scheme == "micro-application") {
        for (auto const &f : workflow->getFileMap()) {
            double size = f.second->getSize();
            size = std::max<double>(0, size + size * random_dist(rng));
            noisy_file_sizes[f.second] = size;
        }
        for (auto const &t : workflow->getTasks()) {
            double flops = t->getFlops();
            flops = std::max<double>(0, flops + flops * random_dist(rng));
            noisy_task_flops[t] = flops;
        }
    }
    // Compute noisy host pstates and link bandwidth that children will use
    // and thus incur simulation error (for the micro-platform scheme)
    std::unordered_map<std::string, int> noisy_host_pstates;
    std::unordered_map<std::string, double> noisy_link_bandwidths;
    if (this->simulation_noise_scheme == "micro-platform") {
        for (auto const &h : wrench::Simulation::getHostnameList()) {
            int num_pstates = wrench::Simulation::getNumberofPstates(h);
            int base_pstate = (num_pstates -1)/2;
            int new_pstate = base_pstate + (int)(random_dist(rng) * (num_pstates -1)/2);
            new_pstate = std::min<int>(new_pstate, num_pstates - 1);
            new_pstate = std::max<int>(new_pstate, 0);
            noisy_host_pstates[h] = new_pstate;
//            std::cerr << "PSTATE BASE WAS " << base_pstate << " ---> " << new_pstate << "\n";
        }
        for (auto const &l : wrench::Simulation::getLinknameList()) {
            double bandwidth = wrench::Simulation::getLinkBandwidth(l);
            bandwidth = std::max<double>(0, bandwidth + bandwidth * random_dist(rng));
            noisy_link_bandwidths[l] = bandwidth;
        }
    }

    while (true) {
        // Scheduler change?

        bool speculation_can_happen = ((not this->i_am_speculative) and (this->scheduler->getEnabledSchedulingAlgorithms().size() > 1));
        bool should_do_first_change = ((not this->one_schedule_change_has_happened) and (this->work_done_since_last_scheduler_change >= this->first_scheduler_change_trigger * total_work));
        bool should_do_next_change;

        if (periodic_scheduler_change_trigger >= 0) {
            // Work-based selection trigger
            should_do_next_change = this->one_schedule_change_has_happened and (this->work_done_since_last_scheduler_change > this->periodic_scheduler_change_trigger * total_work);
        } else {
            // Level-based selection trigger
            bool new_level_became_ready = false;
            for (auto const &t : this->workflow->getReadyTasks()) {
                if (already_ready_levels.find(t->getTopLevel()) == already_ready_levels.end()) {
                    new_level_became_ready = true;
                    already_ready_levels.insert(t->getTopLevel());
                }
            }
            should_do_next_change = this->one_schedule_change_has_happened and (new_level_became_ready);
        }

//        std::cerr << "speculation can happen: " << speculation_can_happen << "\n";
//        std::cerr << "should_do_first_change: " << should_do_first_change << "\n";
//        std::cerr << "should_do_next_change: " << should_do_next_change << "\n";

        if (speculation_can_happen and (should_do_first_change or should_do_next_change)) {
            this->one_schedule_change_has_happened = true;
            this->work_done_since_last_scheduler_change = 0.0;
//            std::cerr << "Exploring scheduling algorithm futures speculatively... \n";
            std::cerr.flush();
            std::vector<std::pair<double, double>> makespans_and_energies;

            for (auto const &algorithm_index : this->scheduler->getEnabledSchedulingAlgorithms()) {
                pipe(pipefd);
                auto pid = fork();
                if (!pid) {
                    // Make the child mute
                    close(STDOUT_FILENO);
                    // Close the read end of the pipe
                    close(pipefd[0]);
//                    std::cerr <<  "Child exploring algorithm "  << algorithm_index << " (" <<  this->scheduler->schedulingAlgorithmToString(algorithm_index) << ")\n";
                    this->scheduler->useSchedulingAlgorithm(algorithm_index);
                    this->i_am_speculative = true;
                    // Apply all noise if micro
                    if (this->simulation_noise_scheme == "micro-application") {
                        for (auto const &f : workflow->getFileMap()) {
                            f.second->setSize(noisy_file_sizes[f.second]);
                        }
                        for (auto const &t : workflow->getTasks()) {
                            t->setFlops(noisy_task_flops[t]);
                        }
                    } else if (this->simulation_noise_scheme == "micro-platform") {
                        for (auto const &h : noisy_host_pstates) {
                            this->simulation->setPstate(h.first, h.second);
                        }
                        for (auto const &l : noisy_link_bandwidths) {
                            simgrid::s4u::Link *link = simgrid::s4u::Engine::get_instance()->link_by_name(l.first);
                            link->set_bandwidth(l.second);
                        }
                    }

                    break;
                } else {
                    // Parent
                    close(pipefd[1]);
                    int stat_loc;
                    double child_time;
                    double child_energy;
                    read(pipefd[0], &child_time, sizeof(double));
//                    std::cerr << "Child told me: " << child_time << "\n";
                    if (this->simulation_noise_scheme == "macro") {
                        child_time = child_time + child_time * random_dist(rng);
                    }
                    read(pipefd[0], &child_energy, sizeof(double));
                    makespans_and_energies.emplace_back(child_time, child_energy);
                    waitpid(pid, &stat_loc, 0);
                }
            }
            if (not this->i_am_speculative) {
//                for (int i=0; i < makespans_and_energies.size(); i++) {
//                    std::cerr << i << " " << makespans_and_energies.at(i).first << " " << makespans_and_energies.at(i).second << "\n";
//                }
                unsigned long argmin;
                if (this->algorithm_selection_scheme == "makespan") {
                    argmin = std::min_element(makespans_and_energies.begin(), makespans_and_energies.end(),
                                              [](std::pair<double, double> a, std::pair<double, double> b) {
                                                  return a.first < b.first;
                                              }) - makespans_and_energies.begin();

                } else if (this->algorithm_selection_scheme == "energy") {
                    argmin = std::min_element(makespans_and_energies.begin(), makespans_and_energies.end(),
                                              [](std::pair<double, double> a, std::pair<double, double> b) {
                                                  return a.second < b.second;
                                              }) - makespans_and_energies.begin();
                } else {
                    throw std::runtime_error("Unknown algorithm selection scheme: " + this->algorithm_selection_scheme);
                }

//                std::cerr << "ARGMIN = " << argmin << "\n";

                double makespan = std::get<0>(makespans_and_energies.at(argmin));
                unsigned long algorithm_index = this->scheduler->getEnabledSchedulingAlgorithms().at(argmin);

                this->algorithm_sequence.push_back(algorithm_index);
                std::cerr << "Switching to algorithm " <<
                          "[" << (algorithm_index < 100 ? "0" : "") << (algorithm_index < 10 ? "0" : "") << algorithm_index << "] " <<
                          this->scheduler->schedulingAlgorithmToString(algorithm_index) << " (makespan = " << makespan << ")\n";

                this->scheduler->useSchedulingAlgorithm(algorithm_index);
            }
        }

        if ((this->i_am_speculative) and (this->work_done_since_last_scheduler_change > this->speculative_work_fraction * total_work)) {
            break;
        }

        // Get the ready tasks
        auto ready_tasks = this->workflow->getReadyTasks();

        // Run ready tasks with defined scheduler implementation
        this->scheduler->scheduleTasks(ready_tasks);

        // Wait for a workflow execution event, and process it
        try {
            this->waitForAndProcessNextEvent();
        } catch (wrench::ExecutionException &e) {
            WRENCH_INFO("Error while getting next execution event (%s)... ignoring and trying again",
                        (e.getCause()->toString().c_str()));
            continue;
        }

        if (this->workflow->isDone()) {
            break;
        }
    }

    if (this->i_am_speculative) {
        // Get current time
        double now = wrench::Simulation::getCurrentSimulatedDate();
        // Send it back to the parent
        write(pipefd[1], &now, sizeof(double));
        // Get the total energy consumption
        double energy = 0.0;
        for (auto const &h : wrench::Simulation::getHostnameList()) {
            if (h != "wms_host") {
                energy += this->simulation->getEnergyConsumed(h);
            }
        }
        // Send it back to the papent
        write(pipefd[1], &energy, sizeof(double));
        close(pipefd[1]);
//        std::cerr << "  CHILD RETURNING TO MAIN AFTER SENDING MAKESPAN " << now << " TO PARENT\n";
    }
    this->job_manager.reset();


    return 0;
}

void SimpleWMS::processEventStandardJobCompletion(std::shared_ptr<wrench::StandardJobCompletedEvent> event) {
    auto task = event->standard_job->getTasks().at(0);
//    std::cerr << "JOB COMPLETED: task " << task->getID() << " (" << task->getExecutionHost() << ", " << task->getNumCoresAllocated() << ")\n";
    this->work_done_since_last_scheduler_change += task->getFlops();
    auto created_files = task->getOutputFiles();
    auto cs = event->compute_service;
    std::shared_ptr<wrench::StorageService> target_ss;
    for (auto const &ss : this->storage_services) {
        if (ss->getHostname() == cs->getHostname()) {
            target_ss = ss;
            break;
        }
    }
    for (auto const &f : created_files) {
        this->file_registry_service->addEntry(f, wrench::FileLocation::LOCATION(target_ss));
    }

//    std::cerr << "UPDATING CORES[" << event->compute_service->getHostname() << "][" << task->getExecutionHost() << "] += " << task->getNumCoresAllocated() << "\n";
    this->scheduler->idle_cores_map[std::dynamic_pointer_cast<wrench::BareMetalComputeService>(event->compute_service)][task->getExecutionHost()] += task->getNumCoresAllocated();
}

void SimpleWMS::processEventStandardJobFailure(std::shared_ptr<wrench::StandardJobFailedEvent> event) {
    throw std::runtime_error("Job Failure shouldn't happen: " + event->toString());
}
