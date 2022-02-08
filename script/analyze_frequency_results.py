#!/usr/bin/env python3
import ast
import sys
import matplotlib.pyplot as plt
from matplotlib.pyplot import figure


def plot_violin(axis, position, width, data, color):

    v = axis.violinplot(data, positions=[position], widths=[width], showmeans=True)
    for pc in v['bodies']:
        pc.set_facecolor(color)
        pc.set_facecolor(color)
        pc.set_edgecolor(color)
    v['cmaxes'].set_color(color)
    v['cmaxes'].set_linewidth(2)
    v['cmins'].set_color(color)
    v['cbars'].set_color(color)
    v['cbars'].set_linewidth(1)
    v['cmeans'].set_color(color)
    return v


if __name__ == "__main__":

    # Read already extracted results from the data file
    input_file_name = "frequency_extracted_results.dict"
    try:
        file = open(input_file_name, "r")
    except OSError:
        sys.stderr.write("Can't find extracted result file. Start Mongo and run the extract_all_results.py script first!\n");
        sys.exit(1)

    contents = file.read()
    results = ast.literal_eval(contents)

    frequencies = sorted(list(results.keys()))
    frequencies = [0.1, 0.2]

    workflows = sorted(list(results[frequencies[0]].keys()))
    workflows.remove("seismology-chameleon-700p-001.json")

    clusters = list(results[frequencies[0]][workflows[0]].keys())
    clusters = ["32:8:100Gf:100MBps,32:8:200Gf:200MBps,32:8:300Gf:300MBps"]



    algo = "8"

    analysis = {}
    for workflow in workflows:
        sys.stderr.write("Workflow " + workflow + ":\n")
        analysis[workflow] = {}
        for cluster in clusters:
            for frequency in frequencies:
                alg_makespan = results[frequency][workflow][cluster][algo]
                us_makespan = results[frequency][workflow][cluster]["us"]
                if us_makespan < 0:
                    break
                improvement = 100.0*(alg_makespan - us_makespan) / alg_makespan
                analysis[workflow][frequency] = improvement
                sys.stderr.write("  freq=" + str(frequency) + ": " + str(improvement) + "\n")

    for workflow in analysis:
        print(workflow + ": " + str(analysis[workflow][0.1] - analysis[workflow][0.2]))

                

