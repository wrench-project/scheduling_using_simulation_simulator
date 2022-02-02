#!/usr/bin/env python3
import random
import subprocess
from os.path import exists
import glob
from multiprocessing import Pool
import sys
import json
from pymongo import MongoClient
import matplotlib.pyplot as plt
from matplotlib.pyplot import figure
import random

global collection

if __name__ == "__main__":

    # Setup Mongo
    try:
        mongo_url = "mongodb://localhost"
        mongo_client = MongoClient(host=mongo_url, serverSelectionTimeoutMS=1000)
        mydb = mongo_client["scheduling_with_simulation"]
        collection = mydb["results"]
    except:
        sys.stderr.write("Cannot connect to MONGO\n")
        sys.exit(1)

    # Get values for fieds
    workflows = set()
    clusters = set()
    algorithms = set()
    speculative_work_fractions = set()
    noises = set()
    cursor = collection.find()
    for doc in cursor:
        clusters.add(doc["clusters"])
        workflows.add(doc["workflow"])
        speculative_work_fractions.add(doc["speculative_work_fraction"])
        noises.add(doc["simulation_noise"])
        if len(doc["algorithms"].split(",")) == 1:
            algorithms.add(doc["algorithms"])

    workflows = sorted(list(workflows))
    clusters = sorted(list(clusters))
    algorithms = sorted(list(algorithms))
    speculative_work_fractions = sorted(list(speculative_work_fractions))
    noises = sorted(list(noises))

    speculative_work_fraction = 1.0


    ### COMPUTING RESULTS
    results = {}
    for simulation_noise in noises:
        sys.stderr.write("Processing simulation_noise " + str(simulation_noise) + "\n")
        results[simulation_noise] = {}
        for workflow in workflows:
            sys.stderr.write("  Processing workflow " + str(workflow) + " ")
            sys.stderr.flush()
            results[simulation_noise][workflow] = []
            for baseline_algo in algorithms:
                sys.stderr.write(".")
                sys.stderr.flush()
                for cluster in clusters:
                    cursor = collection.find({"clusters":cluster,"workflow":workflow})
                    for doc in cursor:
                        if (len(doc["algorithms"].split(",")) != 1) and (doc["speculative_work_fraction"] == speculative_work_fraction) and (doc["simulation_noise"] == simulation_noise):
                            our_makespan = doc["makespan"]
                        elif doc["algorithms"] == baseline_algo:
                            baseline_makespan = doc["makespan"]
                    relative_improvement = 100.0 * (baseline_makespan - our_makespan) / baseline_makespan
                    results[simulation_noise][workflow].append(relative_improvement)
            sys.stderr.write("\n")


    # Save result dict to a file
    output_file_name = "improvement_vs_noise_extracted_results.dict"
    f = open(output_file_name, "w")
    f.write(str(results) + "\n")
    f.close()
    print("  Result dictionary (used by the plotting script) written to file " + output_file_name)

