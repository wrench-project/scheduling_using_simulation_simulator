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


def generate_plot(workflow_name, output_file):

    single_algs = []
    adaptive_alg = {}

    sys.stderr.write("Generating " + output_file + "...\n")

    # Build data structure
    noises = set()
    cursor = collection.find({"workflow":workflow_name})
    for doc in cursor:
        # print(doc)
        noises.add(doc["simulation_noise"])
        if len(doc["algorithms"].split(",")) == 1:
            single_algs.append([doc["algorithms"], doc["makespan"]])
        else:
            noise = doc["simulation_noise"]
            if noise not in adaptive_alg:
                adaptive_alg[noise] = {}
            fraction = doc["speculative_work_fraction"]
            if fraction not in adaptive_alg[noise]:
                adaptive_alg[noise][fraction] = []
            adaptive_alg[noise][fraction].append(doc["makespan"])

    figure(figsize=(len(noises) * 2.4, 6), dpi=80)

    noises = sorted(noises)

    # Plot stuff
    random.seed(42)
    colormap = []
    for i in range(0,10):
        color = [0,0,0]
        while sum(color) <= 0:
            color = [10, random.choice(range(256)), random.choice(range(256))]
        colorstring = "#"
        for c in color:
            code = hex(c)[2:len(hex(c))]
            if len(code) == 1:
                code = "0" + code
            colorstring += code
        colormap.append(colorstring)
    markermap = ["o", "v", "^", "<", ">"]

    plot_count = 1
    for noise in noises:
        ax = plt.subplot(1,len(noises),plot_count)
        ax.set_title("noise = " + str(noise))
        if plot_count == 1:
            ax.set_ylabel("makespan (sec)")
        else:
            ax.axes.yaxis.set_visible(False)
            pass
        ax.set_xlabel("speculative work fraction")
        plot_count += 1
        for [alg, ms] in single_algs:

            ax.plot([0], [ms], markermap[int(alg) % len(markermap)], color=colormap[int(alg) % len(colormap)])
        count = 1
        if noise in adaptive_alg:
            fractions = sorted([x for x in adaptive_alg[noise]])
        else:
            fractions = []
        for fraction in fractions:
            makespans = adaptive_alg[noise][fraction]
            ax.plot([count] * len(makespans), makespans, "ro")
            count += 1
        ax.set_xticks(list(range(0, count)))
        if noise in adaptive_alg:
            ax.set_xticklabels([""] + [str(x) for x in sorted(adaptive_alg[noise])], fontsize=6)
        else:
            ax.set_xticklabels([""], fontsize=6)
                

    plt.savefig(output_file)
    plt.close()
    return


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
    cursor = collection.find({})
    for doc in cursor:
        workflows.add(doc["workflow"])

    # Getting all workflows

    for workflow in sorted(workflows):
        workflow_name = workflow.split("/")[-1]
        generate_plot(workflow_name, workflow_name+".result.pdf")


