#!/usr/bin/env python3
import ast
import random
import sys
import matplotlib.pyplot as plt
from matplotlib.pyplot import figure


if __name__ == "__main__":

    # Read already extracted results from the data file
    input_file_name = "ideal_extracted_results_sequence_by_cluster.dict"
    try:
        file = open(input_file_name, "r")
    except OSError:
        sys.stderr.write("Can't open file 'ideal_extracted_results.dict'. Start Mongo and run the ideal_extract_results.py script first!\n");
    contents = file.read()
    results = ast.literal_eval(contents)

    clusters = list(results.keys())
    algorithms = list(results['96:8:100Gf:100MBps'].keys())
    algorithms = [str(y) for y in sorted([int(x) for x in algorithms])]

    cluster_id_map = {}
    cluster_id_map['96:8:100Gf:100MBps'] = "1"
    cluster_id_map['48:8:150Gf:100MBps,48:8:50Gf:100MBps'] = "2"
    cluster_id_map['48:8:400Gf:10MBps,48:8:50Gf:100MBps'] = "3"
    cluster_id_map['32:8:100Gf:100MBps,32:8:200Gf:200MBps,32:8:300Gf:300MBps'] = "4"
    cluster_id_map['32:8:100Gf:100MBps,32:8:200Gf:300MBps,32:8:300Gf:200MBps'] = "5"
    cluster_id_map['32:8:100Gf:200MBps,32:8:200Gf:100MBps,32:8:300Gf:300MBps'] = "6"
    cluster_id_map['32:8:100Gf:200MBps,32:8:200Gf:300MBps,32:8:300Gf:100MBps'] = "7"
    cluster_id_map['32:8:100Gf:300MBps,32:8:200Gf:200MBps,32:8:300Gf:100MBps'] = "8"
    cluster_id_map['32:8:100Gf:300MBps,32:8:200Gf:100MBps,32:8:300Gf:200MBps'] = "1"
    clusters = dict(sorted(cluster_id_map.items(), key=lambda item: item[1])).keys()

    ### PLOTTING

    fontsize = 18

    output_file = "improvement_ideal_algorithm_usage_by_cluster.pdf"
    sys.stderr.write("Generating " + output_file + "...\n")

    f, ax1 = plt.subplots(1, 1, sharey=True, figsize=(12,6))

    data = []

    ytick_labels = []

    for cluster in reversed(clusters):
        ytick_labels.append("P"+cluster_id_map[cluster])
        alg_data = []
        for algorithm in algorithms:
            alg_data.append(results[cluster][algorithm])
        data.append(alg_data)

    yticks = list(range(0,len(clusters)))
    ax1.set_yticks(yticks)
    ax1.set_yticklabels(ytick_labels, fontsize = fontsize-2)

    xticks = list(range(0,len(algorithms)))
    ax1.set_xticks(xticks)
    ax1.set_xticklabels(xticks, fontsize = fontsize-4)

    ax1.set_ylabel("Cluster", fontsize=fontsize-1)
    ax1.set_xlabel("Algorithm", fontsize=fontsize-1)

    plt.tick_params(left=False, bottom=False)


    plt.imshow(data, cmap='viridis', interpolation='nearest')


    f.tight_layout()
                
    plt.savefig(output_file)
    plt.close()

    sys.stderr.write("Plot saved to file " + output_file + "\n")

