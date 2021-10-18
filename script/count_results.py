#!/usr/bin/env python3
import random
import subprocess
import sys
import json
from pymongo import MongoClient


if __name__ == "__main__":

    # Setup Mongo
    try:
        mongo_url = "mongodb://localhost"
        db_name = "scheduling_with_simulation_results"
        mongo_client = MongoClient(host=mongo_url, serverSelectionTimeoutMS=1000)
        mydb = mongo_client["scheduling_with_simulation"]
        collection = mydb["results"]
    except:
        sys.write("Cannot connect to MONGO\n")
        sys.exit(1)


    workflows = set()
    clusters = set()
    cursor = collection.find()
    for doc in cursor:
        clusters.add(doc["clusters"])
        workflows.add(doc["workflow"])

    workflows = sorted(list(workflows))
    clusters = sorted(list(clusters))
    
    for clusters_spec in clusters:
        sys.stdout.write("CLUSTERS: " + clusters_spec + "\n")
        total = 0
        workflows = set()
        cursor = collection.find({"clusters":clusters_spec})
        for doc in cursor:
            total += 1
            workflows.add(doc["workflow"])
        sys.stdout.write("  Total: " + str(total) + "\n")
    
        for workflow in sorted(workflows):
            sys.stdout.write("    " + workflow + ": ")
            count = collection.count({"clusters":clusters_spec, "workflow":workflow})
            sys.stdout.write(str(count) + "\n")



