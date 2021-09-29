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

namespace po = boost::program_options;

int main(int argc, char **argv) {

    // Declaration of the top-level WRENCH simulation object
    wrench::Simulation simulation;

    // Initialization of the simulation
    simulation.init(&argc, argv);

    // Generic lambda to check if a numeric argument is in some range
    auto in = [](const auto &min, const auto &max, char const * const opt_name) {
        return [opt_name, min, max](const auto &v){
            if (v < min || v > max) {
                throw po::validation_error
                        (po::validation_error::invalid_option_value,
                         opt_name, std::to_string(v));
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

    std::string reference_flops;
    std::string algorithm_list;
    double first_scheduler_change_trigger;
    double periodic_scheduler_change_trigger;
    double speculative_work_fraction;


    // Define command-line argument options
    po::options_description desc("Allowed options");
    desc.add_options()
            ("help",
             "Show this help message\n")
            ("workflow", po::value<std::string>(&workflow_file)->required()->value_name("<path>"),
             "Path to JSON workflow description file\n")
            ("reference_flops", po::value<std::string>(&reference_flops)->required()->value_name("<ref flops>"),
             "Reference flop rate for the workflow file tasks (e.g., \"100Gf\")\n")
            ("cluster", po::value<std::vector<std::string>>()->required()->value_name("name:#nodes:#cores:flops:bw"),
             "Cluster specification. Example: \"cluster:100:8:200Gf:100MBps\"\n")
            ("print_all_algorithms",
             "Print all scheduling algorithms available\n")
            ("algorithms", po::value<std::string>(&algorithm_list)->required()->value_name("<list of algorithm #>"),
             "First one in the list will be used initially\nExample: --algorithms 0-4,12,15-17,19,21\n"
             "(use --print_all_scheduling_algorithms to see the list of algorithms)\n")
            ("first_scheduler_change_trigger", po::value<double>(&first_scheduler_change_trigger)->value_name("<work fraction>")->default_value(1.0),
             ("The algorithm may change for the first time once this fraction of the work has been performed "
              "(between 0.0 and 1, 0.0 meaning \"right away\" and 1.0 meaning \"never change\")\n"))
            ("periodic_scheduler_change_trigger", po::value<double>(&periodic_scheduler_change_trigger)->value_name("<work fraction>")->default_value(1.0),
             ("The algorithm may change each time this fraction of the work has been performed (between 0.0 and 1, 1 meaning \"never change\")\n"))
            ("speculative_work_fraction", po::value<double>(&speculative_work_fraction)->value_name("<work fraction>")->default_value(1.0),
             ("The fraction of work that a speculative execution performs before reporting to the master process (between 0.0 and 1, 1 meaning \"until workflow completion\")"))
            ;

    // Parse command-line arguments
    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
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

    // Creation of the platform
    std::string wms_host = "wms_host";
    try {
        PlatformCreator platform_creator(wms_host, vm["cluster"].as<std::vector<std::string>>());
        simulation.instantiatePlatform(platform_creator);
    } catch (std::exception &e) {
        std::cerr << e.what() << "\n";
        exit(1);
    }

    // Add all specified scheduling algorithms in oder to the scheduler
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

    // Start a BareMetal Service on each cluster, and a Storage Service on the head node
    std::set<std::shared_ptr<wrench::ComputeService>> compute_services;
    std::set<std::shared_ptr<wrench::StorageService>> storage_services;
    for (const auto &spec : vm["cluster"].as<std::vector<std::string>>()) {
        auto parsed_spec = PlatformCreator::parseClusterSpecification(spec);
        std::string name = std::get<0>(parsed_spec);
        int num_hosts = std::get<1>(parsed_spec);
        int num_cores = std::get<2>(parsed_spec);
        std::string flops = std::get<3>(parsed_spec);

        std::string head_node = name+"-head";
        std::vector<std::string> compute_nodes;
        compute_nodes.reserve(num_hosts);
        for (int i=0; i < num_hosts; i++) {
            compute_nodes.push_back(name+"-node-"+std::to_string(i));
        }

        compute_services.insert(simulation.add(
                new wrench::BareMetalComputeService(
                        head_node,
                        compute_nodes,
                        "/scratch",
                        {},
                        {})));

        storage_services.insert(simulation.add(
                new wrench::SimpleStorageService(
                        head_node,
                        {"/"},
                        {},
                        {})));
    }

    // Create a Storage Service on the WMS host
    auto wms_ss = simulation.add(new wrench::SimpleStorageService(wms_host, {"/"}, {}, {}));
    storage_services.insert(wms_ss);

    // Create a file registry service
    auto file_registry_service = simulation.add(new wrench::FileRegistryService(wms_host));

    // Create the WMS
    auto wms = simulation.add(
            new SimpleWMS(scheduler,
                          first_scheduler_change_trigger, periodic_scheduler_change_trigger, speculative_work_fraction,
                          compute_services, storage_services, file_registry_service, wms_host));


    // Parse the workflow
    auto workflow = wrench::PegasusWorkflowParser::createWorkflowFromJSON(
            workflow_file, reference_flops, false, 1, 32, true);

    // Set the amdahl parameter for each task between 0.8 and 1.0
    std::uniform_real_distribution<double> random_dist(0.8, 1.0);
    std::mt19937 rng(42);
    for (auto const &t : workflow->getTasks()) {
        t->setParallelModel(wrench::ParallelModel::AMDAHL(random_dist(rng)));
    }

    // Add it to the WMS
    wms->addWorkflow(workflow);


    // Stage all input files on the WMS Storage Service
    for (const auto &f : workflow->getInputFiles()) {
        simulation.stageFile(f, wms_ss);
    }

    // Launch the simulation
//    std::cerr << "Launching the Simulation..." << std::endl;
    try {
        simulation.launch();
    } catch (std::runtime_error &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 0;
    }


    std::cout << workflow->getCompletionDate() << "\n";

    return 0;
}

