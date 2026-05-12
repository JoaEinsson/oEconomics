#include "simulation.hpp"
#include "systems.hpp"
#include "telemetry.hpp"
#include <chrono>
#include <thread>
#include <random>
#include <algorithm>
#include <fstream>

std::atomic<bool> simulation_running{true};
std::atomic<int> target_tps{10};
std::atomic<bool> simulation_paused{false};

std::mutex ui_mutex;
UIState shared_state;

std::ofstream csv_file;

Agents world_agents;
Items world_items;
Markets world_markets;
Orders world_orders;
TrustGraph world_graph;

uint64_t current_tick = 0;

void init_world(const SimulationConfig& config) {
    world_agents.pos.clear();
    world_agents.energy.clear();
    world_agents.age.clear();
    world_agents.lifespan.clear();
    world_agents.phenotype.clear();
    world_agents.inventory.clear();
    world_agents.intent.clear();
    world_agents.basal_cost.clear();
    
    world_items.owner_id.clear();
    world_items.pos.clear();
    world_items.mass.clear();
    world_items.matter.clear();
    world_items.integrity.clear();
    world_items.anchored.clear();
    
    world_agents.pos.resize(config.initial_pop);
    world_agents.energy.resize(config.initial_pop, 100.0f);
    world_agents.age.resize(config.initial_pop, 0);
    world_agents.lifespan.resize(config.initial_pop, 0);
    world_agents.phenotype.resize(config.initial_pop);
    world_agents.inventory.resize(config.initial_pop);
    world_agents.intent.resize(config.initial_pop);
    world_agents.basal_cost.resize(config.initial_pop, 1.0f);

    world_items.matter.resize(5000);
    world_items.integrity.resize(5000, 100.0f);
    world_items.mass.resize(5000, 0.0f);
    world_items.owner_id.resize(5000, 0);
    world_items.pos.resize(5000);
    world_items.anchored.resize(5000, false);
    world_markets.active_orders.clear();
    world_orders.agent.clear();
    
    world_graph.source.clear();
    world_graph.target.clear();
    world_graph.trust.clear();
    world_graph.active.clear();

    csv_file.open("telemetry.csv");
    if(csv_file.is_open()) {
        csv_file << "Tick,Pop_Alive,Avg_Energy,Avg_Lifespan,Avg_Aggro,Avg_Farm,Active_Edges,Farms_Count,Avg_Age,Avg_Repro_Th,Avg_Metabolism,Avg_Integrity\n";
    }

    std::mt19937 rng(config.seed);
    std::uniform_real_distribution<float> pos_x(0, GRID_WIDTH - 1);
    std::uniform_real_distribution<float> pos_y(0, GRID_HEIGHT - 1);
    std::uniform_int_distribution<int> rand_life(200, 800);
    std::uniform_real_distribution<float> chance(0.0f, 1.0f);

    for (int i = 1; i < config.initial_pop; i++) {
        world_agents.pos[i] = {pos_x(rng), pos_y(rng)};
        world_agents.lifespan[i] = rand_life(rng);
        
        world_agents.phenotype[i][GENE_LIFESPAN] = (float)world_agents.lifespan[i];
        world_agents.phenotype[i][GENE_REPRO_TH] = 80.0f; 
        world_agents.phenotype[i][GENE_METABOLISM] = 1.0f;
        world_agents.phenotype[i][MEME_TRADE_TH] = THRESHOLD;
        world_agents.phenotype[i][MEME_FARM] = chance(rng); 
        world_agents.phenotype[i][MEME_AGGRO] = chance(rng); 
        
        world_agents.basal_cost[i] = world_agents.phenotype[i][GENE_METABOLISM];
        
        for (int j = 0; j < INV_CAP; j++) world_agents.inventory[i][j] = 0;
        world_agents.intent[i].type = Agents::Idle;
        world_agents.intent[i].target_agent = 0;
    }

    world_items.matter.resize(5000);
    world_items.integrity.resize(5000, 100.0f);
    world_items.mass.resize(5000, 0.0f);
    world_items.owner_id.resize(5000, 0);
    world_items.pos.resize(5000);

    int item_id = 1;
    for (int i = 1; i < config.initial_pop; i++) {
        world_agents.inventory[i][0] = item_id;
        world_items.owner_id[item_id] = i;
        world_items.mass[item_id] = 1.0f;
        world_items.matter[item_id] = {50.0f, 0, 0, 0}; 
        item_id++;
    }
    
    // Geração de Geografia (Fontes Primordiais Estáticas)
    // Bosques (Trees) - Fonte de Calorias
    for(int i=0; i<config.initial_forests; i++) {
        world_items.mass[item_id] = 100.0f;
        world_items.pos[item_id] = {pos_x(rng), pos_y(rng)};
        world_items.owner_id[item_id] = 0;
        world_items.integrity[item_id] = 100000.0f; 
        world_items.matter[item_id] = {999.0f, 300.0f, 0, 0}; 
        world_items.anchored[item_id] = true; // Nasce ancorado
        item_id++;
    }
    
    // Jazidas (Ore Veins) - Fonte de Rochas
    for(int i=0; i<config.initial_veins; i++) {
        world_items.mass[item_id] = 100.0f;
        world_items.pos[item_id] = {pos_x(rng), pos_y(rng)};
        world_items.owner_id[item_id] = 0;
        world_items.integrity[item_id] = 100000.0f;
        world_items.matter[item_id] = {0.0f, 999.0f, 0, 0}; 
        world_items.anchored[item_id] = true; // Nasce ancorado
        item_id++;
    }

    // Ruínas (Ruins) - Fonte de Informação (Tábuas)
    for(int i=0; i<config.initial_ruins; i++) {
        world_items.mass[item_id] = 100.0f;
        world_items.pos[item_id] = {pos_x(rng), pos_y(rng)};
        world_items.owner_id[item_id] = 0;
        world_items.integrity[item_id] = 100000.0f;
        world_items.matter[item_id] = {0.0f, 0.0f, 0.0f, 999.0f}; 
        world_items.anchored[item_id] = true; 
        item_id++;
    }

    // Oásis (Exotic) - Fonte de Estética (Joias)
    for(int i=0; i<config.initial_exotic; i++) {
        world_items.mass[item_id] = 100.0f;
        world_items.pos[item_id] = {pos_x(rng), pos_y(rng)};
        world_items.owner_id[item_id] = 0;
        world_items.integrity[item_id] = 100000.0f;
        world_items.matter[item_id] = {0.0f, 0.0f, 999.0f, 0.0f}; 
        world_items.anchored[item_id] = true; 
        item_id++;
    }

    // Spawna abundância de comida inicial pelo mapa
    for(int k=0; k<100; k++) {
        world_items.mass[item_id] = 1.0f;
        world_items.owner_id[item_id] = 0;
        world_items.pos[item_id] = {pos_x(rng), pos_y(rng)};
        world_items.matter[item_id] = {40.0f, 0, 0, 0};
        item_id++;
    }

    // Mercados não nascem mais com o mundo. Eles emergem!
    world_markets.pos.clear();
    world_markets.active_orders.clear();

    shared_state.spatial_grid.resize(GRID_WIDTH * GRID_HEIGHT, ' ');
    shared_state.spatial_health.resize(GRID_WIDTH * GRID_HEIGHT, 0);
    shared_state.energy_histogram.resize(10, 0);
}

