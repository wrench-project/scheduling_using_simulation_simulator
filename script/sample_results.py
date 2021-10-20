#!/usr/bin/env python3
import random
import subprocess
import glob
from multiprocessing import Pool
import sys
import json
from pymongo import MongoClient

global collection

def select_workflows_to_use(workflow_dir):

    workflows = []

    min_sequential_duration_in_hours = 5  

    run_time_map = {}
    
    json_files = glob.glob(workflow_dir + "/**/*.json", recursive = True)
    for json_file in json_files:
        # Ignoring nextflow workflows
        if "nextflow" in json_file:
            continue
        f = open(json_file,"r")
        json_object = json.load(f)
        num_tasks=len(json_object["workflow"]["jobs"])
        run_time = 0.0
        for job in json_object["workflow"]["jobs"]:
            run_time += job["runtime"]
        run_time = int(10 * run_time / 3600.0) / 10.0
        map_key = json_file.split("/")[-1].split("-")[0]
        if not map_key in run_time_map:
            run_time_map[map_key] = [[json_file, num_tasks, run_time]]
        else:
            run_time_map[map_key].append([json_file, num_tasks, run_time])
    
    for map_key in run_time_map:
        instances = sorted(run_time_map[map_key], key=lambda x: x[2])
        marked = False
        for i in range(0, len(instances)):
            instance = instances[i]
            if ((not marked) and instance[2] > min_sequential_duration_in_hours) or ((not marked) and i == len(instances) -1):
                marked = True
                sys.stderr.write(str(instance[2]) + " hours \t" + str(instance[1]) + " tasks \t" + instance[0] + "\n")
                workflows.append(instance[0])

    return workflows


def run_simulation(command_to_run):

    # Get the input JSON to check on whetehr we really need to run this
    json_output = subprocess.check_output(command_to_run + " --print_JSON", shell=True)
    config = json.loads(json_output)
    #print(config)

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
    json_output = subprocess.check_output(command_to_run, shell=True)
    result = json.loads(json_output)
    collection.insert_one(result)


if __name__ == "__main__":

    # Argument parsing
    ######################
    if (len(sys.argv) != 3):
        sys.stderr.write("Usage: " + sys.argv[0] + " <num threads> <dir with checked out WfInstances repos>\n")
        sys.exit(1)

    try:
        num_threads = int(sys.argv[1])
        workflow_dir = sys.argv[2]
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
        #mydb = mongo_client["scheduling_with_simulation"]
        #collection = mydb["results"]
        #collection.drop()
        #sys.exit(0)
    except:
        sys.stderr.write("Cannot connect to Mongo... aborting\n")
        sys.exit(1)

    # Build list of commands
    ####################
    simulator = "../build/simulator"

    #platform = "--clusters 16:8:50Gf:20MBps,16:4:100Gf:10MBps,16:6:80Gf:15MBps "
    platform = "--clusters 32:8:100Gf:20MBps "
    platform += "--reference_flops 100Gf "

    scheduler_change_trigger = "--first_scheduler_change_trigger 0.00 "
    periodic_scheduler_change_trigger = "--periodic_scheduler_change_trigger 0.1 "

    num_samples = 10

    num_algorithms = int(
        subprocess.check_output(simulator + " --print_all_algorithms | wc -l", shell=True, encoding='utf-8').strip())
    algorithms = [str(x) for x in range(0, num_algorithms-1)]  # No Random

    workflows = select_workflows_to_use(workflow_dir)

    speculative_work_fractions = [0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0]
    noises = [0.0, 0.1, 0.2, 0.4, 0.8]

    sys.stderr.write("Phase 1\n")
    commands_to_run = []

    # Standard algorithms
    for workflow in workflows:
        for alg in algorithms:
            command = "../build/simulator " + platform + "  --algorithms " + str(alg) + " --workflow " + workflow
            commands_to_run.append(command)

    with Pool(num_threads) as p:
        p.map(run_simulation, commands_to_run)

    sys.stderr.write("Phase 2\n")

    # Speculative algorithms
    for workflow in workflows:
        sys.stderr.write("  " + workflow + "\n")
        commands_to_run = []
        for speculative_work_fraction in speculative_work_fractions:
            speculative_work_fraction = "--speculative_work_fraction " + str(speculative_work_fraction)
            for noise in noises:
                if noise == 0.0:
                    seeds = [1000]
                else:
                    seeds = range(1000, 1000 + num_samples)
                for seed in seeds:
                    command = "../build/simulator " + platform + scheduler_change_trigger + periodic_scheduler_change_trigger + speculative_work_fraction
                    # All algorithms BUT random
                    command += " --workflow " + workflow + " --algorithms 0-"+str(num_algorithms-2)
                    command += " --simulation_noise " + str(noise) + " --noise_seed " + str(seed)

                    commands_to_run.append(command)
        with Pool(num_threads) as p:
            p.map(run_simulation, commands_to_run)

    # Run the commands
#    sys.stderr.write(str(len(commands_to_run)) + " commands to run\n")





