#!/usr/bin/env python3
import random
from os.path import exists
import glob
import sys
import json
from pymongo import MongoClient


def connect_to_mongo():

    # Setup Mongo
    try:
        mongo_url = "mongodb://localhost"
        mongo_client = MongoClient(host=mongo_url, serverSelectionTimeoutMS=1000)
        mydb = mongo_client["scheduling_with_simulation"]
        collection = mydb["results"]
    except:
        sys.stderr.write("Cannot connect to MONGO\n")
        sys.exit(1)
    return collection


def write_results_to_file(filename, data):
    f = open(filename, "w")
    f.write(str(data) + "\n")
    f.close() 
    print("Extracted result dictionary written to file " + filename)


if __name__ == "__main__":

    collection = connect_to_mongo()

    # Get values for fields in database
    workflows = set()
    clusters = set()
#    baseline_algorithms = set()
    work_fractions = set()
    noises = set()
    frequencies = set()
    cursor = collection.find({})
    for doc in cursor:
        clusters.add(doc["clusters"])
        workflows.add(doc["workflow"])
        work_fractions.add(doc["speculative_work_fraction"])
        noises.add(doc["simulation_noise"])
        frequencies.add(doc["periodic_scheduler_change_trigger"])
#        if len(doc["algorithms"].split(",")) == 1:
#            baseline_algorithms.add(doc["algorithms"])

    workflows = sorted(list(workflows))
    clusters = sorted(list(clusters))
#    base_line_algorithms = sorted(list(algorithms))
    work_fractions = sorted(list(work_fractions))
    frequencies = sorted(list(frequencies))
    noises = sorted(list(noises))
    frequencies = sorted(list(frequencies))


    ##
    ## IDEAL RESULTS
    ##
    sys.stderr.write("Extracting 'ideal' results...\n")
    results = {}
    for workflow in workflows:
        results[workflow] = {}
        for cluster in clusters:
            results[workflow][cluster] = {}
            cursor = collection.find({"clusters":cluster,"workflow":workflow})
            for doc in cursor:
                if (len(doc["algorithms"].split(",")) != 1) and (doc["speculative_work_fraction"] == 1.0) and (doc["simulation_noise"] == 0.0):
                    our_makespan = doc["makespan"]
                    algos_used = list(set(doc["algorithm_sequence"].split(",")))
                    results[workflow][cluster]["us"] = [our_makespan, algos_used]
                else:
                    results[workflow][cluster][doc["algorithms"]] = doc["makespan"]

    write_results_to_file("ideal_extracted_results.dict", results)


    ##
    ## RESULTS FOR WORK FRACTIONS
    ##
    sys.stderr.write("Extracting 'work fraction' results...\n")
    results = {}
    for work_fraction in work_fractions:
        sys.stderr.write("\tProcessing work fraction " + str(work_fraction) + "\n")
        results[work_fraction] = {}
        for workflow in workflows:
            results[work_fraction][workflow] = {}
            for cluster in clusters:
                results[work_fraction][workflow][cluster] = {}
                cursor = collection.find({"clusters":cluster,"workflow":workflow})
                for doc in cursor:
                    if (len(doc["algorithms"].split(",")) != 1) and (doc["simulation_noise"] == 0.0) and (doc["speculative_work_fraction"] == work_fraction):
                        results[work_fraction][workflow][cluster]["us"] = [doc["makespan"], doc["algorithms"].split(",")]
                    else:
                        results[work_fraction][workflow][cluster][doc["algorithms"]] = doc["makespan"]

    write_results_to_file("work_fraction_extracted_results.dict", results)

    ##
    ## RESULTS FOR NOISE
    ##
    sys.stderr.write("Extracting 'noise' results...\n")
    results = {}
    for noise in noises:
        sys.stderr.write("\tProcessing noise " + str(noise) + "\n")
        results[noise] = {}
        for workflow in workflows:
            results[noise][workflow] = {}
            for cluster in clusters:
                results[noise][workflow][cluster] = {}
                cursor = collection.find({"clusters":cluster,"workflow":workflow})
                sum_us_makespans = 0
                num_us_makespans = 0
                for doc in cursor:
                    if (len(doc["algorithms"].split(",")) != 1) and (doc["speculative_work_fraction"] == 1.0) and (doc["simulation_noise"] == noise):
                        sum_us_makespans += doc["makespan"]
                        num_us_makespans += 1
                    else:
                        results[noise][workflow][cluster][doc["algorithms"]] = doc["makespan"]
                results[noise][workflow][cluster]["us"] = sum_us_makespans / num_us_makespans
    write_results_to_file("noise_extracted_results.dict", results)



    ##
    ## RESULTS FOR NOISE AND FREQUENCIES
    ##
    sys.stderr.write("Extracting 'frequency and noise' results...\n")
    results = {}
    for noise in noises:
        sys.stderr.write("\tProcessing noise " + str(noise) + "\n")
        results[noise] = {}
        for frequency in frequencies:
            sys.stderr.write("\t\tProcessing frequency " + str(frequency) + "\n")
            results[noise][frequency] = {}
            for workflow in workflows:
                sys.stderr.flush()
                results[noise][frequency][workflow] = {}
                for cluster in clusters:
                    cursor = collection.find({"clusters":cluster,"workflow":workflow})
                    sum_us_makespans = 0
                    num_us_makespans = 0
                    for doc in cursor:
                        if (len(doc["algorithms"].split(",")) != 1) and (doc["speculative_work_fraction"] == 1.0) and (doc["simulation_noise"] == noise) and (doc["periodic_scheduler_change_trigger"] == frequency):
                            sum_us_makespans += doc["makespan"]
                            num_us_makespans += 1
                        else:
                            results[noise][frequency][workflow][doc["algorithms"]] = doc["makespan"]
                    if num_us_makespans > 0:
                        results[noise][frequency][workflow]["us"] = sum_us_makespans / num_us_makespans
                    else:
                        results[noise][frequency][workflow]["us"] = -1.0

    write_results_to_file("frequency_noise_extracted_results.dict", results)

