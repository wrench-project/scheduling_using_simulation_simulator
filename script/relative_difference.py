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

def generate_plot(x):
    
    fontsize = 14
    
    output_file = "relative_difference.pdf"

    sys.stderr.write("Generating " + output_file + "...\n")
    
    values = range(len(x.keys()))
    y = [i[0] for i in x.values()]

    plt.figure(figsize=(14, 7))
    plt.grid(axis='y')

    plt.plot(values, y, 'b')
    
    #plotting dots to represent algorithm
    i = 0

    dots = [i[1] for i in x.values()]

    for cluster in dots:
        for algo in cluster:
            plt.plot(i, algo, 'o', markersize=1, color='0.5')
        i += 1

    plt.xticks(values, x.keys(), rotation=90, fontsize=fontsize)
    plt.yticks(fontsize=fontsize)
    plt.xlabel("Configurations (Workflow:Platform)",fontsize=fontsize)
    plt.ylabel("% difference",fontsize=fontsize)
   
    plt.tight_layout()

    plt.savefig(output_file)
    plt.close()
    
    sys.stderr.write("Plot saved to file " + output_file + "\n")

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
    algorithms = set()
    cursor = collection.find()
    for doc in cursor:
        if len(doc["algorithms"].split(",")) == 1:
            algorithms.add(doc["algorithms"])
    algorithms = sorted(list(algorithms))

    workflow_id_map = {}
    workflow_id_map['montage-chameleon-2mass-10d-001.json'] = "1"
    workflow_id_map['epigenomics-chameleon-ilmn-4seq-50k-001.json'] = "2"
    workflow_id_map['bwa-chameleon-large-003.json'] = "3"
    workflow_id_map['cycles-chameleon-2l-2c-12p-001.json'] = "4"
    workflow_id_map['1000genome-chameleon-8ch-250k-001.json'] = "5"
    workflow_id_map['blast-chameleon-medium-002.json'] = "6"
    workflow_id_map['soykb-chameleon-10fastq-20ch-001.json'] = "7"
    workflow_id_map['srasearch-chameleon-10a-003.json'] = "8"
    workflows = dict(sorted(workflow_id_map.items(), key=lambda item: item[1])).keys()

    clusters_id_map = {}
    clusters_id_map['96:8:100Gf:100MBps'] = "1"
    clusters_id_map['48:8:150Gf:100MBps,48:8:50Gf:100MBps'] = "2"
    clusters_id_map['48:8:400Gf:10MBps,48:8:50Gf:100MBps'] = "3"
    clusters_id_map['32:8:100Gf:100MBps,32:8:200Gf:200MBps,32:8:300Gf:300MBps'] = "4"
    clusters_id_map['32:8:100Gf:100MBps,32:8:200Gf:300MBps,32:8:300Gf:200MBps'] = "5"
    clusters_id_map['32:8:100Gf:200MBps,32:8:200Gf:100MBps,32:8:300Gf:300MBps'] = "6"
    clusters_id_map['32:8:100Gf:200MBps,32:8:200Gf:300MBps,32:8:300Gf:100MBps'] = "7"
    clusters_id_map['32:8:100Gf:300MBps,32:8:200Gf:200MBps,32:8:300Gf:100MBps'] = "8"
    clusters_id_map['32:8:100Gf:300MBps,32:8:200Gf:100MBps,32:8:300Gf:200MBps'] = "9"
   
    clusters = dict(sorted(clusters_id_map.items(), key=lambda item: item[1])).keys()
    
    percent_diff = {}
    x_labels = []
    for workflow in workflows:
        cluster_mat = []
        for cluster in clusters:
            cursor = collection.find({"clusters":cluster,"workflow":workflow})
            worst_single_alg_makespan = -1
            best_single_alg_makespan = -1
   
            for doc in cursor:
                if len(doc["algorithms"].split(",")) == 1:
                    if (best_single_alg_makespan < 0) or (best_single_alg_makespan >= doc["makespan"]):
                        best_single_alg_makespan = doc["makespan"]
                    if (worst_single_alg_makespan < 0) or (worst_single_alg_makespan <= doc["makespan"]):
                        worst_single_alg_makespan = doc["makespan"]
                
            # Getting relative best-worst difference value for each workflow-cluster configuration
            config_name = "W"+workflow_id_map[workflow]+":P"+clusters_id_map[cluster]
            percent_diff[config_name] = (100.0 * (worst_single_alg_makespan - best_single_alg_makespan) / worst_single_alg_makespan)
            
            # getting the relative difference for each algorithm in each workflow-cluster config
            relative_vals = []
            cursor.rewind()
            for doc in cursor:
              if len(doc["algorithms"].split(",")) == 1:
                  relative_vals.append(100 * (doc["makespan"] - best_single_alg_makespan) / doc["makespan"])
            
            percent_diff[config_name] = (percent_diff[config_name], relative_vals)

    percent_diff = dict(sorted(percent_diff.items(), key=lambda item: item[1][0]))
    generate_plot(percent_diff)

    print("Total configurations: " + str(len(clusters) * len(workflows)))
