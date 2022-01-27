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
    cursor = collection.find()
    for doc in cursor:
        clusters.add(doc["clusters"])
        workflows.add(doc["workflow"])
        if len(doc["algorithms"].split(",")) == 1:
            algorithms.add(doc["algorithms"])

    workflows = sorted(list(workflows))
    clusters = sorted(list(clusters))
    algorithms = sorted(list(algorithms))

    for cluster in clusters:
        for workflow in workflows:
            cursor = collection.find({"clusters":cluster,"workflow":workflow})
            worst_single_alg_makespan = -1
            best_single_alg_makespan = -1
            for doc in cursor:
                if len(doc["algorithms"].split(",")) == 1:
                    if (best_single_alg_makespan < 0) or (best_single_alg_makespan >= doc["makespan"]):
                        best_single_alg_makespan = doc["makespan"]
                    if (worst_single_alg_makespan < 0) or (worst_single_alg_makespan <= doc["makespan"]):
                        worst_single_alg_makespan = doc["makespan"]
            print("["+cluster+"]["+workflow+"] " + str(100.0 * (worst_single_alg_makespan - best_single_alg_makespan) / worst_single_alg_makespan))



#  #  
#w #                                    .: <10%
#o #                                    *: 10% < <20%
#r # .
#k #
#f #
#l #
#  #--------------------------
#          clusters
