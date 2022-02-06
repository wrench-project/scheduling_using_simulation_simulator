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
import numpy as np

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
    algorithms = sorted(list(algorithms), key=int)
    num_scenarios = len(workflows) * len(clusters)

    dfb = {i: 0.0 for i in algorithms}
    worst_dfb = {i: 0.0 for i in algorithms}
    
    for workflow in workflows:
        for cluster in clusters:
            cursor = collection.find({"clusters":cluster,"workflow":workflow})
            makespans = {doc["algorithms"]: doc["makespan"] for doc in cursor if  len(doc["algorithms"].split(",")) == 1}
            
            best = min(makespans.values())
            for algo, makespan in makespans.items():
                dfb_value = 100 * (makespan - best) / best
                dfb[algo] += dfb_value
                if worst_dfb[algo] < dfb_value:
                    worst_dfb[algo] = dfb_value


    dfb = {algo: (value / num_scenarios) for algo, value in dfb.items()}

    dfb = dict(sorted(dfb.items(),key= lambda x:x[1]))

    for algo, avg_dfb in dfb.items():
        print("["+algo+"] " + str(round(avg_dfb, 2)) + "%  (worst dfb: " + str(round(worst_dfb[algo],2)) + "%)")


#  #  
#w #                                    .: <10%
#o #                                    *: 10% < <20%
#r # .
#k #
#f #
#l #
#  #--------------------------
#          clusters
