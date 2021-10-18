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
import random

global collection

if __name__ == "__main__":

    # Setup Mongo
    try:
        mongo_url = "mongodb://localhost"
        db_name = "scheduling_with_simulation_results"
        mongo_client = MongoClient(host=mongo_url, serverSelectionTimeoutMS=1000)
        mydb = mongo_client["scheduling_with_simulation"]
        collection = mydb["results"]
    except:
        sys.stderr.write("Cannot connect to MONGO\n")
        sys.exit(1)

    workflows = set()
    clusters = set()
    speculative_work_fractions = set()
    algorithms = set()
    cursor = collection.find()
    for doc in cursor:
        clusters.add(doc["clusters"])
        workflows.add(doc["workflow"])
        speculative_work_fractions.add(doc["speculative_work_fraction"])
        if len(doc["algorithms"].split(",")) == 1:
            algorithms.add(doc["algorithms"])

    workflows = sorted(list(workflows))
    clusters = sorted(list(clusters))
    speculative_work_fractions = sorted(list(speculative_work_fractions))
    algorithms = sorted(list(algorithms))

    for clusters_spec in clusters:
        print("=============================================")
        print("CLUSTERS: " + clusters_spec)
        print("=============================================")

        # Single algorithm winners
        print("  Number of individual algorithm wins")
        num_wins = [[x, 0] for x in range(0, len(algorithms))]
        for workflow in workflows:
            cursor = collection.find({"clusters":clusters_spec, "workflow":workflow, "simulation_noise":0.0})
            best_makespan = float('inf')
            best_algorithm = -1
            for doc in cursor:
                if len(doc["algorithms"].split(",")) > 1:
                    continue
                if doc["makespan"] < best_makespan:
                    best_makespan = doc["makespan"]
                    best_algorithm = int(doc["algorithms"])
            num_wins[best_algorithm][1] += 1
        num_wins = reversed(sorted(num_wins, key=lambda x: x[1]))
        for x in num_wins:
            print("    Algorithm " + f'{x[0]:02d}' + ": " + str(x[1]) + " wins")
    
        # Algorithm sequences
        print("  Number of times each algorithm appears in an adaptive sequence (noise = 0, fraction = *)")
        num_occurrences = [[x, 0] for x in range(0, len(algorithms))]
        for workflow in workflows:
            cursor = collection.find({"clusters":clusters_spec, "workflow": workflow, "simulation_noise": 0.0})
            for doc in cursor:
                algs = list(set(doc["algorithm_sequence"].split(",")))
                if len(algs) <= 1:
                    continue
                for alg in algs:
                    num_occurrences[int(alg)][1] += 1
        num_occurrences = reversed(sorted(num_occurrences, key=lambda x: x[1]))
        for x in num_occurrences:
            print("    Algorithm " + f'{x[0]:02d}' + ": used at least once in " + str(x[1]) + " sequences")
    
        # Algorithm sequences
        print("  Number of different algorithms in an adaptive sequence (noise = 0, fraction = *)")
        num_different_algorithms_in_sequence = {}
        for workflow in workflows:
            cursor = collection.find({"clusters":clusters_spec, "workflow": workflow, "simulation_noise": 0.0})
            for doc in cursor:
                tokens = doc["algorithm_sequence"].split(",")
                if len(tokens) == 1:
                    continue
                num_algs = len(list(set(tokens)))
                if not num_algs in num_different_algorithms_in_sequence:
                    num_different_algorithms_in_sequence[num_algs] = 1
                else:
                    num_different_algorithms_in_sequence[num_algs] += 1
                
        num_different_algorithms_in_sequence = reversed(sorted(num_different_algorithms_in_sequence, key=lambda x: x[1]))
        for x in num_occurrences:
            print("    " + x[1] + " " + x[0] +"-algorithm sequences")
    
 
        # NO NOISE RESULTS
        for workflow in workflows:
            print("  WORKFLOW: " + workflow)
            best_single_alg_makespan = -1.0
            cursor = collection.find({"clusters":clusters_spec,"workflow":workflow})
            for doc in cursor:
                if len(doc["algorithms"].split(",")) == 1:
                    if (best_single_alg_makespan < 0) or (best_single_alg_makespan >= doc["makespan"]):
                        best_single_alg_makespan = doc["makespan"]
            if best_single_alg_makespan < 0:
                continue
    
            for speculative_work_fraction in speculative_work_fractions: 
                cursor = collection.find({"clusters":clusters_spec, "workflow":workflow, "simulation_noise":0.0, "speculative_work_fraction":speculative_work_fraction})
                best_adaptive_alg_makespan = -1.0
                for doc in cursor:
                    if len(doc["algorithms"].split(",")) > 1:
                        if (best_adaptive_alg_makespan < 0) or (best_adaptive_alg_makespan >= doc["makespan"]):
                            best_adaptive_alg_makespan = doc["makespan"]
                if best_adaptive_alg_makespan < 0:
                    continue
                            
    #            print("       " + str(best_single_alg_makespan) + " vs. " + str(best_adaptive_alg_makespan))
                improvement = 100.0* (best_single_alg_makespan - best_adaptive_alg_makespan) / best_single_alg_makespan
                print( "    SPEC " + str(speculative_work_fraction) + ": " + str("{:.2f}".format(improvement)) + "%")
    