void tick_world() {
    system_nature(world_agents, world_items);
    system_social_maintenance(world_agents, world_graph);
    system_memetics(world_agents, world_graph); // L7 Cultural
    system_market_emergence(world_markets, world_agents, world_graph); // L9 Mercados
    system_cognition(world_agents, world_items);
    system_reproduction(world_agents);
    system_crafting(world_agents, world_items);
    system_agriculture(world_agents, world_items);
    system_combat(world_agents, world_items);
    system_movement(world_agents, world_items, world_markets, world_graph);
    system_peer_to_peer_trade(world_agents, world_items, world_graph);
    system_market_submission(world_agents, world_markets, world_orders);
    system_barter_match(world_markets, world_orders, world_agents, world_items, world_graph);
    system_metabolism_and_consumption(world_agents, world_items);
    system_entropy(world_items, world_agents);
    system_trust_decay(world_graph);
    
    // Clearing House Garbage Collector (Spot Market)
    // Previne Memory Leak e Explosão O(N^2) no matching de ordens acumuladas
    world_orders.agent.clear();
    world_orders.offered_item.clear();
    world_orders.demanded_profile.clear();
    world_orders.ts.clear();
    world_orders.active.clear();
    for(auto& pool : world_markets.active_orders) pool.clear();

    current_tick++;
}

