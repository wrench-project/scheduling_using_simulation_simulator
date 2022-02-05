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
    noises = set()
    cursor = collection.find()
    for doc in cursor:
        clusters.add(doc["clusters"])
        workflows.add(doc["workflow"])
        noises.add(doc["simulation_noise"])
        if len(doc["algorithms"].split(",")) == 1:
            algorithms.add(doc["algorithms"])

    workflows = sorted(list(workflows))
    clusters = sorted(list(clusters))
    algorithms = sorted(list(algorithms))
    noises = sorted(list(noises))

    speculative_work_fraction = 1.0

    results = {}
    for noise in noises:
        sys.stderr.write("Processing noise " + str(speculative_work_fraction) + "\n")
        results[noise] = {}
        for workflow in workflows:
            sys.stderr.write("  Processing workflow " + str(workflow) + "\n")
            results[noise][workflow] = {}
            for cluster in clusters:
                results[noise][workflow][cluster] = {}
                cursor = collection.find({"clusters":cluster,"workflow":workflow})
                for doc in cursor:
                    if (len(doc["algorithms"].split(",")) != 1) and (doc["simulation_noise"] == noise) and (doc["speculative_work_fraction"] == speculative_work_fraction):
                        results[noise][workflow][cluster]["us"] = doc["makespan"]
                    else:
                        results[noise][workflow][cluster][doc["algorithms"]] = doc["makespan"]

    # Save result dict to a file
    output_file_name = "improvement_vs_noise_extracted_results.dict"
    f = open(output_file_name, "w")
    f.write(str(results) + "\n")
    f.close()
    print("  Result dictionary (used by the plotting script) written to file " + output_file_name)
