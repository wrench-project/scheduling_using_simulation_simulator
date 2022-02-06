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

def generate_plot(mat, x_labels, y_labels):


    fig, ax = plt.subplots()
		
    ax.set_xticks(np.arange(len(x_labels)))
    ax.set_xlabel(x_labels)
    ax.set_yticks(np.arange(len(y_labels)))
    ax.set_ylabel(y_labels)
    
    ax.imshow(mat, cmap='hot', interpolation="nearest")

    for i in range(len(y_labels)):
        for j in range(len(x_labels)):
            text = ax.text(j, i, f"{round(mat[i, j], 2)}%", ha="center", va="center", color="r")

    plt.savefig("one_algo_diversity.pdf")
    plt.show()

    #TODO: fix spacing issue
    #TODO: add color bar
		#TODO: change labels
		# 
    plt.close()

    return


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
    degradation_mat = []

    for cluster in clusters:
        cluster_mat = []
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
            
            value = 100.0 * (worst_single_alg_makespan - best_single_alg_makespan) / worst_single_alg_makespan
            cluster_mat.append(value)
            print("["+cluster+"]["+workflow+"] " + str(value))
        
        degradation_mat.append(cluster_mat)

    generate_plot(np.array(degradation_mat).T, clusters, workflows)
    print(np.array(degradation_mat).T)
    print(np.array(degradation_mat))

#  #  
#w #                                    .: <10%
#o #                                    *: 10% < <20%
#r # .
#k #
#f #
#l #
#  #--------------------------
#          clusters