void extract_telemetry_to_state(UIState& state) {
    state.tick = current_tick;
    state.pop_alive = 0;
    state.total_trade_volume = 0.0f; // placeholder

    state.active_edges = 0;
    for(size_t i=0; i<world_graph.active.size(); i++) {
        if(world_graph.active[i]) state.active_edges++;
    }

    std::fill(state.spatial_grid.begin(), state.spatial_grid.end(), ' ');
    std::fill(state.spatial_health.begin(), state.spatial_health.end(), 0);
    std::fill(state.energy_histogram.begin(), state.energy_histogram.end(), 0);

    float sum_e = 0;
    for (size_t i = 1; i < world_agents.energy.size(); i++) {
        if (world_agents.energy[i] > 0) {
            state.pop_alive++;
            sum_e += world_agents.energy[i];
            
            int x = std::clamp((int)world_agents.pos[i].x, 0, GRID_WIDTH - 1);
            int y = std::clamp((int)world_agents.pos[i].y, 0, GRID_HEIGHT - 1);
            int idx = y * GRID_WIDTH + x;
            
            state.spatial_grid[idx] = '@';
            state.spatial_health[idx] = (world_agents.energy[i] < 30.0f) ? 1 : 2;
            
            int bin = std::clamp((int)(world_agents.energy[i] / 10.0f), 0, 9);
            state.energy_histogram[bin]++;
        }
    }
    if (state.pop_alive > 0) state.avg_energy = sum_e / state.pop_alive;

    for (size_t i = 1; i < world_items.mass.size(); i++) {
        if (world_items.mass[i] > 0) {
            int x = std::clamp((int)world_items.pos[i].x, 0, GRID_WIDTH - 1);
            int y = std::clamp((int)world_items.pos[i].y, 0, GRID_HEIGHT - 1);
            int idx = y * GRID_WIDTH + x;
            
            if (world_items.matter[i][MATTER_INDEX_CALORIES] >= 999.0f) {
                state.spatial_grid[idx] = 'T'; // Bosque
                state.spatial_health[idx] = 2; // Verde
            } else if (world_items.matter[i][MATTER_INDEX_HARDNESS] >= 999.0f) {
                state.spatial_grid[idx] = 'M'; // Jazida
                state.spatial_health[idx] = 5; // Cinza
            } else if (world_items.matter[i][MATTER_INDEX_INFORMATION] >= 999.0f) {
                state.spatial_grid[idx] = 'R'; // Ruínas
                state.spatial_health[idx] = 6; // Roxo/Ciano
            } else if (world_items.matter[i][MATTER_INDEX_AESTHETICS] >= 999.0f) {
                state.spatial_grid[idx] = 'O'; // Oásis
                state.spatial_health[idx] = 7; // Dourado
            } else if (world_items.matter[i][MATTER_INDEX_HARDNESS] >= 200.0f && world_items.anchored[i]) {
                state.spatial_grid[idx] = 'H'; // House/Farm
                state.spatial_health[idx] = 8; // Laranja
            } else if (world_items.owner_id[i] == 0) {
                if (world_items.matter[i][MATTER_INDEX_CALORIES] > 0) {
                    state.spatial_grid[idx] = '*'; // Comida
                    state.spatial_health[idx] = 4;
                } else if (world_items.matter[i][MATTER_INDEX_HARDNESS] > 0) {
                    state.spatial_grid[idx] = '.'; // Pedra
                    state.spatial_health[idx] = 5;
                } else if (world_items.matter[i][MATTER_INDEX_INFORMATION] > 0) {
                    state.spatial_grid[idx] = '?'; // Tábua
                    state.spatial_health[idx] = 6;
                } else if (world_items.matter[i][MATTER_INDEX_AESTHETICS] > 0) {
                    state.spatial_grid[idx] = 'J'; // Joia
                    state.spatial_health[idx] = 7;
                }
            }
        }
    }

    for (size_t m = 0; m < world_markets.pos.size(); m++) {
        int x = std::clamp((int)world_markets.pos[m].x, 0, GRID_WIDTH - 1);
        int y = std::clamp((int)world_markets.pos[m].y, 0, GRID_HEIGHT - 1);
        int idx = y * GRID_WIDTH + x;
        state.spatial_grid[idx] = '$';
        state.spatial_health[idx] = 3;
    }

    if (csv_file.is_open()) {
        float avg_lifespan = 0, avg_aggro = 0, avg_farm = 0;
        float avg_age = 0, avg_repro = 0, avg_metabolism = 0;
        int count = state.pop_alive > 0 ? state.pop_alive : 1;
        for(size_t i=1; i<world_agents.energy.size(); i++) {
            if(world_agents.energy[i] > 0) {
                avg_lifespan += world_agents.phenotype[i][GENE_LIFESPAN];
                avg_aggro += world_agents.phenotype[i][MEME_AGGRO];
                avg_farm += world_agents.phenotype[i][MEME_FARM];
                avg_age += world_agents.age[i];
                avg_repro += world_agents.phenotype[i][GENE_REPRO_TH];
                avg_metabolism += world_agents.phenotype[i][GENE_METABOLISM];
            }
        }
        avg_lifespan /= count; avg_aggro /= count; avg_farm /= count;
        avg_age /= count; avg_repro /= count; avg_metabolism /= count;
        
        int farms_count = 0;
        float avg_integrity = 0;
        int integrity_count = 0;
        for(size_t i=1; i<world_items.mass.size(); i++) {
            if(world_items.mass[i] > 0) {
                avg_integrity += world_items.integrity[i];
                integrity_count++;
                if(world_items.matter[i][MATTER_INDEX_HARDNESS] >= 200.0f && world_items.matter[i][MATTER_INDEX_HARDNESS] < 999.0f) {
                    farms_count++;
                }
            }
        }
        if (integrity_count > 0) avg_integrity /= integrity_count;
        
        csv_file << state.tick << "," << state.pop_alive << "," << state.avg_energy << ","
                 << avg_lifespan << "," << avg_aggro << "," << avg_farm << "," 
                 << state.active_edges << "," << farms_count << ","
                 << avg_age << "," << avg_repro << "," << avg_metabolism << "," << avg_integrity << "\n";
        csv_file.flush();
    }
}

void sim_thread(SimulationConfig config) {
    init_world(config);
    auto next_tick_time = std::chrono::steady_clock::now();

    while(simulation_running) {
        if (!simulation_paused) {
            tick_world();
            
            if (current_tick % 1 == 0) {
                std::lock_guard<std::mutex> lock(ui_mutex);
                extract_telemetry_to_state(shared_state);
            }
        }
        
        int tps = target_tps.load();
        if (tps > 0) {
            next_tick_time += std::chrono::milliseconds(1000 / tps);
            std::this_thread::sleep_until(next_tick_time);
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
            next_tick_time = std::chrono::steady_clock::now();
        }
    }
}
