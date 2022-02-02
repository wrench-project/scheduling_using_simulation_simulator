#!/usr/bin/env python3
import random
import subprocess
import glob
from multiprocessing import Pool
import sys
import json
from pymongo import MongoClient

global collection

def run_simulation(command_to_run):

    print(command_to_run)
    return

    # Get the input JSON to check on whether we really need to run this
    json_output = subprocess.check_output(command_to_run + " --print_JSON", shell=True)
    config = json.loads(json_output)

    # Setup Mongo
    mongo_url = "mongodb://localhost"
    db_name = "scheduling_with_simulation_results"
    mongo_client = MongoClient(host=mongo_url, serverSelectionTimeoutMS=1000)
    mydb = mongo_client["scheduling_with_simulation"]
    collection = mydb["results"]

    # Look up Mongo to see if results aren't already there, in wich case nevermind
    if collection.find_one(config):
        sys.stderr.write(".")
        sys.stderr.flush()
        return

    # Run the simulator
    print(command_to_run)
    try:
        json_output = subprocess.check_output(command_to_run, shell=True)
    except CalledProcessError:
        return
    result = json.loads(json_output)
    collection.insert_one(result)


if __name__ == "__main__":

    platform_configurations = [
            #"--clusters 96:8:100Gf:100MBps",
            #"--clusters 48:8:50Gf:100MBps,48:8:150Gf:100MBps",
            #"--clusters 48:8:50Gf:100MBps,48:8:400Gf:10MBps",
            "--clusters 32:8:100Gf:100MBps,32:8:200Gf:200MBps,32:8:300Gf:300MBps",
            #"--clusters 32:8:100Gf:100MBps,32:8:200Gf:300MBps,32:8:300Gf:200MBps",
            #"--clusters 32:8:100Gf:200MBps,32:8:200Gf:100MBps,32:8:300Gf:300MBps",
            #"--clusters 32:8:100Gf:200MBps,32:8:200Gf:300MBps,32:8:300Gf:100MBps",
            #"--clusters 32:8:100Gf:300MBps,32:8:200Gf:200MBps,32:8:300Gf:100MBps",
            #"--clusters 32:8:100Gf:300MBps,32:8:200Gf:100MBps,32:8:300Gf:200MBps"
               ]

    workflow_dir = "../../wfinstances/"
    workflow_json_files = glob.glob(workflow_dir + "/**/*.json", recursive = True)


    # Argument parsing
    ######################
    if (len(sys.argv) != 3):
        sys.stderr.write("Usage: " + sys.argv[0] + " <num threads> <workflow list>\n\n")
        sys.stderr.write("Example: " + sys.argv[0] + " 8 0,1,2\n\n")
        sys.stderr.write("Workflows :\n")
        for i in range(0, len(workflow_json_files)):
            sys.stderr.write("\t"+str(i)+": " + workflow_json_files[i] + "\n")
        sys.stderr.write("\n")
        sys.exit(1)

    try:
        num_threads = int(sys.argv[1])
        workflows = [int(x) for x in sys.argv[2].split(",")]
    except:
        sys.stderr.write("Invalid argument\n")
        sys.exit(1)

    #
    # Setup Mongo
    ####################
    try:
        mongo_url = "mongodb://localhost"
        mongo_client = MongoClient(host=mongo_url, serverSelectionTimeoutMS=1000)
        mongo_client.server_info()
    except:
        sys.stderr.write("Cannot connect to Mongo... aborting\n")
        sys.exit(1)

    # Build list of commands
    ####################
    simulator = "../../build/simulator "


    reference_flop = " --reference_flops 100Gf --wrench-energy-simulation "
    scheduler_change_trigger = "--first_scheduler_change_trigger 0.00 "
    speculative_work_fraction = "--speculative_work_fraction 1.0 "

    num_samples = 10

    num_algorithms = int(
        subprocess.check_output(simulator + " --print_all_algorithms | wc -l", shell=True, encoding='utf-8').strip())
    algorithms = [str(x) for x in range(0, num_algorithms-1)]  # No Random

    workflow_json_files = glob.glob(workflow_dir + "/**/*.json", recursive = True)

    periodic_scheduler_change_triggers = [0.05, 0.1, 0.15, 0.2, 0.3, 0.4, 0.5]
    speculative_work_fractions = [1.0]
    noises = [0.0, 0.1, 0.2, 0.4, 0.8]


    sys.stderr.write("Phase 1\n")
    commands_to_run = []


    platform = platform_configurations[0]
    for periodic_scheduler_change_trigger in periodic_scheduler_change_triggers:
        periodic_scheduler_change_trigger = "--periodic_scheduler_change_trigger " + str(periodic_scheduler_change_trigger) + " " 

        for workflow in workflow_json_files:
            sys.stderr.write("  " + workflow + "\n")
            commands_to_run = []
            for speculative_work_fraction in speculative_work_fractions:
                speculative_work_fraction = "--speculative_work_fraction " + str(speculative_work_fraction) + " "
                for noise in noises:
                    if noise == 0.0:
                        seeds = [1000]
                    else:
                        seeds = range(1000, 1000 + num_samples)
                    for seed in seeds:
                        command = simulator + platform + reference_flop + scheduler_change_trigger + periodic_scheduler_change_trigger + speculative_work_fraction
                        # All algorithms BUT random
                        command += " --workflow " + workflow + " --algorithms 0-"+str(num_algorithms-2)
                        command += " --simulation_noise " + str(noise) + " --noise_seed " + str(seed)

                        commands_to_run.append(command)
            with Pool(num_threads) as p:
                p.map(run_simulation, commands_to_run)






