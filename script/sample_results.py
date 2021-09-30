#!/opt/local/bin/python3.9
import subprocess
import glob


command_prefix = "../build/simulator --cluster cluster1:16:8:50Gf:20MBps --cluster cluster2:16:4:100Gf:10MBps --cluster cluster3:16:6:80Gf:15MBps  --reference_flops 100Gf  --first_scheduler_change_trigger 0.00 --periodic_scheduler_change_trigger 0.1 --speculative_work_fraction 1.0"

algorithms=[str(x) for x in range(0,24)]
workflows = glob.glob("../workflows/*.json")

for workflow in workflows:
    print("WORKFLOW: " + workflow)
    makespans = []
    # Run all algorithms on their own
    for alg in algorithms:
        #print("ALGORITHM: " + alg)
        command = command_prefix + " --workflow " + workflow + " --algorithms " + alg
        result = subprocess.check_output(command, shell=True)
        makespans += [float(result.strip())]
    print("  SINGLE: " + str(sorted((val, idx) for (idx, val) in enumerate(makespans))))
    # Do the simulation thing
    for noise in [0.0, 0.1, 0.2, 0.4, 0.8]:
        makespans = []
        for trial in range(0,5):
            command = command_prefix + " --workflow " + workflow + " --algorithms 0-23 --simulation_noise " + str(noise)
            result = subprocess.check_output(command, shell=True)
            makespans += [float(result.strip())]
        print("  ADAPTIVE (" + str(100*noise) + "% NOISE): " + str(sorted(makespans)))





