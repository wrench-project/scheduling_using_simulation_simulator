#!/usr/bin/env python3
import sys
import ast
import matplotlib.pyplot as plt
from matplotlib.pyplot import figure

global collection

if __name__ == "__main__":


    # Read already extracted results from the data file
    input_file_name = "ideal_extracted_results.dict"
    try:
        file = open(input_file_name, "r")
    except OSError:
        sys.stderr.write("Can't find extracted result file. Start Mongo and run the extract_all_results.py script first!\n");
        sys.exit(1)
        
    contents = file.read()
    results = ast.literal_eval(contents)
    
    # Get values for fieds
    workflows = sorted(list(results.keys()))
    clusters = sorted(results[workflows[0]].keys())
    algorithms = sorted(list(results[workflows[0]][clusters[0]].keys()))
    algorithms.remove("us")
    num_scenarios = len(workflows) * len(clusters)

    dfb = {i: 0.0 for i in algorithms}
    worst_dfb = {i: 0.0 for i in algorithms}
    
    for workflow in workflows:
        for cluster in clusters:
            makespans = {algo: results[workflow][cluster][algo] for algo in algorithms} 
            best = min(makespans.values())
            for algo, makespan in makespans.items():
                dfb_value = 100 * (makespan - best) / best
                dfb[algo] += dfb_value
                if worst_dfb[algo] < dfb_value:
                    worst_dfb[algo] = dfb_value

    dfb = {algo: (value / num_scenarios) for algo, value in dfb.items()}

    dfb = dict(sorted(dfb.items(), key=lambda x: x[1]))

    for algo, avg_dfb in dfb.items():
        print("["+algo+"] " + str(round(avg_dfb, 2)) + "%  (worst dfb: " + str(round(worst_dfb[algo], 2)) + "%)")


#  #  
#w #                                    .: <10%
#o #                                    *: 10% < <20%
#r # .
#k #
#f #
#l #
#  #--------------------------
#          clusters
