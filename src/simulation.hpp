#pragma once
#include <atomic>

extern std::atomic<bool> simulation_running;
extern std::atomic<int> target_tps;
extern std::atomic<bool> simulation_paused;

extern std::atomic<uint32_t> global_deaths_starvation;
extern std::atomic<uint32_t> global_deaths_old_age;
extern std::atomic<uint32_t> global_deaths_combat;

struct SimulationConfig {
    int seed;
    int initial_pop;
    int initial_forests;
    int initial_veins;
    int initial_ruins;
    int initial_exotic;
};

void init_world(const SimulationConfig& config);
void sim_thread(SimulationConfig config);
