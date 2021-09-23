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

namespace po = boost::program_options;

int main(int argc, char **argv) {

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

    // Define command-line argument options
    po::options_description desc("Allowed options");
    desc.add_options()
            ("help",
             "Show this help message")
            ("workflow", po::value<std::string>(&workflow_file)->required()->value_name("<path>"),
             "Path to JSON workflow description file")
            ("cluster", po::value<std::vector<std::string>>()->required()->value_name("name:#nodes:#cores:flops:bw"),
             "Cluster specification. Example: \"cluster:100:8:200Gf:100MBps\".")
            ;

    // Parse command-line arguments
    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        // Print help message and exit if needed
        if (vm.count("help")) {
            cout << desc << "\n";
            exit(0);
        }
        // Throw whatever exception in case argument values are erroneous
        po::notify(vm);
    } catch (std::exception &e) {
        cerr << "Error: " << e.what() << "\n";
        exit(1);
    }

    // Declaration of the top-level WRENCH simulation object
    wrench::Simulation simulation;

    // Initialization of the simulation
    simulation.init(&argc, argv);

    // Creation of the platform
    std::string wms_host = "wms_host";
    try {
        PlatformCreator platform_creator(wms_host, vm["cluster"].as<std::vector<std::string>>());
        simulation.instantiatePlatform(platform_creator);
    } catch (std::exception &e) {
        std::cerr << e.what() << "\n";
        exit(1);
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

    // Create the WMS
    auto wms = simulation.add(
            new SimpleWMS(compute_services, storage_services, wms_host));


    // Parse the workflow
    auto workflow = wrench::PegasusWorkflowParser::createWorkflowFromJSON(
            workflow_file, "1000Gf");

    // Add it to the WMS
    wms->addWorkflow(workflow);

    // Create a file registry service
    simulation.add(new wrench::FileRegistryService(wms_host));

    // Stage all input files on the WMS Storage Service
    for (const auto &f : workflow->getInputFiles()) {
        simulation.stageFile(f, wms_ss);
    }

    // Launch the simulation
    std::cerr << "Launching the Simulation..." << std::endl;
    try {
        simulation.launch();
    } catch (std::runtime_error &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 0;
    }
    std::cerr << "Simulation done!" << std::endl;

    return 0;
}

