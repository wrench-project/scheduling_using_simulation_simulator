/**
 * Copyright (c) 2020. <ADD YOUR HEADER INFORMATION>.
 * Generated with the wrench-init.in tool.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
#include <wrench.h>
#include "SimpleStandardJobScheduler.h"
#include "SimpleWMS.h"
#include "PlatformCreator.h"
#include <memory>
#include <boost/program_options.hpp>
#include <random>
#include <nlohmann/json.hpp>


namespace po = boost::program_options;

int main(int argc, char **argv) {

    // Declaration of the top-level WRENCH simulation object
    auto simulation = wrench::Simulation::createSimulation();

    // Initialization of the simulation
    simulation->init(&argc, argv);

    // Generic lambda to check if a numeric argument is in some range
    auto in = [](const auto &min, const auto &max, char const * const opt_name) {
        return [opt_name, min, max](const auto &v){
            if (v < min || v > max) {
                throw std::invalid_argument("Argument value for --" + std::string(opt_name) + " should be between " + std::to_string(min) + " and " + std::to_string(max));
            }
        };
    };

    std::string workflow_file;

    // Creating the scheduler here so that we can grab the documentation for
    // all scheduling schemes
    auto scheduler = new SimpleStandardJobScheduler();

    std::string cluster_specs;
    std::string reference_flops;
    std::string task_selection_scheme_list;
    std::string cluster_selection_scheme_list;
    std::string core_selection_scheme_list;
    double first_scheduler_change_trigger;
    double periodic_scheduler_change_trigger;
    double speculative_work_fraction;
    double simulation_overhead;
    double min_task_parallel_efficiency;
    double max_task_parallel_efficiency;
    std::string simulation_noise_scheme;
    double simulation_noise;
    double simulation_noise_reduction;
    int random_algorithm_seed;
    int simulation_noise_seed;
    std::string algorithm_selection_scheme;
    double energy_bound;

    // Define command-line argument options
    po::options_description desc("Allowed options");
    desc.add_options()
            ("help",
             "Show this help message\n")
            ("workflow", po::value<std::string>(&workflow_file)->required()->value_name("<path>"),
             "Path to JSON workflow description file\n")
            ("min_task_parallel_efficiency", po::value<double>(&min_task_parallel_efficiency)->value_name("<parallel efficiency>")->default_value(0.5)->notifier(in(0.0, 1.0, "min_task_parallel_efficiency")),
             "The minimum task parallel efficiency"
             "(between 0.0 and 1, 0.0)\n")
            ("max_task_parallel_efficiency", po::value<double>(&max_task_parallel_efficiency)->value_name("<parallel efficiency>")->default_value(0.9)->notifier(in(0.0, 1.0, "max_task_parallel_efficiency")),
             "The maximum task parallel efficiency"
             "(between minimum parallel efficiency and 1, 0.0)\n")
            ("reference_flops", po::value<std::string>(&reference_flops)->required()->value_name("<ref flops>"),
             "Reference flop rate for the workflow file tasks (e.g., \"100Gf\" means that each second of computation in the JSON file corresponds to 100Gf)\n")
            ("clusters", po::value<std::string>(&cluster_specs)->required()->value_name("#nodes:#cores:flops:watts:io_bw:bw,#nodes:#cores:flops:watts:io_bw:internet_bw,..."),
             "Cluster specifications. Example: \"100:8:120Gf:200.0:100MBps:100MBps,10:4:200Gf:100.0:100MBps:25MBps\"\n")
            ("list_all_algorithms",
             "Print all scheduling algorithms available\n")
            ("print_JSON",
             "Print the JSON input configuration, without the actual simulation results\n")
//            ("algorithms", po::value<std::string>(&algorithm_list)->required()->value_name("<list of algorithm #>"),
//             "First one in the list will be used initially\nExample: --algorithms 0-4,12,15-17,19,21\n"
//             "(use --list_all_algorithms to see the list of algorithms)\n")
            ("task_selection_schemes", po::value<std::string>(&task_selection_scheme_list)->required()->value_name("<list of task selection schemes to use>"),
               "command-separated list of task selection schemes to use\n")
            ("cluster_selection_schemes", po::value<std::string>(&cluster_selection_scheme_list)->required()->value_name("<list of cluster selection schemes to use>"),
             "command-separated list of cluster selection schemes to use\n")
            ("core_selection_schemes", po::value<std::string>(&core_selection_scheme_list)->required()->value_name("<list of core selection schemes to use>"),
             "command-separated list of core selection schemes to use\n")
            ("random_algorithm_seed", po::value<int>(&random_algorithm_seed)->value_name("<random algorithm seed>")->default_value(42)->notifier(in(1, 200000, "random_algorithm_seed")),
             "The seed used for the RNG used for the random:random:random algorithm "
             "(between 1 and 200000)\n")
            ("first_scheduler_change_trigger", po::value<double>(&first_scheduler_change_trigger)->value_name("<work fraction>")->required()->notifier(in(0.0, 1.0, "first_scheduler_change_trigger")),
             "The algorithm may change for the first time once this fraction of the work has been performed "
             "(between 0.0 and 1, 0.0 meaning \"right away\" and 1.0 meaning \"never change\")\n")
            ("periodic_scheduler_change_trigger", po::value<double>(&periodic_scheduler_change_trigger)->value_name("<work fraction>")->required()->notifier(in(-1.0, 1.0, "periodic_scheduler_change_trigger")),
             "The algorithm may change each time this fraction of the work has been performed (between 0.0 and 1, 1 meaning \"never change\")\nIf <0, then changes occur each time the first task in a workflow level becomes ready!\n")
            ("speculative_work_fraction", po::value<double>(&speculative_work_fraction)->value_name("<work fraction>")->required()->notifier(in(0.0, 1.0, "speculative_work_fraction")),
             "The fraction of work that a speculative execution performs before reporting to the master process "
             "(between 0.0 and 1, 1 meaning \"until workflow completion\")\n")
            ("simulation_noise_scheme", po::value<std::string>(&simulation_noise_scheme)->required()->value_name("<simulation noise scheme>"),
             "(either 'macro' (makespan scaling), 'micro-application' (flops and byte scaling), 'micro-platform (cluster flop/sec and link byte/sec\n")
            ("simulation_overhead", po::value<double>(&simulation_overhead)->value_name("<simulation overhead in seconds>")->default_value(0.0)->notifier(in(0.0, 1000000, "per_algorithm_simulation_overhead")),
             "The overhead, in seconds, of simulating one future execution\n")
            ("simulation_noise", po::value<double>(&simulation_noise)->value_name("<simulation noise>")->default_value(0.0)->notifier(in(0.0, 1.0, "simulation_noise")),
             "The added uniformly distributed noise added to speculative simulation results "
             "(between 0.0 and 1, 0 meaning \"perfectly accurate\")\n")
            ("simulation_noise_reduction", po::value<double>(&simulation_noise_reduction)->value_name("<simulation noise reduction>")->default_value(0.0)->notifier(in(0.0, 1.0, "simulation_noise_reduction")),
             "The reduction of the simulation noise at each adaptation beyond the first one, i.e., noise = max(0, noise - reduction "
             "(between 0.0 and 1, 0.0 meaning \"no noise reduction\")\n")
            ("simulation_noise_seed", po::value<int>(&simulation_noise_seed)->value_name("<simulation noise seed>")->default_value(42)->notifier(in(1, 200000, "simulation_noise_seed")),
             "The seed used for the RNG that generates simulation noise "
             "(between 1 and 200000)\n")
            ("algorithm_selection_scheme", po::value<std::string>(&algorithm_selection_scheme)->required()->value_name("<algorithm selection scheme>"),
             "('makespan', 'energy', 'makespan_over_energy', 'makespan_given_energy_bound')\n")
            ("energy_bound", po::value<double>(&energy_bound)->value_name("<energy bound in Joules>")->default_value(-1.0),
            "An energy bound to not overcome\n")
            ("no-contention",
             "Disables network contention simulation (every link is a 'FATPIPE' in speculative executions)\n")
            ("adapt-only-if-noise-has-changed",
             "Disable scheduling algorithm adaptation if simulation noise hasn't changed\n")
            ("at-most-one-noise-reduction",
             "Reduce noise at most once\n")
            ;

    // Parse command-line arguments
    po::variables_map vm;
    po::store(
            po::parse_command_line(argc, argv, desc),
            vm
    );

    try {
        // Print help message and exit if needed
        if (vm.count("help")) {
            std::cerr << desc << "\n";
            exit(0);
        }
        if (vm.count("list_all_algorithms")) {
            std::cout << scheduler->getDocumentation() << "\n";
            exit(0);
        }
        // Throw whatever exception in case argument values are erroneous
        po::notify(vm);
    } catch (std::exception &e) {
        cerr << "Error: " << e.what() << "\n";
        exit(1);
    }

    // Set the random:random:random algorithm's seed
    scheduler->setRandomAlgorithmSeed(random_algorithm_seed);

    // Disable/enable contention
    bool disable_contention = vm.count("no-contention") > 0;

    // Disable adaption if noise hasn't changed
    bool disable_adaptation_if_noise_has_not_changed = vm.count("adapt-only-if-noise-has-changed") > 0;

    // Disable adaption if noise hasn't changed
    bool at_most_one_noise_reduction = vm.count("at-most-one-noise-reduction") > 0;

    // Check the noise scheme
    if (simulation_noise_scheme != "macro" and
        simulation_noise_scheme != "micro-application" and
        simulation_noise_scheme != "micro-platform") {
        throw std::invalid_argument("--simulation_noise_scheme value should be 'macro', 'micro-application', or 'micro-platform'");
    }

    // Check the algorithm selection scheme
    if ((algorithm_selection_scheme != "makespan") and
        (algorithm_selection_scheme != "energy") and
        (algorithm_selection_scheme != "makespan_over_energy") and
        (algorithm_selection_scheme != "makespan_given_energy_bound")) {
        throw std::invalid_argument("Invalid --algorithm_selection_scheme value");
    }

    // Check the task parallel efficiencies
    if (min_task_parallel_efficiency > max_task_parallel_efficiency) {
        throw std::invalid_argument("Minimum task parallel efficiency should be lower than maximum task parallel efficiency");
    }

    // Check the energy bound
    if ((algorithm_selection_scheme == "makespan_given_energy_bound") and (energy_bound < 0)) {
        throw std::invalid_argument("A positive energy_bound argument should be provided when using 'makespan_given_energy_bound' algorithm selection scheme");
    }

    // Enable all desired scheduling algorithms in the scheduler
    try {
        auto tokens = SimpleStandardJobScheduler::stringSplit(task_selection_scheme_list, ',');
        std::sort(tokens.begin(), tokens.end());
        task_selection_scheme_list = std::accumulate(tokens.begin(), tokens.end(), std::string(""),
                        [](const std::string &a, const std::string &b) { if (a.empty()) return a + b; else return a+","+b;});
        for (const auto &scheme : tokens) {
            scheduler->enableTaskSelectionScheme(scheme);
        }
        tokens = SimpleStandardJobScheduler::stringSplit(cluster_selection_scheme_list, ',');
        std::sort(tokens.begin(), tokens.end());
        cluster_selection_scheme_list = std::accumulate(tokens.begin(), tokens.end(), std::string(""),
                                                     [](const std::string &a, const std::string &b) { if (a.empty()) return a + b; else return a+","+b;});
        for (const auto &scheme : tokens) {
            scheduler->enableClusterSelectionScheme(scheme);
        }
        tokens = SimpleStandardJobScheduler::stringSplit(core_selection_scheme_list, ',');
        std::sort(tokens.begin(), tokens.end());
        core_selection_scheme_list = std::accumulate(tokens.begin(), tokens.end(), std::string(""),
                                                     [](const std::string &a, const std::string &b) { if (a.empty()) return a + b; else return a+","+b;});
        for (const auto &scheme : tokens) {
            scheduler->enableCoreSelectionScheme(scheme);
        }

        scheduler->finalizeEnabledAlgorithmList();

    } catch (std::invalid_argument &e) {
        std::cerr << "Error: " << e.what() << "\n\n";
        std::cerr << scheduler->getDocumentation() << "\n";
        exit(1);
    }

    // Create DETERMINISTIC JSON output
    nlohmann::json output_json;
    // Clusters
    auto tokens = SimpleStandardJobScheduler::stringSplit(cluster_specs, ',');
    std::sort(tokens.begin(), tokens.end());
    output_json["clusters"] = std::accumulate(tokens.begin(), tokens.end(), std::string(""),
                                              [](const std::string &a, const std::string &b) { if (a.empty()) return a + b; else return a+","+b;});
    // Algorithms
    output_json["task_selection_schemes"] =  task_selection_scheme_list;
    output_json["cluster_selection_schemes"] =  cluster_selection_scheme_list;
    output_json["core_selection_schemes"] = core_selection_scheme_list;

    // Workflow
    tokens = SimpleStandardJobScheduler::stringSplit(workflow_file, '/');
    output_json["workflow"] = tokens.at(tokens.size()-1);
    output_json["reference_flops"] = reference_flops;
    output_json["min_task_parallel_efficiency"] = min_task_parallel_efficiency;
    output_json["max_task_parallel_efficiency"] = max_task_parallel_efficiency;

    // Configs
    output_json["first_scheduler_change_trigger"] = first_scheduler_change_trigger;
    output_json["periodic_scheduler_change_trigger"] = periodic_scheduler_change_trigger;
    output_json["speculative_work_fraction"] = speculative_work_fraction;
    output_json["simulation_noise"] = simulation_noise;
    output_json["simulation_noise_reduction"] = simulation_noise_reduction;
    output_json["simulation_noise_seed"] = simulation_noise_seed;
    output_json["simulation_overhead"] = simulation_overhead;

    output_json["simulation_noise_scheme"] = simulation_noise_scheme;
    output_json["algorithm_selection_scheme"] = algorithm_selection_scheme;
    output_json["at_most_one_noise_reduction"] = at_most_one_noise_reduction;
    output_json["disable_adaptation_if_noise_has_not_changed"] = disable_adaptation_if_noise_has_not_changed;

    output_json["no_contention"] = disable_contention;

    if (vm.count("print_JSON")) {
        std::cout << output_json.dump() << std::endl;
        exit(0);
    }

    // Creation of the platform
    std::string wms_host = "wms_host";
    try {
        PlatformCreator platform_creator(wms_host, vm["clusters"].as<std::string>());
        simulation->instantiatePlatform(platform_creator);
    } catch (std::exception &e) {
        std::cerr << e.what() << "\n";
        exit(1);
    }

    // Start a BareMetal Service on each cluster, and a Storage Service on the head node
    std::set<std::shared_ptr<wrench::BareMetalComputeService>> compute_services;
    std::set<std::shared_ptr<wrench::StorageService>> storage_services;
    auto specs = SimpleStandardJobScheduler::stringSplit(cluster_specs,',');
    int counter = 0;
    for (const auto &spec : specs) {
        std::string name = "cluster_" + std::to_string(counter++);
        auto parsed_spec = PlatformCreator::parseClusterSpecification(spec);
        int num_hosts = std::get<0>(parsed_spec);
//        int num_cores = std::get<1>(parsed_spec);
//        std::string flops = std::get<2>(parsed_spec);
//        std::string watts = std::get<3>(parsed_spec);

        std::string head_node = name+"-head";
        std::vector<std::string> compute_nodes;
        compute_nodes.reserve(num_hosts);
        for (int i=0; i < num_hosts; i++) {
            compute_nodes.push_back(name+"-node-"+std::to_string(i));
        }

        compute_services.insert(simulation->add(
                new wrench::BareMetalComputeService(
                        head_node,
                        compute_nodes,
                        "/scratch",
                        {},
                        {})));

        storage_services.insert(simulation->add(
                new wrench::SimpleStorageService(
                        head_node,
                        {"/"},
                        {{wrench::SimpleStorageServiceProperty::BUFFER_SIZE, "1000000000"}},
                        {})));
    }

    // Set all time-outs to DBLMAX
    for (auto const &cs: compute_services) {
        cs->setNetworkTimeoutValue(DBL_MAX);
    }
    for (auto const &ss: storage_services) {
        ss->setNetworkTimeoutValue(DBL_MAX);
    }

    // Create a Storage Service on the WMS host
    auto wms_ss = simulation->add(new wrench::SimpleStorageService(wms_host, {"/"}, {{wrench::SimpleStorageServiceProperty::BUFFER_SIZE, "1000000000"}}, {}));
    storage_services.insert(wms_ss);
    wms_ss->setNetworkTimeoutValue(DBL_MAX);


    // Parse the workflow
    // As a performance optimization, in this whole simulator, instead of calling getMinNumCores() and getMaxNumCores(), we just
    // hardcode 1 and 64. Check out the macros.
    auto workflow = wrench::WfCommonsWorkflowParser::createWorkflowFromJSON(
            workflow_file, reference_flops, false, true, 1, 64, true);

    // Compute all task bottom levels, which is useful for some scheduling options
    scheduler->computeBottomLevels(workflow);
    scheduler->computeNumbersOfChildren(workflow);

    // Create the WMS
    auto wms = simulation->add(
            new SimpleWMS(scheduler,
                          workflow,
                          first_scheduler_change_trigger, periodic_scheduler_change_trigger, speculative_work_fraction,
                          simulation_noise_scheme, simulation_noise, simulation_noise_seed,
                          simulation_noise_reduction, energy_bound,
                          algorithm_selection_scheme, simulation_overhead,
                          disable_contention,
                          disable_adaptation_if_noise_has_not_changed,
                          at_most_one_noise_reduction,
                          compute_services, storage_services, wms_host));

    // Set the amdahl parameter for each task between 0.5 and 0.9
    std::uniform_real_distribution<double> random_dist(min_task_parallel_efficiency, max_task_parallel_efficiency);
    std::mt19937 rng(42);
    for (auto const &t : workflow->getTasks()) {
        t->setParallelModel(wrench::ParallelModel::AMDAHL(random_dist(rng)));
    }

    // Stage all input files on the WMS Storage Service
    for (const auto &f : workflow->getInputFiles()) {
//        simulation->stageFile(f, wms_ss);
        simulation->createFile(f, wrench::FileLocation::LOCATION(wms_ss));
        scheduler->file_replica_locations[f].insert(wms_ss);
    }

    simulation->getOutput().enableFileReadWriteCopyTimestamps(false);
    simulation->getOutput().enableWorkflowTaskTimestamps(false);
    // Launch the simulation
//    std::cerr << "Launching the Simulation..." << std::endl;
    auto simulation_begin_time = time(NULL);
    try {
        simulation->launch();
    } catch (std::runtime_error &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 0;
    }

    if (wms->i_am_speculative) {
        exit(0);
    }

    // Output
    output_json["simulation_time"] = time(NULL) - simulation_begin_time;

    output_json["makespan"] = workflow->getCompletionDate();

    std::string alg_sequence;
    for (auto const &a : wms->getAlgorithmSequence()) {
        alg_sequence += scheduler->algorithmIndexToString(a) + ",";
    }
    if (not alg_sequence.empty()) {
        alg_sequence.pop_back();
    }
    output_json["algorithm_sequence"] = alg_sequence;

    // Compute total energy
    double energy = 0.0;
    for (auto const &h : simulation->getHostnameList()) {
        if (h == "wms_host") continue;

        energy += simulation->getEnergyConsumed(h);
    }
    output_json["energy_consumed"] = energy;

    std::cout << output_json.dump() << std::endl;
    std::cerr << "MAKESPAN: " << output_json["makespan"] << "\n";
    std::cerr << "ENERGY: " << output_json["energy_consumed"] << "\n";

//    std::cerr << workflow->getCompletionDate() << "\n";

    return 0;
}

