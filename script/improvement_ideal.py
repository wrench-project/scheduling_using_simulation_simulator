#!/usr/bin/env python3
import random
import subprocess
from os.path import exists
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

    workflow_id_map = {}
    workflow_id_map['montage-chameleon-2mass-10d-001.json'] = "1"
    workflow_id_map['epigenomics-chameleon-ilmn-4seq-50k-001.json'] = "2"
    workflow_id_map['bwa-chameleon-large-003.json'] = "3"
    workflow_id_map['cycles-chameleon-2l-2c-12p-001.json'] = "4"
    workflow_id_map['seismology-chameleon-700p-001.json'] = "5"
    workflow_id_map['1000genome-chameleon-8ch-250k-001.json'] = "6"
    workflow_id_map['blast-chameleon-medium-002.json'] = "7"
    workflow_id_map['soykb-chameleon-10fastq-20ch-001.json'] = "8"
    workflow_id_map['srasearch-chameleon-10a-003.json'] = "9"

    clusters_id_map = {}
    clusters_id_map['96:8:100Gf:100MBps'] = "1"
    clusters_id_map['48:8:150Gf:100MBps,48:8:50Gf:100MBps'] = "2"
    clusters_id_map['48:8:400Gf:10MBps,48:8:50Gf:100MBps'] = "3"
    clusters_id_map['32:8:100Gf:100MBps,32:8:200Gf:200MBps,32:8:300Gf:300MBps'] = "4"
    clusters_id_map['32:8:100Gf:100MBps,32:8:200Gf:300MBps,32:8:300Gf:200MBps'] = "5"
    clusters_id_map['32:8:100Gf:200MBps,32:8:200Gf:100MBps,32:8:300Gf:300MBps'] = "6"
    clusters_id_map['32:8:100Gf:200MBps,32:8:200Gf:300MBps,32:8:300Gf:100MBps'] = "7"
    clusters_id_map['32:8:100Gf:300MBps,32:8:200Gf:200MBps,32:8:300Gf:100MBps'] = "8"
    clusters_id_map['32:8:100Gf:300MBps,32:8:200Gf:100MBps,32:8:300Gf:200MBps'] = "1"

    workflow_color_map = {}
    workflow_color_map['montage-chameleon-2mass-10d-001.json'] = "red"
    workflow_color_map['epigenomics-chameleon-ilmn-4seq-50k-001.json'] = "blue"
    workflow_color_map['bwa-chameleon-large-003.json'] = "green"
    workflow_color_map['cycles-chameleon-2l-2c-12p-001.json'] = "darkslategray"
    workflow_color_map['seismology-chameleon-700p-001.json'] = "chocolate"
    workflow_color_map['1000genome-chameleon-8ch-250k-001.json'] = "orange"
    workflow_color_map['blast-chameleon-medium-002.json'] = "olive"
    workflow_color_map['soykb-chameleon-10fastq-20ch-001.json'] = "gray"
    workflow_color_map['srasearch-chameleon-10a-003.json'] = "magenta"


    speculative_work_fraction = 1.0


    algorithms_used = set()

    ### COMPUTING RESULTS
    if False:
        results = {}
        for workflow in workflows:
            sys.stderr.write("  Processing workflow " + str(workflow) + " ")
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
                        if (len(doc["algorithms"].split(",")) != 1) and (doc["speculative_work_fraction"] == 1.0) and (doc["simulation_noise"] == 0.0):
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
                sys.stderr.write("\n")
        print("\tresults = " + str(results))
        print("\talgorithms_used = " + str(algorithms_used))


    results = {'1000genome-chameleon-8ch-250k-001.json': {'32:8:100Gf:100MBps,32:8:200Gf:200MBps,32:8:300Gf:300MBps': [0.027405322818006218, 4], '32:8:100Gf:100MBps,32:8:200Gf:300MBps,32:8:300Gf:200MBps': [0.06406313158806995, 5], '32:8:100Gf:200MBps,32:8:200Gf:100MBps,32:8:300Gf:300MBps': [-0.0006737027335056998, 3], '32:8:100Gf:200MBps,32:8:200Gf:300MBps,32:8:300Gf:100MBps': [-0.024432399218765787, 5], '32:8:100Gf:300MBps,32:8:200Gf:100MBps,32:8:300Gf:200MBps': [-0.009138090730150237, 2], '32:8:100Gf:300MBps,32:8:200Gf:200MBps,32:8:300Gf:100MBps': [0.02739114335862691, 4], '48:8:150Gf:100MBps,48:8:50Gf:100MBps': [0.0026151623523937577, 4], '48:8:400Gf:10MBps,48:8:50Gf:100MBps': [0.0001350380008159347, 2], '96:8:100Gf:100MBps': [0.005331379587193421, 4]}, 'blast-chameleon-medium-002.json': {'32:8:100Gf:100MBps,32:8:200Gf:200MBps,32:8:300Gf:300MBps': [1.0708381991392866, 4], '32:8:100Gf:100MBps,32:8:200Gf:300MBps,32:8:300Gf:200MBps': [1.4970768618327275, 3], '32:8:100Gf:200MBps,32:8:200Gf:100MBps,32:8:300Gf:300MBps': [2.19078779259953, 4], '32:8:100Gf:200MBps,32:8:200Gf:300MBps,32:8:300Gf:100MBps': [1.8840796657207894, 3], '32:8:100Gf:300MBps,32:8:200Gf:100MBps,32:8:300Gf:200MBps': [0.0, 2], '32:8:100Gf:300MBps,32:8:200Gf:200MBps,32:8:300Gf:100MBps': [2.1473896387016946, 4], '48:8:150Gf:100MBps,48:8:50Gf:100MBps': [2.057299825273951, 3], '48:8:400Gf:10MBps,48:8:50Gf:100MBps': [1.6306735047423029, 2], '96:8:100Gf:100MBps': [0.7673541485250198, 3]}, 'bwa-chameleon-large-003.json': {'32:8:100Gf:100MBps,32:8:200Gf:200MBps,32:8:300Gf:300MBps': [-0.04709122754679327, 2], '32:8:100Gf:100MBps,32:8:200Gf:300MBps,32:8:300Gf:200MBps': [0.11238526891255628, 2], '32:8:100Gf:200MBps,32:8:200Gf:100MBps,32:8:300Gf:300MBps': [0.04017338929109919, 2], '32:8:100Gf:200MBps,32:8:200Gf:300MBps,32:8:300Gf:100MBps': [0.03042544784886557, 2], '32:8:100Gf:300MBps,32:8:200Gf:100MBps,32:8:300Gf:200MBps': [0.0011778727038214984, 2], '32:8:100Gf:300MBps,32:8:200Gf:200MBps,32:8:300Gf:100MBps': [0.0007843490148015743, 2], '48:8:150Gf:100MBps,48:8:50Gf:100MBps': [0.04912031370126085, 3], '48:8:400Gf:10MBps,48:8:50Gf:100MBps': [0.0018873541297603515, 2], '96:8:100Gf:100MBps': [0.029498335888829195, 3]}, 'cycles-chameleon-2l-2c-12p-001.json': {'32:8:100Gf:100MBps,32:8:200Gf:200MBps,32:8:300Gf:300MBps': [2.5232383650777037, 3], '32:8:100Gf:100MBps,32:8:200Gf:300MBps,32:8:300Gf:200MBps': [0.8002978682855376, 4], '32:8:100Gf:200MBps,32:8:200Gf:100MBps,32:8:300Gf:300MBps': [2.6029135092042646, 5], '32:8:100Gf:200MBps,32:8:200Gf:300MBps,32:8:300Gf:100MBps': [-0.05623134828185918, 2], '32:8:100Gf:300MBps,32:8:200Gf:100MBps,32:8:300Gf:200MBps': [1.5755948509382525, 4], '32:8:100Gf:300MBps,32:8:200Gf:200MBps,32:8:300Gf:100MBps': [2.0264663282379733, 4], '48:8:150Gf:100MBps,48:8:50Gf:100MBps': [0.0048260994129283846, 3], '48:8:400Gf:10MBps,48:8:50Gf:100MBps': [1.8765159266393665, 4], '96:8:100Gf:100MBps': [0.1509882503866, 4]}, 'epigenomics-chameleon-ilmn-4seq-50k-001.json': {'32:8:100Gf:100MBps,32:8:200Gf:200MBps,32:8:300Gf:300MBps': [4.672578567926875, 5], '32:8:100Gf:100MBps,32:8:200Gf:300MBps,32:8:300Gf:200MBps': [0.5735342418502606, 4], '32:8:100Gf:200MBps,32:8:200Gf:100MBps,32:8:300Gf:300MBps': [1.5938638151601474, 5], '32:8:100Gf:200MBps,32:8:200Gf:300MBps,32:8:300Gf:100MBps': [0.9874313372920283, 5], '32:8:100Gf:300MBps,32:8:200Gf:100MBps,32:8:300Gf:200MBps': [1.2580380865173424, 6], '32:8:100Gf:300MBps,32:8:200Gf:200MBps,32:8:300Gf:100MBps': [1.0507898569616339, 4], '48:8:150Gf:100MBps,48:8:50Gf:100MBps': [0.9240394220193925, 4], '48:8:400Gf:10MBps,48:8:50Gf:100MBps': [0.6347655431724302, 5], '96:8:100Gf:100MBps': [0.5237447009567611, 4]}, 'montage-chameleon-2mass-10d-001.json': {'32:8:100Gf:100MBps,32:8:200Gf:200MBps,32:8:300Gf:300MBps': [2.9295851279492773, 3], '32:8:100Gf:100MBps,32:8:200Gf:300MBps,32:8:300Gf:200MBps': [1.9550872258647962, 3], '32:8:100Gf:200MBps,32:8:200Gf:100MBps,32:8:300Gf:300MBps': [6.901419239302743, 3], '32:8:100Gf:200MBps,32:8:200Gf:300MBps,32:8:300Gf:100MBps': [7.511684386860746, 5], '32:8:100Gf:300MBps,32:8:200Gf:100MBps,32:8:300Gf:200MBps': [0.39352151565815036, 3], '32:8:100Gf:300MBps,32:8:200Gf:200MBps,32:8:300Gf:100MBps': [5.843595300684115, 5], '48:8:150Gf:100MBps,48:8:50Gf:100MBps': [0.0, 1], '48:8:400Gf:10MBps,48:8:50Gf:100MBps': [19.08731084234091, 4], '96:8:100Gf:100MBps': [0.06869079270328715, 3]}, 'seismology-chameleon-700p-001.json': {'32:8:100Gf:100MBps,32:8:200Gf:200MBps,32:8:300Gf:300MBps': [0.0, 1], '32:8:100Gf:100MBps,32:8:200Gf:300MBps,32:8:300Gf:200MBps': [0.0, 1], '32:8:100Gf:200MBps,32:8:200Gf:100MBps,32:8:300Gf:300MBps': [0.0, 1], '32:8:100Gf:200MBps,32:8:200Gf:300MBps,32:8:300Gf:100MBps': [0.0, 1], '32:8:100Gf:300MBps,32:8:200Gf:100MBps,32:8:300Gf:200MBps': [0.0, 1], '32:8:100Gf:300MBps,32:8:200Gf:200MBps,32:8:300Gf:100MBps': [0.0, 1], '48:8:150Gf:100MBps,48:8:50Gf:100MBps': [0.0, 2], '48:8:400Gf:10MBps,48:8:50Gf:100MBps': [0.0, 2], '96:8:100Gf:100MBps': [-0.0003151766475474522, 2]}, 'soykb-chameleon-10fastq-20ch-001.json': {'32:8:100Gf:100MBps,32:8:200Gf:200MBps,32:8:300Gf:300MBps': [-0.6981345401911107, 3], '32:8:100Gf:100MBps,32:8:200Gf:300MBps,32:8:300Gf:200MBps': [-0.6943945099476891, 3], '32:8:100Gf:200MBps,32:8:200Gf:100MBps,32:8:300Gf:300MBps': [-0.25569223781923966, 3], '32:8:100Gf:200MBps,32:8:200Gf:300MBps,32:8:300Gf:100MBps': [-0.2185037565949868, 4], '32:8:100Gf:300MBps,32:8:200Gf:100MBps,32:8:300Gf:200MBps': [-0.3241428165008427, 3], '32:8:100Gf:300MBps,32:8:200Gf:200MBps,32:8:300Gf:100MBps': [-0.20013024134705176, 3], '48:8:150Gf:100MBps,48:8:50Gf:100MBps': [-2.3278568696819697, 3], '48:8:400Gf:10MBps,48:8:50Gf:100MBps': [-0.5674645083859974, 4], '96:8:100Gf:100MBps': [-0.5559821805617857, 3]}, 'srasearch-chameleon-10a-003.json': {'32:8:100Gf:100MBps,32:8:200Gf:200MBps,32:8:300Gf:300MBps': [0.27834904086075246, 2], '32:8:100Gf:100MBps,32:8:200Gf:300MBps,32:8:300Gf:200MBps': [0.2783490410508389, 2], '32:8:100Gf:200MBps,32:8:200Gf:100MBps,32:8:300Gf:300MBps': [0.3211427036718034, 2], '32:8:100Gf:200MBps,32:8:200Gf:300MBps,32:8:300Gf:100MBps': [0.2783470410109691, 2], '32:8:100Gf:300MBps,32:8:200Gf:100MBps,32:8:300Gf:200MBps': [0.3211427032706256, 2], '32:8:100Gf:300MBps,32:8:200Gf:200MBps,32:8:300Gf:100MBps': [0.2783470404963485, 2], '48:8:150Gf:100MBps,48:8:50Gf:100MBps': [0.0, 3], '48:8:400Gf:10MBps,48:8:50Gf:100MBps': [0.0, 1], '96:8:100Gf:100MBps': [0.0, 1]}}
    algorithms_used = {'27', '2', '0', '6', '31', '14', '20', '13', '21', '25', '26', '23', '22', '18', '7', '35', '32', '8', '4', '24', '29', '11', '5', '17'}

    ### PLOTTING

    output_file = "improvement_ideal.pdf"
    sys.stderr.write("Generating " + output_file + "...\n")


    f, ax1 = plt.subplots(1, 1, sharey=True, figsize=(12,6))
    
    ax1.yaxis.grid()

    #speculative_work_fractions =[0.7, 0.8, 0.9, 1.0]

    #ax1.set_xlim(-0.04,0.44)

    x_value = 0.1
    x_ticks = []
    x_ticklabels = []
    for workflow in workflows:
        x_ticks.append(x_value)
        x_ticklabels.append(workflow.split('-')[0])
        y_to_plot = []
        for cluster in clusters:
            y_to_plot.append(results[workflow][cluster][0])
        x_to_plot = [x_value + random.uniform(-0.02, +0.02) for x in y_to_plot]
        mean_y = sum(y_to_plot) / len(y_to_plot)

        ax1.plot(x_to_plot,y_to_plot, '.', color=workflow_color_map[workflow], markersize=14)
        ax1.plot([x_value - 0.025, x_value + 0.025], [mean_y, mean_y], color=workflow_color_map[workflow], linewidth=4)
        x_value += 0.1
             
    ax1.set_xticks(x_ticks)
    ax1.set_xticklabels(x_ticklabels, rotation=45, fontsize=14)

    ax1.set_ylabel("% improvement", fontsize=14)


    plt.ylim(-3, 20)
    plt.yticks(fontsize=14)
    f.tight_layout()
                
    plt.savefig(output_file)
    plt.close()


    num_one_algo_was_used = 0
    for workflow in workflows:
        max_num_algorithms = 0
        ave_num_algorithms = 0
        for cluster in clusters:
            if results[workflow][cluster][1] == 1:
                num_one_algo_was_used += 1
            max_num_algorithms = max(max_num_algorithms, results[workflow][cluster][1])
            ave_num_algorithms += results[workflow][cluster][1]
        ave_num_algorithms /= len(clusters)
        print("# of algs for " + workflow + ": max=" + str(max_num_algorithms) + "; ave=" + str(ave_num_algorithms))
                

    print("Number of cases in which our appraoch used one algorithm: " + str(num_one_algo_was_used) + " / " + str(len(workflows) * len(clusters)))
    print("Number of algorithms used at some point: " + str(len(algorithms_used)))

