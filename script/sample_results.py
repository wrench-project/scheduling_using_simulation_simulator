#!/opt/local/bin/python3.9
import subprocess
import glob
from multiprocessing import Pool
import sys

def print_to_file(file_name, message):
    with open(file_name, "a") as f:
        f.write(message + "\n")
    return

def run_simulation(command):
    result = subprocess.check_output(command, shell=True)
    return float(result.strip())


def run_parallel_simulations(command_list, num_threads):
    with Pool(num_threads) as p:
        return p.map(run_simulation, command_list)


if __name__ == "__main__": 

    # Argument parsing
    ######################
    if (len(sys.argv) != 3):
        sys.stderr.write("Usage: " + sys.argv[0] + " <num threads> <outputfile>\n")
        sys.exit(1)

    try:
        num_threads = int(sys.argv[1])
    except:
        sys.stderr.write("Invalid argument\n")
        sys.exit(1)

    # Configuration
    ####################
    simulator = "../build/simulator "

    platform = ""
    platform += "--cluster cluster1:16:8:50Gf:20MBps "
    platform += "--cluster cluster2:16:4:100Gf:10MBps "
    platform += "--cluster cluster3:16:6:80Gf:15MBps "
    platform += "--reference_flops 100Gf "

    scheduler_change_trigger =          "--first_scheduler_change_trigger 0.00 "
    periodic_scheduler_change_trigger = "--periodic_scheduler_change_trigger 0.1 "
    speculative_work_fraction =         "--speculative_work_fraction 1.0 "

    num_trials  = 10

    # Create output file
    ####################################
    output_file_name = sys.argv[2]
    with open(output_file_name, "w") as f:
        f.write(platform + "\n")
        f.write("\n")
        f.write(scheduler_change_trigger + "\n")
        f.write(periodic_scheduler_change_trigger + "\n")
        f.write(speculative_work_fraction + "\n")
        f.write("--------------------------------------------------\n")

    # Create command prefix
    ####################################
    
    command_prefix = "../build/simulator " + platform + scheduler_change_trigger + periodic_scheduler_change_trigger + speculative_work_fraction
    
    # Get the number of algorithms
    ####################################
    num_algorithms = int(subprocess.check_output(simulator + "--print_all_algorithms | wc -l", shell=True, encoding='utf-8'). strip())

    algorithms=[str(x) for x in range(0,num_algorithms)]
    workflows = glob.glob("../workflows/*.json")
    
    for workflow in workflows:
        print_to_file(output_file_name, "WORKFLOW: " + workflow)

        # Run all algorithms on their own, in parallel
        #################################################
        commands_to_run = [(command_prefix + " --workflow " + workflow + " --algorithms " + alg) for alg in algorithms]
        makespans = run_parallel_simulations(commands_to_run, num_threads)
        print_to_file(output_file_name, "  SINGLE: " + str(sorted((val, idx) for (idx, val) in enumerate(makespans))))

        # Do the simulation thing
        for noise in [0.0, 0.1, 0.2, 0.4, 0.8]:
            makespans = []
            # Run all trials in parallel
            #################################################
            if (noise > 0):
                commands_to_run = [command_prefix + " --workflow " + workflow + " --algorithms 0-23 --simulation_noise " + str(noise)] * num_trials
            else:
                commands_to_run = [command_prefix + " --workflow " + workflow + " --algorithms 0-23 --simulation_noise " + str(noise)]
            makespans = run_parallel_simulations(commands_to_run, num_threads)
            print_to_file(output_file_name, "  ADAPTIVE (" + str(100*noise) + "% NOISE): " + str(sorted(makespans)))





