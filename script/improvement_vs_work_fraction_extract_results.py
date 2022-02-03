#!/usr/bin/env python3
import random
from os.path import exists
import glob
import sys
import json
from pymongo import MongoClient

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
    cursor = collection.find()
    for doc in cursor:
        clusters.add(doc["clusters"])
        workflows.add(doc["workflow"])
        speculative_work_fractions.add(doc["speculative_work_fraction"])
        if len(doc["algorithms"].split(",")) == 1:
            algorithms.add(doc["algorithms"])

    workflows = sorted(list(workflows))
    clusters = sorted(list(clusters))
    algorithms = sorted(list(algorithms))
    speculative_work_fractions = sorted(list(speculative_work_fractions))

    simulation_noise = 0.0

    ### COMPUTING RESULTS BY WORKLOW
    results_by_workflow = {}
    for speculative_work_fraction in speculative_work_fractions:
        sys.stderr.write("Processing work fraction " + str(speculative_work_fraction) + "\n")
        results_by_workflow[speculative_work_fraction] = {}
        for workflow in workflows:
            sys.stderr.write("  Processing workflow " + str(workflow) + " ")
            sys.stderr.flush()
            results_by_workflow[speculative_work_fraction][workflow] = []
            worst_relative_improvement = 0;
            num_losses={}
            for baseline_algo in algorithms:
                num_losses[baseline_algo] = 0
                sys.stderr.write(".")
                sys.stderr.flush()
                for cluster in clusters:
                    cursor = collection.find({"clusters":cluster,"workflow":workflow})
                    for doc in cursor:
                        if (len(doc["algorithms"].split(",")) != 1) and (doc["simulation_noise"] == simulation_noise) and (doc["speculative_work_fraction"] == speculative_work_fraction):
                            our_makespan = doc["makespan"]
                        elif doc["algorithms"] == baseline_algo:
                            baseline_makespan = doc["makespan"]
                    relative_improvement = 100.0 * (baseline_makespan - our_makespan) / baseline_makespan
                    if relative_improvement < 0:
                        num_losses[baseline_algo] += 1
                    worst_relative_improvement = min(worst_relative_improvement, relative_improvement)
                    results_by_workflow[speculative_work_fraction][workflow].append(relative_improvement)
            #print("WORST: " + str(worst_relative_improvement))
            total_losses = sum([num_losses[x] for x in num_losses])
            total_wins = 81  - total_losses
            sys.stderr.write("TOTAL LOSS/WIN = " + str(total_losses) + "/" + str(total_wins))
            sys.stderr.write("MAX LOSS TO ONE ALG = " + str(max(total_losses.values())))
            sys.stderr.write("\n")

    # Save result dict to a file
    output_file_name = "improvement_vs_work_fraction_extracted_results_by_workflow.dict"
    f = open(output_file_name, "w")
    f.write(str(results_by_workflow) + "\n")
    f.close()
    print("  Result dictionary (used by the plotting script) written to file " + output_file_name)

    ### COMPUTING RESULTS BY CLUSTER
    results_by_cluster = {}
    for speculative_work_fraction in speculative_work_fractions:
        sys.stderr.write("Processing work fraction " + str(speculative_work_fraction) + "\n")
        results_by_cluster[speculative_work_fraction] = {}
        for cluster in clusters:
            sys.stderr.write("  Processing cluster " + str(cluster) + " ")
            sys.stderr.flush()
            results_by_cluster[speculative_work_fraction][cluster] = []
            worst_relative_improvement = 0;
            for baseline_algo in algorithms:
                sys.stderr.write(".")
                sys.stderr.flush()
                for workflow in workflows:
                    cursor = collection.find({"clusters":cluster,"workflow":workflow})
                    for doc in cursor:
                        if (len(doc["algorithms"].split(",")) != 1) and (doc["simulation_noise"] == simulation_noise) and (doc["speculative_work_fraction"] == speculative_work_fraction):
                            our_makespan = doc["makespan"]
                        elif doc["algorithms"] == baseline_algo:
                            baseline_makespan = doc["makespan"]
                    relative_improvement = 100.0 * (baseline_makespan - our_makespan) / baseline_makespan
                    worst_relative_improvement = min(worst_relative_improvement, relative_improvement)
                    results_by_cluster[speculative_work_fraction][cluster].append(relative_improvement)
            #print("WORST: " + str(worst_relative_improvement))
            sys.stderr.write("\n")

    # Save result dict to a file
    output_file_name = "improvement_vs_work_fraction_extracted_results_by_cluster.dict"
    f = open(output_file_name, "w")
    f.write(str(results_by_cluster) + "\n")
    f.close()
    print("  Result dictionary (used by the plotting script) written to file " + output_file_name)
