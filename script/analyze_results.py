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


    # Getting sets of things
    workflows = set()
    algorithms = set()
    cursor = collection.find({})
    for doc in cursor:
        workflows.add(doc["workflow"])
        if len(doc["algorithms"].split(",")) == 1:
            algorithms.add(doc["algorithms"])

    # Single algorithm winners
    print("Number of individual algorithm wins")
    num_wins = [[x, 0] for x in range(0, len(algorithms))]
    for workflow in workflows:
        cursor = collection.find({"workflow":workflow, "simulation_noise":0.0})
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
        print("\tAlgorithm " + f'{x[0]:02d}' + ": " + str(x[1]) + " wins")

    # Algorithm sequences
    print("Number of times each algorithm appears in an adaptive sequence (noise = 0, fraction = *)")
    num_occurrences = [[x, 0] for x in range(0, len(algorithms))]
    for workflow in workflows:
        cursor = collection.find({"workflow": workflow, "simulation_noise": 0.0})
        for doc in cursor:
            algs = list(set(doc["algorithm_sequence"].split(",")))
            if len(algs) <= 1:
                continue
            for alg in algs:
                num_occurrences[int(alg)][1] += 1
    num_occurrences = reversed(sorted(num_occurrences, key=lambda x: x[1]))
    for x in num_occurrences:
        print("\tAlgorithm " + f'{x[0]:02d}' + ": used at least once in " + str(x[1]) + " sequences")






