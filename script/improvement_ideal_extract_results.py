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
    noise = 0.0

    algorithms_used = set()

    results = {}
    for workflow in workflows:
        sys.stderr.write("\nProcessing workflow " + str(workflow) + " ")
        results[workflow] = {}
        for cluster in clusters:
            sys.stderr.flush()
            worst_improvement = 0
            baselines = []
            for baseline_algo in algorithms:
                sys.stderr.write(".")
                sys.stderr.flush()
                cursor = collection.find({"clusters":cluster,"workflow":workflow})
                for doc in cursor:
                    if (len(doc["algorithms"].split(",")) != 1) and (doc["speculative_work_fraction"] == 1.0) and (doc["simulation_noise"] == noise):
                        our_makespan = doc["makespan"]
                        algos_used = list(set(doc["algorithm_sequence"].split(",")))
                        num_different_algos = len(algos_used)
                        for a in algos_used:
                            algorithms_used.add(a)
                    elif doc["algorithms"] == baseline_algo:
                        baseline_makespan = doc["makespan"]
                baselines.append(baseline_makespan)
            best_baseline = min(baselines)
            relative_improvement = 100.0 * (best_baseline - our_makespan) / baseline_makespan
            results[workflow][cluster]  = [relative_improvement, num_different_algos]



    statistics_output_file_name = "improvement_ideal_extracted_statistics.txt"
    f = open(statistics_output_file_name, "w")
    f.write("ALGORITHM USAGE STATISTICS:\n");
    num_one_algo_was_used = 0
    total_average = 0
    for workflow in workflows:
        max_num_algorithms = 0
        ave_num_algorithms = 0
        for cluster in clusters:
            if results[workflow][cluster][1] == 1:
                print("ONE:  " + workflow + " " + cluster)
                num_one_algo_was_used += 1
            max_num_algorithms = max(max_num_algorithms, results[workflow][cluster][1])
            ave_num_algorithms += results[workflow][cluster][1]
            total_average += results[workflow][cluster][1]
        ave_num_algorithms /= len(clusters)
        f.write("  # of algs for " + workflow + ": max=" + str(max_num_algorithms) + "; ave=" + str(ave_num_algorithms) + "\n")
                 

    f.write("\n  Number of cases in which our approach used one algorithm: " + str(num_one_algo_was_used) + " / " + str(len(workflows) * len(clusters)) + "\n")
    f.write("\n  Number of different algorithms used on average: " + str(total_average / (len(workflows) * len(clusters))) + "\n")
    f.write("\n  Number of algorithms used at least once: " + str(len(algorithms_used)) + "\n")
    f.close()
    print("  Statistics writte to file " + statistics_output_file_name)


    # Save result dict to a file
    print("FULL RESULTS:")
    dict_output_file_name = "improvement_ideal_extracted_results.dict"
    f = open(dict_output_file_name, "w")
    f.write(str(results) + "\n")
    f.close() 
    print("  Result dictionary (used by the plotting script) written to file " + dict_output_file_name)

