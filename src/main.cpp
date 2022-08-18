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

    std::string scheduler_doc;
    scheduler_doc += "* Task selection schemes:\n";
    scheduler_doc += scheduler->getTaskPrioritySchemeDocumentation();
    scheduler_doc += "* Service selection schemes:\n";
    scheduler_doc += scheduler->getServiceSelectionSchemeDocumentation();
    scheduler_doc += "* Core selection schemes:\n";
    scheduler_doc += scheduler->getCoreSelectionSchemeDocumentation();

    std::string cluster_specs;
    std::string reference_flops;
    std::string algorithm_list;
    double first_scheduler_change_trigger;
    double periodic_scheduler_change_trigger;
    double speculative_work_fraction;
    std::string simulation_noise_scheme;
    double simulation_noise;
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
            ("reference_flops", po::value<std::string>(&reference_flops)->required()->value_name("<ref flops>"),
             "Reference flop rate for the workflow file tasks (e.g., \"100Gf\" means that each second of computation in the JSON file corresponds to 100Gf)\n")
            ("clusters", po::value<std::string>(&cluster_specs)->required()->value_name("#nodes:#cores:flops:watts:bw,#nodes:#cores:flops:watts:bw,..."),
             "Cluster specifications. Example: \"100:8:120Gf:200.0:100MBps,10:4:200Gf:100.0:25MBps\"\n")
            ("print_all_algorithms",
             "Print all scheduling algorithms available\n")
            ("print_JSON",
             "Print the JSON input configuration, without the actual simulation results\n")
            ("algorithms", po::value<std::string>(&algorithm_list)->required()->value_name("<list of algorithm #>"),
             "First one in the list will be used initially\nExample: --algorithms 0-4,12,15-17,19,21\n"
             "(use --print_all_algorithms to see the list of algorithms)\n")
            ("random_algorithm_seed", po::value<int>(&random_algorithm_seed)->value_name("<random algorithm seed>")->default_value(42)->notifier(in(1, 200000, "simulation_noise_seed")),
             "The seed used for the RNG used by the random:random:random algorithm "
             "(between 1 and 200000)")
            ("first_scheduler_change_trigger", po::value<double>(&first_scheduler_change_trigger)->value_name("<work fraction>")->default_value(1.0)->notifier(in(0.0, 1.0, "first_scheduler_change_trigger")),
             "The algorithm may change for the first time once this fraction of the work has been performed "
             "(between 0.0 and 1, 0.0 meaning \"right away\" and 1.0 meaning \"never change\")\n")
            ("periodic_scheduler_change_trigger", po::value<double>(&periodic_scheduler_change_trigger)->value_name("<work fraction>")->default_value(1.0)->notifier(in(-1.0, 1.0, "periodic_scheduler_change_trigger")),
             "The algorithm may change each time this fraction of the work has been performed (between 0.0 and 1, 1 meaning \"never change\")\nIf <0, then changes occur each time the first task in a workflow level becomes ready!\n")
            ("speculative_work_fraction", po::value<double>(&speculative_work_fraction)->value_name("<work fraction>")->default_value(1.0)->notifier(in(0.0, 1.0, "speculative_work_fraction")),
             "The fraction of work that a speculative execution performs before reporting to the master process "
             "(between 0.0 and 1, 1 meaning \"until workflow completion\")")
            ("simulation_noise_scheme", po::value<std::string>(&simulation_noise_scheme)->required()->value_name("<simulation noise scheme>"),
             "(either 'macro' (makespan scaling), 'micro-application' (flops and byte scaling), 'micro-platform' (flop/sec and link byte/sec)")
            ("simulation_noise", po::value<double>(&simulation_noise)->value_name("<simulation noise>")->default_value(0.0)->notifier(in(0.0, 1.0, "simulation_noise")),
             "The added uniformly distributed noise added to speculative simulation results "
             "(between 0.0 and 1, 0 meaning \"perfectly accurate\")")
            ("simulation_noise_seed", po::value<int>(&simulation_noise_seed)->value_name("<simulation noise seed>")->default_value(42)->notifier(in(1, 200000, "simulation_noise_seed")),
             "The seed used for the RNG that generates simulation noise "
             "(between 1 and 200000)")
            ("algorithm_selection_scheme", po::value<std::string>(&algorithm_selection_scheme)->required()->value_name("<algorithm selection scheme>"),
             "('makespan', 'energy', 'makespan_over_energy', 'makespan_given_energy_bound')")
            ("energy_bound", po::value<double>(&energy_bound)->value_name("<energy bound in Joules>")->default_value(-1.0),
            "An energy bound to not overcome\n")
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
        if (vm.count("print_all_algorithms")) {
            scheduler->printAllSchemes();
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

    // Check the energy bound
    if ((algorithm_selection_scheme == "makespan_given_energy_bound") and (energy_bound < 0)) {
        throw std::invalid_argument("A positive energy_bound argument should be provided when using 'makespan_given_energy_bound' algorithm selection scheme");
    }


    // Add all specified scheduling algorithms in oder to the scheduler
    std::vector<unsigned long> algorithm_index_list;
    {
        auto tokens = SimpleStandardJobScheduler::stringSplit(algorithm_list, ',');

        bool first = true;

        try {
            for (const auto &s_index : tokens) {
                unsigned long index_low, index_high;
                auto range_tokens = SimpleStandardJobScheduler::stringSplit(s_index, '-');

                index_low = std::strtol(range_tokens.at(0).c_str(), nullptr, 10);
                if (range_tokens.size() == 1) {
                    index_high = std::strtol(range_tokens.at(0).c_str(), nullptr, 10);
                } else if (range_tokens.size() == 2) {
                    index_high = std::strtol(range_tokens.at(1).c_str(), nullptr, 10);
                } else {
                    throw std::invalid_argument("Invalid algorithm list range item " + s_index);
                }
                if (index_low > index_high) {
                    throw std::invalid_argument("Invalid algorithm list range item " + s_index);
                }
                if (index_low >= scheduler->getNumAvailableSchedulingAlgorithms()) {
                    throw std::invalid_argument("Invalid algorithm index " + s_index);
                }
                if (index_high >= scheduler->getNumAvailableSchedulingAlgorithms()) {
                    throw std::invalid_argument("Invalid algorithm index " + s_index);
                }
                for (auto i = index_low; i <= index_high; i++) {
                    scheduler->enableSchedulingAlgorithm(i);
                    algorithm_index_list.push_back(i);
                    if (first) {
                        scheduler->useSchedulingAlgorithm(i);
                        first = false;
                    }
                }
            }
        } catch (std::invalid_argument &e) {
            std::cerr << "Error: invalid algorithm list: "<< e.what() << "\n";
            exit(1);
        }
    }

    // Create DETERMINISTIC JSON output
    nlohmann::json output_json;
    // Clusters
    auto tokens = SimpleStandardJobScheduler::stringSplit(cluster_specs, ',');
    std::sort(tokens.begin(), tokens.end());
    output_json["clusters"] = std::accumulate(tokens.begin(), tokens.end(), std::string(""),
                                              [](const std::string &a, const std::string &b) { if (a.empty()) return a + b; else return a+","+b;});
    // Algorithms
    std::sort(algorithm_index_list.begin(), algorithm_index_list.end());
    std::vector<std::string> algorithm_index_string_list;
    algorithm_index_string_list.reserve(algorithm_index_list.size());
    for (const auto &i : algorithm_index_list) {
        algorithm_index_string_list.push_back(std::to_string(i));
    }
    output_json["algorithms"] =  std::accumulate(algorithm_index_string_list.begin(), algorithm_index_string_list.end(), std::string(""),
                                                 [](const std::string &a, const std::string &b) { if (a.empty()) return a + b; else return a+","+b;});
    // Workflow
    tokens = SimpleStandardJobScheduler::stringSplit(workflow_file, '/');
    output_json["workflow"] = tokens.at(tokens.size()-1);
    output_json["reference_flops"] = reference_flops;

    // Configs
    output_json["first_scheduler_change_trigger"] = first_scheduler_change_trigger;
    output_json["periodic_scheduler_change_trigger"] = periodic_scheduler_change_trigger;
    output_json["speculative_work_fraction"] = speculative_work_fraction;
    output_json["simulation_noise"] = simulation_noise;
    output_json["simulation_noise_seed"] = simulation_noise_seed;

    output_json["simulation_noise_scheme"] = simulation_noise_scheme;
    output_json["algorithm_selection_scheme"] = algorithm_selection_scheme;

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


    // Create a file registry service
    auto file_registry_service = simulation->add(new wrench::FileRegistryService(wms_host));
    file_registry_service->setNetworkTimeoutValue(DBL_MAX);

    // Parse the workflow
    auto workflow = wrench::WfCommonsWorkflowParser::createWorkflowFromJSON(
            workflow_file, reference_flops, false, 1, 32, true);

    // Compute all task bottom levels, which is useful for some scheduling options
    scheduler->computeBottomLevels(workflow);
    scheduler->computeNumberOfChildren(workflow);

    // Create the WMS
    auto wms = simulation->add(
            new SimpleWMS(scheduler,
                          workflow,
                          first_scheduler_change_trigger, periodic_scheduler_change_trigger, speculative_work_fraction,
                          simulation_noise_scheme, simulation_noise, simulation_noise_seed, energy_bound,
                          algorithm_selection_scheme,
                          compute_services, storage_services, file_registry_service, wms_host));

    // Set the amdahl parameter for each task between 0.8 and 1.0
    std::uniform_real_distribution<double> random_dist(0.8, 1.0);
    std::mt19937 rng(42);
    for (auto const &t : workflow->getTasks()) {
        t->setParallelModel(wrench::ParallelModel::AMDAHL(random_dist(rng)));
    }

    // Stage all input files on the WMS Storage Service
    for (const auto &f : workflow->getInputFiles()) {
        simulation->stageFile(f, wms_ss);
    }

    simulation->getOutput().enableFileReadWriteCopyTimestamps(false);
    simulation->getOutput().enableWorkflowTaskTimestamps(false);
    // Launch the simulation
//    std::cerr << "Launching the Simulation..." << std::endl;
    try {
        simulation->launch();
    } catch (std::runtime_error &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 0;
    }

    // Output
    output_json["makespan"] = workflow->getCompletionDate();
    std::string alg_sequence;
    for (auto const &a : wms->getAlgorithmSequence()) {
        alg_sequence += std::to_string(a) + ",";
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

//    std::cerr << workflow->getCompletionDate() << "\n";

    return 0;
}

