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
                     double simulation_noise_reduction,
                     double energy_bound,
                     std::string &algorithm_selection_scheme,
                     double simulation_overhead,
                     bool disable_contention,
                     bool disable_contention_in_speculative_executions,
                     bool disable_amdahl_in_speculative_executions,
                     bool disable_adaptation_if_noise_has_not_changed,
                     bool at_most_one_noise_reduction,
                     bool at_most_one_adaptation,
                     std::set<std::shared_ptr<wrench::BareMetalComputeService>> compute_services,
                     std::set<std::shared_ptr<wrench::StorageService>> storage_services,
                     const std::string &hostname,
                     double initial_load_max_duration,
                     int initial_load_duration_seed,
                     double initial_load_prob_core_loaded) : wrench::ExecutionController(
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
                                                    simulation_noise_reduction(simulation_noise_reduction),
                                                    energy_bound(energy_bound),
                                                    algorithm_selection_scheme(algorithm_selection_scheme),
                                                    simulation_overhead(simulation_overhead),
                                                    disable_contention(disable_contention),
                                                    disable_contention_in_speculative_executions(disable_contention_in_speculative_executions),
                                                    disable_amdahl_in_speculative_executions(disable_amdahl_in_speculative_executions),
                                                    disable_adaptation_if_noise_has_not_changed(disable_adaptation_if_noise_has_not_changed),
                                                    at_most_one_noise_reduction(at_most_one_noise_reduction),
                                                    at_most_one_adaptation(at_most_one_adaptation),
                                                    compute_services(std::move(compute_services)),
                                                    storage_services(std::move(storage_services)),
                                                    initial_load_max_duration(initial_load_max_duration),
                                                    initial_load_duration_seed(initial_load_duration_seed),
                                                    initial_load_prob_core_loaded(initial_load_prob_core_loaded) { }


void SimpleWMS::compute_micro_simulation_noise(wrench::Simulation *simulation,
                                               const std::shared_ptr<wrench::Workflow>& workflow,
                                               const std::string& noise_scheme,
                                               double previous_noise,
                                               double noise,
                                               double seed,
                                               std::unordered_map<std::shared_ptr<wrench::DataFile>, double> &noisy_file_sizes,
                                               std::unordered_map<std::shared_ptr<wrench::WorkflowTask>, double> &noisy_task_flops,
                                               std::unordered_map<std::string, int> &noisy_host_pstates,
                                               std::unordered_map<std::string, double> &noisy_link_bandwidths) {

    if (noise <= 0.0) return;


    if (previous_noise < 0.0) {

//        std::cerr << "COMPUTING INITIAL NOISE\n";

        std::uniform_real_distribution<double> random_dist(-noise, noise);
        std::mt19937 rng(seed);

        // Compute noisy tasks and file sizes that children will use and thus
        // incur simulation errors (for the micro-application scheme)
        if (noise_scheme == "micro-application") {

            for (auto const &f: workflow->getFileMap()) {
                double size = f.second->getSize();
                size = std::max<double>(0, size + size * random_dist(rng));
                noisy_file_sizes[f.second] = size;
            }
            for (auto const &t: workflow->getTasks()) {
                double flops = t->getFlops();
                flops = std::max<double>(0, flops + flops * random_dist(rng));
                noisy_task_flops[t] = flops;
            }

        }  else if (noise_scheme == "micro-platform") {
            // Compute noisy host pstates (homogeneous within a cluster) and link bandwidth that children will use
            // and thus incur simulation error (for the micro-platform scheme)
            // Note that this uses 100 pstates to make it possible to noisy-fy host speeds

//        std::cerr << "MICRO APPLYNG NOISE " << noise << "\n";
            auto cluster_map = wrench::Simulation::getHostnameListByCluster();
            for (auto const &c: cluster_map) {
                int num_pstates = wrench::Simulation::getNumberofPstates(c.second.at(0));
                int base_pstate = (num_pstates - 1) / 2;
                int new_pstate = base_pstate + (int) (random_dist(rng) * (num_pstates - 1) / 2);
                new_pstate = std::min<int>(new_pstate, num_pstates - 1);
                new_pstate = std::max<int>(new_pstate, 0);
                for (auto const &h: c.second) {
                    noisy_host_pstates[h] = new_pstate;
                }
            }

            for (auto const &l: wrench::Simulation::getLinknameList()) {
                double bandwidth = wrench::Simulation::getLinkBandwidth(l);
                bandwidth = std::max<double>(0, bandwidth + bandwidth * random_dist(rng));
                noisy_link_bandwidths[l] = bandwidth;
            }
        }

    } else {

        if (noise == previous_noise) {
//            std::cerr << "NEW NOISE IS SAME AS OLD NOISE - NOT RECOMPUTING MITIGATED NOISE\n";
            return;
        }

//        std::cerr << "COMPUTING MITIGATED NOISE\n";

        if (noise_scheme == "micro-application") {
            throw std::runtime_error("micro-application noise mitigation not implemented yet!");

        } else if (noise_scheme == "micro-platform") {
            // Compute noisy host pstates (homogeneous within a cluster) and link bandwidth that children will use
            // and thus incur simulation error (for the micro-platform scheme)
            // Note that this uses 100 pstates to make it possible to noisy-fy host speeds

//            std::cerr << "COMPUTING NOISE MITIGATION\n";

            auto cluster_map = wrench::Simulation::getHostnameListByCluster();
            for (auto const &c: cluster_map) {
                for (auto const &h: c.second) {
                    int num_pstates = wrench::Simulation::getNumberofPstates(h);
                    int current_pstate = noisy_host_pstates[h];
//                    std::cerr << "current_pstate = " << current_pstate << " / "  << num_pstates << "\n";
                    int base_pstate = (num_pstates - 1) / 2;
                    int delta_from_base = current_pstate - base_pstate;
//                    std::cerr << "delta_from_base = " << delta_from_base << "\n";
                    int new_pstate = base_pstate + (int)((double)delta_from_base * (noise / previous_noise));
                    new_pstate = std::min<int>(new_pstate, num_pstates - 1);
                    new_pstate = std::max<int>(new_pstate, 0);
//                    std::cerr << "new_pstate = " << new_pstate << "\n";
                    noisy_host_pstates[h] = new_pstate;
                }
            }

            for (auto const &l: wrench::Simulation::getLinknameList()) {
                double current_bandwidth = noisy_link_bandwidths[l];
//                std::cerr << "current_bandwidth = " << current_bandwidth << "\n";
                double original_bandwidth = this->original_link_bandwidths[l];
//                std::cerr << "original_bandwidth = " << original_bandwidth << "\n";
                double delta = (current_bandwidth - original_bandwidth);
//                std::cerr << "delta = " << delta << "\n";
                noisy_link_bandwidths[l] = original_bandwidth + delta * (noise / previous_noise);
//                std::cerr << "new_bandwidth = " << noisy_link_bandwidths[l] << "\n";
            }

        }
    }
}



void SimpleWMS::apply_micro_simulation_noise(wrench::Simulation *simulation,
                                             const std::shared_ptr<wrench::Workflow>& workflow,
                                             const std::string& noise_scheme,
                                             double previous_noise,
                                             double noise,
                                             double seed,
                                             std::unordered_map<std::shared_ptr<wrench::DataFile>, double> &noisy_file_sizes,
                                             std::unordered_map<std::shared_ptr<wrench::WorkflowTask>, double> &noisy_task_flops,
                                             std::unordered_map<std::string, int> &noisy_host_pstates,
                                             std::unordered_map<std::string, double> &noisy_link_bandwidths) {

    if (noise <= 0.0) return;

    if (noise_scheme == "micro-application") {

//        std::cerr << "APPLYING MICRO NOISE\n";

        for (auto const &f: workflow->getFileMap()) {
            f.second->setSize(noisy_file_sizes[f.second]);
        }
        for (auto const &t: workflow->getTasks()) {
            t->setFlops(noisy_task_flops[t]);
        }

    }  else if (noise_scheme == "micro-platform") {
        // Compute noisy host pstates (homogeneous within a cluster) and link bandwidth that children will use
        // and thus incur simulation error (for the micro-platform scheme)
        // Note that this uses 100 pstates to make it possible to noisy-fy host speeds

        for (auto const &h: noisy_host_pstates) {
            simulation->setPstate(h.first, h.second);
//                std::cerr << "  SETTING: " << h.first << " to pstate << " << h.second << "\n";
        }
        for (auto const &l: noisy_link_bandwidths) {
            simgrid::s4u::Link *link = simgrid::s4u::Engine::get_instance()->link_by_name(l.first);
            link->set_bandwidth(l.second);
//                std::cerr << "  SETTING: " << l.first << " to bandwdith << " << l.second << "\n";
        }
    }

}

/**
* @brief main method of the SimpleWMS daemon
*/
int SimpleWMS::main() {

    if (this->disable_contention) {
        for (auto const &link : simgrid::s4u::Engine::get_instance()->get_all_links()) {
            link->set_sharing_policy(simgrid::s4u::Link::SharingPolicy::FATPIPE);
        }
    }

    auto macro_random_dist = new std::uniform_real_distribution<double>(-this->simulation_noise, this->simulation_noise);
    auto macro_rng = new std::mt19937(this->noise_seed);
    (*macro_random_dist)(*macro_rng);

    // Set up original link bandwidth map
    for (auto const &l: wrench::Simulation::getLinknameList()) {
        simgrid::s4u::Link *link = simgrid::s4u::Engine::get_instance()->link_by_name(l);
        original_link_bandwidths[l] = link->get_bandwidth();
    }

    std::unordered_map<std::shared_ptr<wrench::DataFile>, double> noisy_file_sizes;
    std::unordered_map<std::shared_ptr<wrench::WorkflowTask>, double> noisy_task_flops;
    std::unordered_map<std::string, int> noisy_host_pstates;
    std::unordered_map<std::string, double> noisy_link_bandwidths;

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
                          wrench::Simulation::getHostName(),
                          initial_load_max_duration,
                          initial_load_duration_seed,
                          initial_load_prob_core_loaded);

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

    bool simulation_noise_has_changed = true;
    double previous_noise = -1.0;
    int num_adaptations = 0;

    /* Main simulation loop */
    while (true) {

        // Scheduler change?
        bool speculation_can_happen = ((not this->i_am_speculative) and
                                       (this->scheduler->getNumEnabledSchedulingAlgorithms() > 1));
        if (this->disable_adaptation_if_noise_has_not_changed and (not simulation_noise_has_changed)) {
            speculation_can_happen = false;
        }
        if ((num_adaptations >= 2) and this->at_most_one_adaptation) {
            speculation_can_happen = false;
        }

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

//        std::cerr << "should_do_first_change: " << should_do_first_change << "\n";
//        std::cerr << "speculation can happen: " << speculation_can_happen << "\n";
//        std::cerr << "should_do_next_change: " << should_do_next_change << "\n";

        if (speculation_can_happen and (should_do_first_change or should_do_next_change)) {
            num_adaptations += 1;

            compute_micro_simulation_noise(this->simulation, workflow, this->simulation_noise_scheme,
                                           previous_noise, this->simulation_noise, this->noise_seed,
                                           noisy_file_sizes, noisy_task_flops, noisy_host_pstates,
                                           noisy_link_bandwidths);


            this->work_done_since_last_scheduler_change = 0.0;
//            std::cerr << "[" << wrench::Simulation::getCurrentSimulatedDate() << "] PARENT: " << getpid() << " Exploring scheduling algorithm futures speculatively... \n";
//            std::cerr.flush();
            std::vector<std::pair<double, double>> makespans_and_energies;

            delete macro_random_dist;
            delete macro_rng;
            macro_random_dist = new std::uniform_real_distribution<double>(-this->simulation_noise, this->simulation_noise);
            macro_rng = new std::mt19937(this->noise_seed);

            for (int algorithm_index = 0; algorithm_index < this->scheduler->getNumEnabledSchedulingAlgorithms(); algorithm_index++) {
                auto err = pipe(pipefd);
                if (err < 0) {
                    throw std::runtime_error("pipe() failed!: " + std::to_string(errno));
                }
                auto pid = fork();
                if (!pid) {


                    // Make the child mute
//                    std::cerr <<  "Child exploring algorithm "  << algorithm_index << " (" <<  this->scheduler->algorithmIndexToString(algorithm_index) << ")\n";
                    close(STDOUT_FILENO);
                    close(STDERR_FILENO);
                    // Close the read end of the pipe
                    close(pipefd[0]);
                    this->scheduler->useSchedulingAlgorithmNow(algorithm_index);
                    this->i_am_speculative = true;

                    // Apply all micro noise, if any
//                    std::cerr << "APPLYING SIMUILATION NOISE " << this->simulation_noise << "\n";
                    apply_micro_simulation_noise(this->simulation, workflow, this->simulation_noise_scheme,
                                                 previous_noise,
                                                 this->simulation_noise, this->noise_seed,
                                                 noisy_file_sizes, noisy_task_flops, noisy_host_pstates, noisy_link_bandwidths);



                    // Disable contention if need be
                    if (this->disable_contention_in_speculative_executions) {
                        for (auto const &link : simgrid::s4u::Engine::get_instance()->get_all_links()) {
                            link->set_sharing_policy(simgrid::s4u::Link::SharingPolicy::FATPIPE);
                        }
                    }

                    // Disable amdahl if need be
                    if (this->disable_amdahl_in_speculative_executions) {
                        for (auto const &task : this->workflow->getTasks())  {
                            //  task->setParallelModel(wrench::ParallelModel::CONSTANTEFFICIENCY(1.0));
                            auto model = std::dynamic_pointer_cast<wrench::AmdahlParallelModel>(task->getParallelModel());
                            model->setAlpha(1.0);
                        }
                    }

                    break;
                } else {
                    // Parent
                    close(pipefd[1]);
                    int stat_loc;
                    double child_time;
                    double child_energy;
                    auto err = read(pipefd[0], &child_time, sizeof(double));
                    if (err < 0) {
                        throw std::runtime_error("read() failed!: " + std::to_string(errno));
                    }
                    //std::cerr << "Child told me: " << child_time << "\n";
                    if (this->simulation_noise_scheme == "macro") {
                        child_time = child_time + child_time * (*macro_random_dist)(*macro_rng);
                    }
                    err = read(pipefd[0], &child_energy, sizeof(double));
                    if (err < 0) {
                        throw std::runtime_error("read() failed!: " + std::to_string(errno));
                    }
                    makespans_and_energies.emplace_back(child_time, child_energy);
                    waitpid(pid, &stat_loc, 0);
                }
            }

            if (not this->i_am_speculative) {
                for (int i=0; i < makespans_and_energies.size(); i++) {
                    std::cerr << "Alg " << i << " " << makespans_and_energies.at(i).first << std::endl;
                }

//                std::cerr << "MASTER: REDUCING BY " << this->simulation_noise_reduction << "\n";
//                std::cerr << "CURRENT SIMULATION NOISE " << this->simulation_noise << "\n";
                previous_noise = this->simulation_noise;
                double current_noise = this->simulation_noise;
                this->simulation_noise = std::max<double>(0, this->simulation_noise - this->simulation_noise_reduction);
                if (this->at_most_one_noise_reduction) {
                    this->simulation_noise_reduction = 0.0;
                }
                simulation_noise_has_changed = (std::abs<double>(current_noise - this->simulation_noise) > 0.0001);
//                std::cerr << "NEW SIMULATION NOISE " << this->simulation_noise << "\n";

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
                } else if (this->algorithm_selection_scheme == "makespan_over_energy") {
                    std::vector<double> ratios;
                    ratios.reserve(makespans_and_energies.size());
                    for (auto const &me : makespans_and_energies) {
                        ratios.push_back(me.second / me.first); // compute energy / makespan
                    }
                    argmin = std::min_element(ratios.begin(), ratios.end(),
                                              [](double a, double b) {
                                                  return a < b;
                                              }) - ratios.begin();
                } else if (this->algorithm_selection_scheme == "makespan_given_energy_bound") {
                    argmin = ULONG_MAX;
                    for (unsigned long i=0; i < makespans_and_energies.size(); i++) {
                        if (makespans_and_energies.at(i).second > this->energy_bound) {
                            continue;
                        }
                        if (argmin == ULONG_MAX) {
                            argmin = i;
                        } else {
                            if (makespans_and_energies.at(argmin).first > makespans_and_energies.at(i).first) {
                                argmin = i;
                            }
                        }
                    }
                    if (argmin == ULONG_MAX) {
                        // A Makespan of -1 will be returned, somehow!
                        std::cerr << "Not suitable algorithm was found because none can meet the energy bound\n";
                        return 1;
                    }

                } else {
                    throw std::runtime_error("Unknown algorithm selection scheme: " + this->algorithm_selection_scheme);
                }

                double makespan = std::get<0>(makespans_and_energies.at(argmin));
                double energy = std::get<1>(makespans_and_energies.at(argmin));
                unsigned long algorithm_index = argmin;

                this->algorithm_sequence.push_back(algorithm_index);
                std::cerr << "[" + std::to_string(wrench::Simulation::getCurrentSimulatedDate()) + "] Deciding to switch to algorithm " <<
                          "[" << (algorithm_index < 100 ? "0" : "") << (algorithm_index < 10 ? "0" : "") << algorithm_index << "] " <<
                          this->scheduler->algorithmIndexToString(algorithm_index) << " (makespan = " << makespan << ", " <<
                          "energy = " << energy << ")\n";

                double simulation_delay = this->simulation_overhead *
                                          (double)(this->scheduler->getNumEnabledSchedulingAlgorithms());

                if (this->one_schedule_change_has_happened) {
                    this->scheduler->useSchedulingAlgorithmThen(algorithm_index,
                                                                wrench::Simulation::getCurrentSimulatedDate() +
                                                                simulation_delay);
                } else {
                    wrench::Simulation::sleep(simulation_delay);
                    this->scheduler->useSchedulingAlgorithmThen(algorithm_index, wrench::Simulation::getCurrentSimulatedDate());
                }
                this->one_schedule_change_has_happened = true;
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
        auto err = write(pipefd[1], &now, sizeof(double));
        if (err < 0) {
            throw std::runtime_error("write() failed!: " + std::to_string(errno));
        }
        // Get the total energy consumption
        double energy = 0.0;
        for (auto const &h : wrench::Simulation::getHostnameList()) {
            if (h != "wms_host") {
                energy += this->simulation->getEnergyConsumed(h);
            }
        }
        // Send it back to the parent
        err = write(pipefd[1], &energy, sizeof(double));
        if (err < 0) {
            throw std::runtime_error("write() failed!: " + std::to_string(errno));
        }
        close(pipefd[1]);
//        std::cerr << "  CHILD RETURNING TO MAIN AFTER SENDING MAKESPAN " << now << " TO PARENT\n";
    }
    this->job_manager.reset();


    return 0;
}

void SimpleWMS::processEventStandardJobCompletion(std::shared_ptr<wrench::StandardJobCompletedEvent> event) {
    auto task = event->standard_job->getTasks().at(0);
//    std::cerr << wrench::Simulation::getCurrentSimulatedDate() << ": COMPLETED: " << task->getID() << ": " << task->getNumCoresAllocated() << "\n";
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
        this->scheduler->file_replica_locations[f].insert(target_ss);
    }

//    std::cerr << "UPDATING CORES[" << event->compute_service->getHostname() << "][" << task->getExecutionHost() << "] += " << task->getNumCoresAllocated() << "\n";
    this->scheduler->idle_cores_map[std::dynamic_pointer_cast<wrench::BareMetalComputeService>(event->compute_service)][task->getExecutionHost()] += task->getNumCoresAllocated();
}

void SimpleWMS::processEventStandardJobFailure(std::shared_ptr<wrench::StandardJobFailedEvent> event) {
    throw std::runtime_error("Job Failure shouldn't happen: " + event->toString());
}
