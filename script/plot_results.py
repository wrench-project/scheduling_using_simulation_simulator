#!/usr/bin/env python3
import random
import subprocess
import glob
from multiprocessing import Pool
import sys
import json
from pymongo import MongoClient
import matplotlib.pyplot as plt
from matplotlib.pyplot import figure

global collection

def generate_plot(workflow_name, output_file):

    single_algs = []
    adaptive_alg = {}

    sys.stderr.write("Generating " + output_file + "...\n")

    # Build data structure
    noises = set()
    cursor = collection.find({"workflow":workflow_name})
    for doc in cursor:
        # print(doc)
        noises.add(doc["simulation_noise"])
        if len(doc["algorithms"].split(",")) == 1:
            single_algs.append(doc["makespan"])
        else:
            noise = doc["simulation_noise"]
            if noise not in adaptive_alg:
                adaptive_alg[noise] = {}
            fraction = doc["speculative_work_fraction"]
            if fraction not in adaptive_alg[noise]:
                adaptive_alg[noise][fraction] = []
            adaptive_alg[noise][fraction].append(doc["makespan"])

    figure(figsize=(len(noises) * 2.4, 6), dpi=80)

    noises = sorted(noises)

    # Plot stuff
    plot_count = 1
    for noise in noises:
        ax = plt.subplot(1,len(noises),plot_count)
        ax.set_title("noise = " + str(noise))
        if plot_count == 1:
            ax.set_ylabel("makespan (sec)")
        else:
            ax.axes.yaxis.set_visible(False)
            pass
        ax.set_xlabel("speculative work fraction")
        plot_count += 1
        ax.plot([0] * len(single_algs), single_algs, "go")
        count = 1
        if noise in adaptive_alg:
            fractions = sorted([x for x in adaptive_alg[noise]])
        else:
            fractions = []
        for fraction in fractions:
            makespans = adaptive_alg[noise][fraction]
            ax.plot([count] * len(makespans), makespans, "ro")
            count += 1
        ax.set_xticks(list(range(0, count)))
        if noise in adaptive_alg:
            ax.set_xticklabels([""] + [str(x) for x in sorted(adaptive_alg[noise])], fontsize=6)
        else:
            ax.set_xticklabels([""], fontsize=6)
                

    plt.savefig(output_file)
    plt.close()
    return


if __name__ == "__main__":

    # Setup Mongo
    try:
        mongo_url = "mongodb://localhost"
        db_name = "scheduling_with_simulation_results"
        mongo_client = MongoClient(host=mongo_url, serverSelectionTimeoutMS=1000)
        mydb = mongo_client["scheduling_with_simulation"]
        collection = mydb["results"]
    except:
        sys.write("Cannot connect to MONGO\n")
        sys.exit(1)

    workflows = set()
    cursor = collection.find({})
    for doc in cursor:
        workflows.add(doc["workflow"])

    # Getting all workflows

    for workflow in workflows:
        workflow_name = workflow.split("/")[-1]
        generate_plot(workflow_name, workflow_name+".result.pdf")



#     # Look up Mongo to see if results aren't already there, in which case nevermind
#     if collection.find_one(config):
#         sys.stderr.write("ALREADY RAN!\n")
#         return
#     else:
#         sys.stderr.write("RUNNING: " + str(config) + "\n")
#
#     # Run the simulator
#     # json_output = subprocess.check_output(command_to_run, shell=True)
#     # result = json.loads(json_output)
#     result = config
#     result["makespan"] = 1000.0 + random.random() * 10000.0
#     sys.stderr.write("ADDING: " + str(result))
#     collection.insert_one(result)
#
#
# if __name__ == "__main__":
#
#     # Argument parsing
#     ######################
#     if (len(sys.argv) != 2):
#         sys.stderr.write("Usage: " + sys.argv[0] + " <num threads>\n")
#         sys.exit(1)
#
#     try:
#         num_threads = int(sys.argv[1])
#     except:
#         sys.stderr.write("Invalid argument\n")
#         sys.exit(1)
#
#     #
#     # Setup Mongo
#     ####################
#     try:
#         mongo_url = "mongodb://localhost"
#         mongo_client = MongoClient(host=mongo_url, serverSelectionTimeoutMS=1000)
#         mongo_client.server_info()
#         #mydb = mongo_client["scheduling_with_simulation"]
#         #collection = mydb["results"]
#         #collection.drop()
#         #sys.exit(0)
#     except:
#         sys.stderr.write("Cannot connect to Mongo... aborting\n")
#         sys.exit(1)
#
#         # Build list of commands
#     ####################
#     simulator = "../build/simulator"
#     workflow_dir = "../workflows/"
#
#     platform = "--clusters 16:8:50Gf:20MBps,16:4:100Gf:10MBps,16:6:80Gf:15MBps "
#     platform += "--reference_flops 100Gf "
#
#     scheduler_change_trigger = "--first_scheduler_change_trigger 0.00 "
#     periodic_scheduler_change_trigger = "--periodic_scheduler_change_trigger 0.1 "
#
#     num_samples = 10
#
#     num_algorithms = int(
#         subprocess.check_output(simulator + " --print_all_algorithms | wc -l", shell=True, encoding='utf-8').strip())
#     algorithms = [str(x) for x in range(0, num_algorithms)]
#
#     workflows = sorted(glob.glob(workflow_dir+"*.json"))
#
#     speculative_work_fractions = [0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0]
#     noises = [0.0, 0.1, 0.2, 0.4, 0.8]
#
#     commands_to_run = []
#
#     # Standard algorithms
#     for workflow in workflows:
#         for alg in algorithms:
#             command = "../build/simulator " + platform + "  --algorithms " + str(alg) + " --workflow " + workflow
#             commands_to_run.append(command)
#
#     # Speculative algorithms
#     for workflow in workflows:
#         for speculative_work_fraction in speculative_work_fractions:
#             speculative_work_fraction = "--speculative_work_fraction " + str(speculative_work_fraction)
#             for noise in noises:
#                 if noise == 0.0:
#                     seeds = [1000]
#                 else:
#                     seeds = range(1000, 1000 + num_samples)
#                 for seed in seeds:
#                     command = "../build/simulator " + platform + scheduler_change_trigger + periodic_scheduler_change_trigger + speculative_work_fraction
#                     command += " --workflow " + workflow + " --algorithms 0-"+str(num_algorithms-1)
#                     command += " --simulation_noise " + str(noise) + " --noise_seed " + str(seed)
#
#                     commands_to_run.append(command)
#
#     # Run the commands
#     sys.stderr.write(str(len(commands_to_run)) + " commands to run\n")
#
#     with Pool(num_threads) as p:
#         p.map(run_simulation, commands_to_run)
#
#
#
#
