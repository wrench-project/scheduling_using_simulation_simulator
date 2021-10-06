#!/usr/bin/env python3
import subprocess
import glob
from multiprocessing import Pool
import sys
import json
from pymongo import MongoClient

global collection

def run_simulation(command_to_run):
    # Get the input JSON to check on whetehr we really need to run this
    json_output = subprocess.check_output(command_to_run + " --print_JSON", shell=True)
    config = json.loads(json_output)
    print(config)

    # Look up Mongo to see if results aren't already there, in wich care nevermind
    if collection.find_one(config):
        sys.stderr.write("result already there!")
        return

    # Run the simulator
    #json_output = subprocess.check_output(command_to_run, shell=True)
    json_output["makespan"] = 666.42
    collection.insert_one(json_output)


if __name__ == "__main__":

    # Argument parsing
    ######################
    if (len(sys.argv) != 2):
        sys.stderr.write("Usage: " + sys.argv[0] + " <num threads>\n")
        sys.exit(1)

    try:
        num_threads = int(sys.argv[1])
    except:
        sys.stderr.write("Invalid argument\n")
        sys.exit(1)

    #
    # Setup Mongo
    ####################
    mongo_url = "localhost"
    db_name = "scheduling_with_simulation_results"
    try:
        mongo_client = MongoClient(host=mongo_url, serverSelectionTimeoutMS=1000)
        mongo_client.server_info()
        db_names = mongo_client.list_database_names()
        print(db_names)
    except:
        sys.stderr.write("Cannot connect to Mongo... aborting\n")
        sys.exit(1)

    collection = mongo_client.scheduling_with_simulation.results

    # Build list of commands
    ####################
    simulator = "../build/simulator"
    workflow_dir = "../workflows/"

    platform = "--clusters 16:8:50Gf:20MBps,16:4:100Gf:10MBps,16:6:80Gf:15MBps "
    platform += "--reference_flops 100Gf "

    scheduler_change_trigger = "--first_scheduler_change_trigger 0.00 "
    periodic_scheduler_change_trigger = "--periodic_scheduler_change_trigger 0.1 "

    num_samples = 10

    num_algorithms = int(
        subprocess.check_output(simulator + " --print_all_algorithms | wc -l", shell=True, encoding='utf-8').strip())
    algorithms = [str(x) for x in range(0, num_algorithms)]

    workflows = sorted(glob.glob(workflow_dir+"*.json"))

    speculative_work_fractions = [0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0]
    noises = [0.0, 0.1, 0.2, 0.4, 0.8]

    commands_to_run = []

    # Standard algorithms
    for workflow in workflows:
        for alg in algorithms:
            command = "../build/simulator " + platform + "  --algorithms " + str(alg) + " --workflow " + workflow
            commands_to_run.append(command)

    # Speculative algorithms
    for workflow in workflows:
        for speculative_work_fraction in speculative_work_fractions:
            speculative_work_fraction = "--speculative_work_fraction " + str(speculative_work_fraction)
            for noise in noises:
                if noise == 0.0:
                    seeds = [1000]
                else:
                    seeds = range(1000, 1000 + num_samples)
                for seed in seeds:
                    command = "../build/simulator " + platform + scheduler_change_trigger + periodic_scheduler_change_trigger + speculative_work_fraction
                    command += " --workflow " + workflow + " --algorithms 0-"+str(num_algorithms-1)
                    command += " --simulation_noise " + str(noise) + " --noise_seed " + str(seed)

                    commands_to_run.append(command)

    # Run the commands
    sys.stderr.write(str(len(commands_to_run)) + " commands to run\n")

    with Pool(num_threads) as p:
        p.map(run_simulation, commands_to_run)




