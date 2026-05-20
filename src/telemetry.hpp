#pragma once
#include <vector>
#include <mutex>
#include <cstdint>
#include "ecs_types.hpp"

struct UIState {
    uint64_t tick;
    uint32_t pop_alive;
    float avg_energy;
    float total_trade_volume;
    uint32_t active_edges;
    uint32_t active_tools;
    uint32_t tools_crafted;
    uint32_t farms_count;
    
    // Grid geográfico achatado
    std::vector<char> spatial_grid; // Tamanho: WIDTH * HEIGHT
    std::vector<int> spatial_health; // 0=morto, 1=crítico, 2=saudável, 3=mercado
    
    // Dados para os Histogramas (Distribuição de Renda/Energia)
    std::vector<int> energy_histogram; // Bins de 0 a E_MAX
};

extern std::mutex ui_mutex;
extern UIState shared_state;
