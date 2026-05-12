#include "simulation.hpp"
#include "gui_engine.hpp"
#include <thread>

int main() {
    SimulationConfig config;
    config.seed = 42; // Semente determinística
    config.initial_pop = 300;
    config.initial_forests = 30;
    config.initial_veins = 15;
    config.initial_ruins = 15;
    config.initial_exotic = 15;
    
    std::thread t_sim(sim_thread, config);
    ui_thread(); 
    
    simulation_running = false;
    t_sim.join();
    
    return 0;
}
