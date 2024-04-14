#include "InitialLoadCreator.h"


void InitialLoadCreator::create_initial_load(
        double initial_load_max_duration,
        int initial_load_duration_seed,
        double initial_load_prob_core_loaded,
        std::unordered_map<std::shared_ptr<wrench::BareMetalComputeService>, std::map<std::string, unsigned long>> &idle_cores_map) {

    // Initial  \load amount RNG
    auto load_duration_random_dist = new std::uniform_real_distribution<double>(0, initial_load_max_duration);
    auto load_duration_rng = new std::mt19937(initial_load_duration_seed);
//    (*macro_random_dist)(*macro_rng);

    auto coin_random_dist = new std::uniform_real_distribution<double>(0, 1.0);
    auto coin_rng = new std::mt19937(initial_load_duration_seed);

    for (auto const &mbcs_item : idle_cores_map) {
        for (auto const &host_item : mbcs_item.second) {
            unsigned long original_num_idle_core = idle_cores_map[mbcs_item.first][host_item.first];
            for (int i = 0; i < original_num_idle_core; i++) {
                // flip a coin
                if ((*coin_random_dist)(*coin_rng) > initial_load_prob_core_loaded) {
//                    std::cerr << "NOT CREATING INITIAL LOAD ON " << mbcs_item.first->getName() << ":" << host_item.first << "\n";
                    break;
                }
                // reduce idle count by one
                idle_cores_map[mbcs_item.first][host_item.first] -= 1;
                // Create an Actor that will do +1 at a given time
                double load_duration = (*load_duration_random_dist)(*load_duration_rng);
                auto key1 = mbcs_item.first;
                auto key2 = host_item.first;
//                std::cerr << "CREATING INITIAL LOAD ON " << key1->getName() << ":" << key2 << "\n";
                simgrid::s4u::Actor::create(
                        "Client", simgrid::s4u::this_actor::get_host(),
                        [load_duration, &idle_cores_map, key1, key2]() {
                            simgrid::s4u::this_actor::sleep_for(load_duration);
//                            std::cerr << wrench::Simulation::getCurrentSimulatedDate() << " INITIAL LOAD: CORE BECOMES AVAILABLE " << key1->getName() << ":" << key2 << "\n";
                            idle_cores_map[key1][key2] += 1;
                        });
            }
        }
    }
}