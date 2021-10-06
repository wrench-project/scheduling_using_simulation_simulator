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

    cursor = collection.find({})
    count = 0
    for doc in cursor:
        count += 1

    print(str(count)+" results")


