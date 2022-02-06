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

    results = {}
    for workflow in workflows:
        sys.stderr.write("Processing workflow " + str(workflow) + "\n")
        results[workflow] = {}
        for cluster in clusters:
            results[workflow][cluster] = {}
            for baseline_algo in algorithms:
                cursor = collection.find({"clusters":cluster,"workflow":workflow})
                for doc in cursor:
                    if (len(doc["algorithms"].split(",")) != 1) and (doc["speculative_work_fraction"] == 1.0) and (doc["simulation_noise"] == noise):
                        our_makespan = doc["makespan"]
                        algos_used = list(set(doc["algorithm_sequence"].split(",")))
                        results[workflow][cluster]["us"] = [our_makespan, algos_used]
                    elif doc["algorithms"] == baseline_algo:
                        baseline_makespan = doc["makespan"]
                        results[workflow][cluster][baseline_algo] = baseline_makespan

    # Save result dicts to a file
    print("FULL RESULTS:")
    dict_output_file_name = "improvement_ideal_extracted_results.dict"
    f = open(dict_output_file_name, "w")
    f.write(str(results) + "\n")
    f.close() 
    print("  Result dictionary (used by the plotting script) written to file " + dict_output_file_name)



