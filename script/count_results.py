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

    total = 0
    workflows = set()
    cursor = collection.find({})
    for doc in cursor:
        total += 1
        workflows.add(doc["workflow"])
    sys.stdout.write("Total: " + str(total) + "\n")

    for workflow in sorted(workflows):
        sys.stdout.write("  " + workflow + ": ")
        count = 0
        cursor = collection.find({})
        for doc in cursor:
            if doc["workflow"] == workflow:
                count += 1
        sys.stdout.write(str(count) + "\n")



