#include "systems.hpp"
#include <cmath>
#include <algorithm>
#include <random>
#include <execution>
#include <numeric>
#include "simulation.hpp"

static std::mt19937 global_rng;
static std::vector<int> spatial_head;
static std::vector<int> spatial_next;
static std::vector<int> spatial_item_head;
static std::vector<int> spatial_item_next;
static std::vector<EntityID> agent_home;

void seed_systems(int seed) {
    global_rng.seed(seed);
}

extern uint64_t current_tick;

void update_memory(Agents& A, size_t agent_idx, EntityID id, uint8_t type, Vec2 pos, uint32_t ts) {
    auto& mem = A.spatial_memory[agent_idx];
    int oldest_idx = 0;
    uint32_t oldest_ts = 0xFFFFFFFF;
    
    for(int i = 0; i < 10; i++) {
        if(mem[i].id == id && mem[i].type == type) {
            if(ts > mem[i].ts) { // Só atualiza se for uma informação mais recente
                mem[i].pos = pos;
                mem[i].ts = ts;
            }
            return;
        }
        if(mem[i].type == MEM_NONE) {
            mem[i] = {id, type, pos, ts};
            return;
        }
        if(mem[i].ts < oldest_ts) {
            oldest_ts = mem[i].ts;
            oldest_idx = i;
        }
    }
    
    // Sobrescreve a lembrança mais velha se a nova fofoca for mais recente do que ela
    if(ts > oldest_ts) {
        mem[oldest_idx] = {id, type, pos, ts};
    }
}

void gossip_sync(Agents& A, size_t a1, size_t a2) {
    for(int i = 0; i < 10; i++) {
        if(A.spatial_memory[a2][i].type != MEM_NONE) {
            update_memory(A, a1, A.spatial_memory[a2][i].id, A.spatial_memory[a2][i].type, A.spatial_memory[a2][i].pos, A.spatial_memory[a2][i].ts);
        }
        if(A.spatial_memory[a1][i].type != MEM_NONE) {
            update_memory(A, a2, A.spatial_memory[a1][i].id, A.spatial_memory[a1][i].type, A.spatial_memory[a1][i].pos, A.spatial_memory[a1][i].ts);
        }
    }
}


void build_spatial_grid(const Agents& A, const Items& I) {
    spatial_head.assign(GRID_WIDTH * GRID_HEIGHT, -1);
    if(spatial_next.size() < A.energy.size()) spatial_next.resize(A.energy.size(), -1);
    
    for (size_t i = 1; i < A.energy.size(); i++) {
        if (A.energy[i] > 0) {
            int x = std::clamp((int)A.pos[i].x, 0, GRID_WIDTH - 1);
            int y = std::clamp((int)A.pos[i].y, 0, GRID_HEIGHT - 1);
            int cell_idx = y * GRID_WIDTH + x;
            spatial_next[i] = spatial_head[cell_idx];
            spatial_head[cell_idx] = i;
        }
    }
    
    spatial_item_head.assign(GRID_WIDTH * GRID_HEIGHT, -1);
    if(spatial_item_next.size() < I.mass.size()) spatial_item_next.resize(I.mass.size(), -1);
    if(agent_home.size() < A.energy.size()) agent_home.resize(A.energy.size(), 0);
    std::fill(agent_home.begin(), agent_home.end(), 0);
    
    for (size_t k = 1; k < I.mass.size(); k++) {
        if (I.mass[k] > 0) {
            int x = std::clamp((int)I.pos[k].x, 0, GRID_WIDTH - 1);
            int y = std::clamp((int)I.pos[k].y, 0, GRID_HEIGHT - 1);
            int cell_idx = y * GRID_WIDTH + x;
            spatial_item_next[k] = spatial_item_head[cell_idx];
            spatial_item_head[cell_idx] = k;
            
            // Indexação O(1) de propriedades (Casas) para Faro
            if (I.anchored[k] && I.owner_id[k] != 0 && I.matter[k][MATTER_INDEX_HARDNESS] >= 100.0f) {
                if(I.owner_id[k] < agent_home.size()) agent_home[I.owner_id[k]] = k;
            }
        }
    }
}

std::array<float, M_MAT> calculate_deficit_vector(size_t agent_idx, const Agents& A, const Items& I) {
    std::array<float, M_MAT> d = {0};
    // L4: Valor Subjetivo Dinâmico - O déficit calórico emerge do desejo genético de reprodução
    float ideal_energy = A.phenotype[agent_idx][GENE_REPRO_TH] + 10.0f; // Margem de segurança
    float energy_deficit = ideal_energy - A.energy[agent_idx];
    if(energy_deficit > 0.0f) { 
        d[MATTER_INDEX_CALORIES] = energy_deficit;
    }
    
    // Soma vetorial do inventário
    float current_hardness = 0.0f;
    float current_aesthetics = 0.0f;
    float current_information = 0.0f;
    for(int j=0; j<INV_CAP; j++) {
        EntityID item = A.inventory[agent_idx][j];
        if(item != 0) {
            current_hardness += I.matter[item][MATTER_INDEX_HARDNESS];
            current_aesthetics += I.matter[item][MATTER_INDEX_AESTHETICS];
            current_information += I.matter[item][MATTER_INDEX_INFORMATION];
        }
    }
    
    // Desejo multidimensional: Otimiza os vetores carentes
    if(current_hardness < 50.0f) {
        d[MATTER_INDEX_HARDNESS] = 50.0f - current_hardness;
    }
    if(current_aesthetics < 50.0f) {
        d[MATTER_INDEX_AESTHETICS] = 50.0f - current_aesthetics;
    }
    if(current_information < 50.0f) {
        d[MATTER_INDEX_INFORMATION] = 50.0f - current_information;
    }
    
    return d;
}

float calculate_local_utility(size_t agent_idx, EntityID item_id, const Agents& A, const Items& I) {
    if(item_id >= I.matter.size()) return 0.0f;
    std::array<float, M_MAT> d = calculate_deficit_vector(agent_idx, A, I);
    
    // Utilidade é o Produto Escalar perfeito entre Demanda e Matéria
    float dot = 0.0f;
    for(int i = 0; i < M_MAT; i++) {
        dot += d[i] * I.matter[item_id][i];
    }
    return dot;
}

float vector_similarity(const std::array<float, M_MAT>& v1, const std::array<float, M_MAT>& v2) {
    float dot = 0.0f, n1 = 0.0f, n2 = 0.0f;
    for(int i = 0; i < M_MAT; i++) {
        dot += v1[i] * v2[i];
        n1 += v1[i] * v1[i];
        n2 += v2[i] * v2[i];
    }
    if(n1 == 0 || n2 == 0) return 0.0f;
    return dot / (std::sqrt(n1) * std::sqrt(n2));
}

void swap_items_in_inventory(Agents& A, EntityID agent1, EntityID item1, EntityID agent2, EntityID item2) {
    for(int i = 0; i < INV_CAP; i++) {
        if(A.inventory[agent1][i] == item1) A.inventory[agent1][i] = item2;
    }
    for(int i = 0; i < INV_CAP; i++) {
        if(A.inventory[agent2][i] == item2) A.inventory[agent2][i] = item1;
    }
}

void destroy_item(EntityID item, Items& I) {
    if(item != 0 && item < I.mass.size()) {
        if(I.mass[item] > 0.0f) {
            I.mass[item] = 0.0f;
            I.integrity[item] = 0.0f;
            I.owner_id[item] = 0;
            I.anchored[item] = false;
            for(int i = 0; i < M_MAT; i++) I.matter[item][i] = 0.0f;
            I.dead_ids.push_back(item);
        }
    }
}

void system_cognition(Agents& A, Items& I) {
    for(size_t i = 1; i < A.energy.size(); i++) {
        if(A.energy[i] <= 0) continue; 
        
        float repro_th = A.phenotype[i][GENE_REPRO_TH];
        float aggro_meme = A.phenotype[i][MEME_AGGRO];
        float farm_meme = A.phenotype[i][MEME_FARM];
        
        EntityID item_to_discard = 0;
        float worst_score = 999999.0f;
        int raw_materials = 0;
        int heavy_tools = 0;
        EntityID seed_item = 0;
        
        for(int j=0; j<INV_CAP; j++) {
            EntityID item = A.inventory[i][j];
            if(item == 0) continue;
            float v = calculate_local_utility(i, item, A, I);
            if(v < worst_score) { worst_score = v; item_to_discard = item; }
            
            float h = I.matter[item][MATTER_INDEX_HARDNESS];
            float c = I.matter[item][MATTER_INDEX_CALORIES];
            float info = I.matter[item][MATTER_INDEX_INFORMATION];
            
            if(h > 0 && h <= 50.0f) raw_materials++;
            if(h >= 60.0f && h < 190.0f) heavy_tools++; // Facilitado para 60 (1 fusão)
            if(c > 0) seed_item = item;
            if(info > 0) A.knowledge[i] += info * 0.01f; // Leitura passiva L3/L7
        }
        
        std::array<float, M_MAT> d = calculate_deficit_vector(i, A, I);
        float sum_d = 0; for(float v : d) sum_d += v;

        float panic_th = A.phenotype[i][GENE_PANIC_TH];
        float attack_th = A.phenotype[i][GENE_ATTACK_TH];
        float min_repro_age = A.phenotype[i][GENE_MIN_REPRO_AGE];
        float barter_desire = A.phenotype[i][GENE_BARTER_DESIRE];

        if (A.energy[i] < attack_th && aggro_meme > 0.8f) { // Mais desesperado e raro
            // Fome e Ódio: Procura vítima
            A.intent[i].type = Agents::Attack;
            A.intent[i].target_agent = 0;
            float min_dist = 9999.0f;
            
            int ax = std::clamp((int)A.pos[i].x, 0, GRID_WIDTH - 1);
            int ay = std::clamp((int)A.pos[i].y, 0, GRID_HEIGHT - 1);
            int search_radius = 5;
            
            for(int dy = -search_radius; dy <= search_radius; dy++) {
                for(int dx = -search_radius; dx <= search_radius; dx++) {
                    int nx = ax + dx, ny = ay + dy;
                    if(nx >= 0 && nx < GRID_WIDTH && ny >= 0 && ny < GRID_HEIGHT) {
                        int cell_idx = ny * GRID_WIDTH + nx;
                        for(int tgt = spatial_head[cell_idx]; tgt != -1; tgt = spatial_next[tgt]) {
                            if(tgt != i && A.energy[tgt] > 10.0f) {
                                float dist = std::abs(A.pos[i].x - A.pos[tgt].x) + std::abs(A.pos[i].y - A.pos[tgt].y);
                                if(dist < min_dist) { min_dist = dist; A.intent[i].target_agent = tgt; }
                            }
                        }
                    }
                }
            }
            if(A.intent[i].target_agent == 0) A.intent[i].type = Agents::Idle;
        } else if (A.energy[i] > 30.0f && farm_meme > 0.6f && heavy_tools > 0 && seed_item != 0) {
            A.intent[i].type = Agents::Farm;
            A.intent[i].offered_item = seed_item;
        } else if (A.energy[i] > repro_th && A.age[i] > min_repro_age) {
            A.intent[i].type = Agents::Reproduce;
        } else if (raw_materials >= 2 && A.energy[i] > 20.0f) { // Alterado para 20.0f (CRAFT para desocupar espaço)
            A.intent[i].type = Agents::CraftItem; 
        } else if (item_to_discard != 0 && A.energy[i] < panic_th) {
            bool has_food = false;
            int empty_slots = 0;
            for(int j=0; j<INV_CAP; j++) {
                if(A.inventory[i][j] != 0 && I.matter[A.inventory[i][j]][MATTER_INDEX_CALORIES] > 0) has_food = true;
                if(A.inventory[i][j] == 0) empty_slots++;
            }
            if(!has_food && empty_slots == 0) {
                for(int j=0; j<INV_CAP; j++) {
                    if(A.inventory[i][j] == item_to_discard) {
                        I.owner_id[item_to_discard] = 0;
                        I.pos[item_to_discard] = A.pos[i];
                        A.inventory[i][j] = 0;
                        break;
                    }
                }
            }
            A.intent[i].type = Agents::Idle; // Livre para usar o Faro
        } else if(item_to_discard != 0 && sum_d > barter_desire) {
            A.intent[i].type = Agents::PlaceBarterOrder;
            A.intent[i].offered_item = item_to_discard;
            A.intent[i].demanded_profile = d; 
        } else {
            A.intent[i].type = Agents::Idle;
        }
    }
}

void system_barter_match(Markets& M, Orders& O, Agents& A, Items& I, TrustGraph& G) {
    for(size_t m = 0; m < M.pos.size(); m++) {
        auto& pool = M.active_orders[m];
        
        // Limpeza O(N) para evitar OOM (Lei L10)
        std::vector<uint32_t> next_pool;
        for(size_t i = 0; i < pool.size(); i++) {
            if(O.active[pool[i]]) next_pool.push_back(pool[i]);
        }
        pool = next_pool;

        for(size_t i = 0; i < pool.size(); i++) {
            for(size_t j = i + 1; j < pool.size(); j++) {
                uint32_t o1 = pool[i]; uint32_t o2 = pool[j];
                if(!O.active[o1] || !O.active[o2]) continue;
                EntityID agent1 = O.agent[o1], agent2 = O.agent[o2];
                float match_1_wants_2 = vector_similarity(O.demanded_profile[o1], I.matter[O.offered_item[o2]]);
                float match_2_wants_1 = vector_similarity(O.demanded_profile[o2], I.matter[O.offered_item[o1]]);
                if(match_1_wants_2 > THRESHOLD && match_2_wants_1 > THRESHOLD) {
                    swap_items_in_inventory(A, agent1, O.offered_item[o1], agent2, O.offered_item[o2]);
                    O.active[o1] = false; O.active[o2] = false;
                    
                    // L6: Contratos Imperfeitos (Cria/Atualiza Confiança)
                    bool found = false;
                    for(size_t e = 0; e < G.active.size(); e++) {
                        if(G.active[e] && ((G.source[e] == agent1 && G.target[e] == agent2) || (G.source[e] == agent2 && G.target[e] == agent1))) {
                            G.trust[e] += 10.0f;
                            found = true; break;
                        }
                    }
                    if(!found) {
                        G.source.push_back(agent1);
                        G.target.push_back(agent2);
                        G.trust.push_back(10.0f);
                        G.active.push_back(true);
                    }
                    break;
                }
            }
        }
    }
}

void system_metabolism_and_consumption(Agents& A, Items& I) {
    for(size_t i = 1; i < A.energy.size(); i++) {
        if(A.energy[i] <= 0) continue; 
        
        bool was_alive = (A.energy[i] > 0);
        A.age[i]++;
        if(A.age[i] > A.lifespan[i]) {
            A.energy[i] = 0; // Entropia Genética L10
            if (was_alive) global_deaths_old_age++;
            was_alive = false;
        }
        
        if (A.energy[i] <= 0) {
            // Morreu! Lei Termodinâmica L1: Solta os itens de volta na natureza
            for(int j=0; j<INV_CAP; j++) {
                EntityID item = A.inventory[i][j];
                if(item != 0) {
                    I.owner_id[item] = 0;
                    I.pos[item] = A.pos[i];
                    A.inventory[i][j] = 0;
                }
            }
            A.dead_ids.push_back(i); // Adiciona ao pool de reciclagem
            continue;
        }

        // Checar se está abrigado (em cima de uma estrutura ancorada com Hardness >= 100)
        float current_basal = A.basal_cost[i];
        
        int ax = std::clamp((int)A.pos[i].x, 0, GRID_WIDTH - 1);
        int ay = std::clamp((int)A.pos[i].y, 0, GRID_HEIGHT - 1);
        for(int dy = -1; dy <= 1; dy++) {
            for(int dx = -1; dx <= 1; dx++) {
                int nx = ax + dx, ny = ay + dy;
                if(nx >= 0 && nx < GRID_WIDTH && ny >= 0 && ny < GRID_HEIGHT) {
                    int cell_idx = ny * GRID_WIDTH + nx;
                    for(int k = spatial_item_head[cell_idx]; k != -1; k = spatial_item_next[k]) {
                        if (I.anchored[k] && I.matter[k][MATTER_INDEX_HARDNESS] >= 100.0f) {
                            if (I.owner_id[k] == i || I.owner_id[k] == 0) {
                                if (std::abs(I.pos[k].x - A.pos[i].x) <= 1.0f && std::abs(I.pos[k].y - A.pos[i].y) <= 1.0f) {
                                    current_basal = 0.2f; 
                                    goto found_shelter;
                                }
                            }
                        }
                    }
                }
            }
        }
        found_shelter:
        
        A.energy[i] -= current_basal;
        if(A.energy[i] <= 0 && was_alive) global_deaths_starvation++;
        std::array<float, M_MAT> d_metabolism = calculate_deficit_vector(i, A, I);
        if(d_metabolism[MATTER_INDEX_CALORIES] > 5.0f) { // Dinâmico pelo fenótipo
            float best_util = 0.0f;
            EntityID best_food = 0;
            for(int j=0; j<INV_CAP; j++) {
                EntityID item = A.inventory[i][j];
                if(item != 0 && I.matter[item][MATTER_INDEX_CALORIES] > 0) {
                    float util = d_metabolism[MATTER_INDEX_CALORIES] * I.matter[item][MATTER_INDEX_CALORIES];
                    if(util > best_util) { best_util = util; best_food = item; }
                }
            }
            if(best_food != 0) {
                A.intent[i].type = Agents::ConsumeItem;
                A.intent[i].offered_item = best_food;
            }
        }
        if (A.intent[i].type == Agents::ConsumeItem) {
            EntityID target = A.intent[i].offered_item; 
            if (target != 0) {
                float extractable_energy = I.matter[target][MATTER_INDEX_CALORIES];
                A.energy[i] += extractable_energy * 0.8f; 
                destroy_item(target, I); 
                for(int j=0; j<INV_CAP; j++) if(A.inventory[i][j] == target) A.inventory[i][j] = 0;
            }
            A.intent[i].type = Agents::Idle; 
        }

        // Se permaneceu morto após a digestão, drop de itens e reciclagem de ID
        if (A.energy[i] <= 0) {
            for(int j=0; j<INV_CAP; j++) {
                EntityID item = A.inventory[i][j];
                if(item != 0) {
                    I.owner_id[item] = 0;
                    I.pos[item] = A.pos[i];
                    A.inventory[i][j] = 0;
                }
            }
            A.dead_ids.push_back(i);
        }
    }
}

void system_market_submission(Agents& A, Markets& M, Orders& O) {
    for (size_t i = 1; i < A.energy.size(); i++) {
        if (A.energy[i] <= 0) continue;
        
        if (A.intent[i].type == Agents::PlaceBarterOrder && !M.pos.empty()) {
            for(size_t m = 0; m < M.pos.size(); m++) {
                if(std::abs(A.pos[i].x - M.pos[m].x) <= 1.0f && std::abs(A.pos[i].y - M.pos[m].y) <= 1.0f) {
                    uint32_t order_idx = O.agent.size();
                    O.agent.push_back(i);
                    O.offered_item.push_back(A.intent[i].offered_item);
                    O.demanded_profile.push_back(A.intent[i].demanded_profile);
                    O.ts.push_back(0); 
                    O.active.push_back(true);
                    
                    M.active_orders[m].push_back(order_idx);
                    A.intent[i].type = Agents::Idle; 
                    break;
                }
            }
        }
    }
}

void system_nature(Agents& A, Items& I) {
    std::uniform_real_distribution<float> chance(0.0f, 1.0f);
    std::uniform_int_distribution<int> dir(-1, 1);
    
    // Iterar sobre todos os itens para achar Fontes
    for (size_t src = 1; src < I.mass.size(); src++) {
        if (I.mass[src] <= 0 || I.owner_id[src] != 0) continue;
        
        bool is_forest = (I.matter[src][MATTER_INDEX_CALORIES] >= 999.0f);
        bool is_vein = (I.matter[src][MATTER_INDEX_HARDNESS] >= 999.0f);
        bool is_farm = (I.matter[src][MATTER_INDEX_HARDNESS] >= 200.0f && I.matter[src][MATTER_INDEX_HARDNESS] < 300.0f);
        bool is_ruin = (I.matter[src][MATTER_INDEX_INFORMATION] >= 999.0f);
        bool is_exotic = (I.matter[src][MATTER_INDEX_AESTHETICS] >= 999.0f);
        
        if (!is_forest && !is_vein && !is_farm && !is_ruin && !is_exotic) continue;

        // Probabilidades de Spawn
        float spawn_chance = 0.0f;
        if (is_forest) spawn_chance = 0.20f; 
        else if (is_vein) spawn_chance = 0.02f; 
        else if (is_farm) spawn_chance = 0.8f; 
        else if (is_ruin) spawn_chance = 0.01f; // Ruínas dão info muito raro
        else if (is_exotic) spawn_chance = 0.01f; // Oásis dão joias muito raro

        if (chance(global_rng) < spawn_chance) {
            // Acha espaço vazio via dead_ids pool
            size_t j = 0;
            if (!I.dead_ids.empty()) {
                j = I.dead_ids.back();
                I.dead_ids.pop_back();
            } else {
                // Crescimento dinâmico elástico do SoA de itens
                j = I.mass.size();
                I.matter.push_back({});
                I.integrity.push_back(100.0f);
                I.mass.push_back(0.0f);
                I.owner_id.push_back(0);
                I.pos.push_back({0.0f, 0.0f});
                I.anchored.push_back(false);
            }

            if (j != 0) {
                I.pos[j].x = std::clamp(I.pos[src].x + dir(global_rng), 0.0f, (float)GRID_WIDTH - 1);
                I.pos[j].y = std::clamp(I.pos[src].y + dir(global_rng), 0.0f, (float)GRID_HEIGHT - 1);
                I.mass[j] = 1.0f;
                I.integrity[j] = 100.0f;
                I.owner_id[j] = 0;
                I.anchored[j] = false;
                
                if (is_vein) {
                    I.matter[j] = {0.0f, 20.0f, 0.0f, 0.0f}; // Pedra
                } else if (is_ruin) {
                    I.matter[j] = {0.0f, 0.0f, 0.0f, 30.0f}; // Tábua
                } else if (is_exotic) {
                    I.matter[j] = {0.0f, 0.0f, 40.0f, 0.0f}; // Joia
                } else {
                    I.matter[j] = {40.0f, 0.0f, 0.0f, 0.0f}; // Maçã
                }
            }
        }
    }
}

void system_movement(Agents& A, Items& I, const Markets& M, const TrustGraph& G) {
    std::uniform_int_distribution<int> dir(-1, 1);

    for (size_t i = 1; i < A.energy.size(); i++) {
        if (A.energy[i] <= 0) continue;

        // Vantagem Tecnológica e Epistemológica (Calculada antes do movimento)
        float tool_efficiency = 1.0f;
        float tool_information = 0.0f;
        for(int j=0; j<INV_CAP; j++) {
            EntityID item = A.inventory[i][j];
            if(item != 0) {
                tool_efficiency += I.matter[item][MATTER_INDEX_HARDNESS];
                tool_information += I.matter[item][MATTER_INDEX_INFORMATION];
            }
        }
        
        // Raio de Visão expandido pelo Conhecimento (Redução de Fog of War L2)
        float base_radar = 5.0f;
        float expanded_radar = base_radar + (tool_information * 0.1f);

        float dx = 0.0f;
        float dy = 0.0f;
        
        if (A.intent[i].type == Agents::Attack && A.intent[i].target_agent != 0) {
            EntityID tgt = A.intent[i].target_agent;
            if(A.energy[tgt] > 0) {
                if (A.pos[i].x < A.pos[tgt].x) dx = 1.0f;
                else if (A.pos[i].x > A.pos[tgt].x) dx = -1.0f;
                if (A.pos[i].y < A.pos[tgt].y) dy = 1.0f;
                else if (A.pos[i].y > A.pos[tgt].y) dy = -1.0f;
            }
        } else if (A.intent[i].type == Agents::PlaceBarterOrder) {
            // L8: Inteligência P2P - Busca primeiro por amigos na rede antes de ir ao mercado
            bool going_to_friend = false;
            float best_trust = 0.0f;
            Vec2 target = {0,0};
            
            // Busca L2: Apenas memória espacial local (Sem Onisciência Global)
            for(size_t e = 0; e < G.active.size(); e++) {
                if(!G.active[e]) continue;
                if(G.source[e] == i || G.target[e] == i) {
                    EntityID friend_id = (G.source[e] == i) ? G.target[e] : G.source[e];
                    if(G.trust[e] > best_trust && G.trust[e] > 5.0f) {
                        // Consulta a memória em vez do oráculo global
                        for(int m = 0; m < 10; m++) {
                            if(A.spatial_memory[i][m].type == MEM_AGENT && A.spatial_memory[i][m].id == friend_id) {
                                best_trust = G.trust[e];
                                target = A.spatial_memory[i][m].pos;
                                going_to_friend = true;
                                break;
                            }
                        }
                    }
                }
            }
            
            if(going_to_friend) {
                if (A.pos[i].x < target.x) dx = 1.0f;
                else if (A.pos[i].x > target.x) dx = -1.0f;
                if (A.pos[i].y < target.y) dy = 1.0f;
                else if (A.pos[i].y > target.y) dy = -1.0f;
            } else {
                // Consulta memória para buscar um Mercado
                float min_dist = 999999.0f;
                bool found_market = false;
                Vec2 m_target = {0,0};
                for(int m = 0; m < 10; m++) {
                    if(A.spatial_memory[i][m].type == MEM_MARKET) {
                        Vec2 pos = A.spatial_memory[i][m].pos;
                        float dist = std::abs(A.pos[i].x - pos.x) + std::abs(A.pos[i].y - pos.y);
                        if(dist < min_dist) { min_dist = dist; m_target = pos; found_market = true; }
                    }
                }
                if(found_market) {
                    if (A.pos[i].x < m_target.x) dx = 1.0f;
                    else if (A.pos[i].x > m_target.x) dx = -1.0f;
                    if (A.pos[i].y < m_target.y) dy = 1.0f;
                    else if (A.pos[i].y > m_target.y) dy = -1.0f;
                } else {
                    // Sem memória, anda aleatoriamente (Exploração)
                    dx = dir(global_rng);
                    dy = dir(global_rng);
                }
            }
        } else {
            // Faro Local Universal (L5 + L4: Gradiente descendente guiado por Produto Escalar)
            bool found_target = false;
            float best_utility = 0.0f;
            Vec2 best_target = {0,0};
            
            std::array<float, M_MAT> d = calculate_deficit_vector(i, A, I);
            float sum_d = 0; for(float v : d) sum_d += v;
            
            if (sum_d > 10.0f) { // Só fareja se tiver carência vetorial
                int s_rad = std::ceil(expanded_radar);
                int ax = std::clamp((int)A.pos[i].x, 0, GRID_WIDTH - 1);
                int ay = std::clamp((int)A.pos[i].y, 0, GRID_HEIGHT - 1);
                for(int dy = -s_rad; dy <= s_rad; dy++) {
                    for(int dx = -s_rad; dx <= s_rad; dx++) {
                        int nx = ax + dx, ny = ay + dy;
                        if(nx >= 0 && nx < GRID_WIDTH && ny >= 0 && ny < GRID_HEIGHT) {
                            int cell_idx = ny * GRID_WIDTH + nx;
                            for(int k = spatial_item_head[cell_idx]; k != -1; k = spatial_item_next[k]) {
                                if (I.owner_id[k] == 0 && !I.anchored[k]) {
                                    float utility = 0.0f;
                                    for(int m = 0; m < M_MAT; m++) utility += d[m] * I.matter[k][m];
                                    
                                    if (utility > 50.0f) { // Percebe valor real na matéria
                                        float dist = std::abs(I.pos[k].x - A.pos[i].x) + std::abs(I.pos[k].y - A.pos[i].y);
                                        float score = utility / (dist + 1.0f); // Desconto hiperbólico pelo custo de transporte L5
                                        if (dist <= expanded_radar && score > best_utility) {
                                            best_utility = score;
                                            best_target = I.pos[k];
                                            found_target = true;
                                        }
                                    }
                                }
                                
                                // Gravação de Memória (Percepção Geográfica)
                                if (I.anchored[k] && I.matter[k][MATTER_INDEX_HARDNESS] >= 100.0f) {
                                    update_memory(A, i, k, MEM_RESOURCE, I.pos[k], current_tick);
                                }
                            }
                            
                            // Percepção de Agentes e Gossip Passivo
                            for(int tgt = spatial_head[cell_idx]; tgt != -1; tgt = spatial_next[tgt]) {
                                if(tgt != i) {
                                    update_memory(A, i, tgt, MEM_AGENT, A.pos[tgt], current_tick);
                                    if(current_tick % 5 == 0) gossip_sync(A, i, tgt); // Troca informações casualmente (fofoca)
                                }
                            }
                        }
                    }
                }
            }
            
            // Percepção de Mercados (Emergentes) no Radar
            for(size_t m = 0; m < M.pos.size(); m++) {
                float dist = std::abs(M.pos[m].x - A.pos[i].x) + std::abs(M.pos[m].y - A.pos[i].y);
                if(dist <= expanded_radar) {
                    update_memory(A, i, m, MEM_MARKET, M.pos[m], current_tick);
                }
            }
            
            if(found_target) {
                if (A.pos[i].x < best_target.x) dx = 1.0f;
                else if (A.pos[i].x > best_target.x) dx = -1.0f;
                if (A.pos[i].y < best_target.y) dy = 1.0f;
                else if (A.pos[i].y > best_target.y) dy = -1.0f;
            } else {
                // Faro Local: Voltar para Casa (Propriedade Privada)
                bool going_home = false;
                Vec2 home_pos = {0,0};
                
                EntityID home_id = 0;
                if(i < agent_home.size()) {
                    home_id = agent_home[i];
                }
                if(home_id != 0 && I.mass[home_id] > 0) {
                    home_pos = I.pos[home_id];
                    going_home = true;
                }
                
                if(going_home) {
                    if (A.pos[i].x < home_pos.x) dx = 1.0f;
                    else if (A.pos[i].x > home_pos.x) dx = -1.0f;
                    if (A.pos[i].y < home_pos.y) dy = 1.0f;
                    else if (A.pos[i].y > home_pos.y) dy = -1.0f;
                } else {
                    dx = dir(global_rng);
                    dy = dir(global_rng);
                }
            }
        }

        if (dx != 0 || dy != 0) {
            float total_mass = 1.0f; // Massa do próprio corpo
            for(int j=0; j<INV_CAP; j++) {
                if(A.inventory[i][j] != 0) total_mass += I.mass[A.inventory[i][j]];
            }
            // L5: Custo reduzido pela raiz da Dureza da Ferramenta!
            float move_cost = (total_mass * 0.05f) / std::sqrt(tool_efficiency); 
            
            if (A.energy[i] > move_cost) {
                A.pos[i].x = std::clamp(A.pos[i].x + dx, 0.0f, (float)GRID_WIDTH - 1);
                A.pos[i].y = std::clamp(A.pos[i].y + dy, 0.0f, (float)GRID_HEIGHT - 1);
                A.energy[i] -= move_cost;
            }
        }

        // Foraging: Se passar por cima de comida no chão e tiver espaço, coleta.
        int ax = std::clamp((int)A.pos[i].x, 0, GRID_WIDTH - 1);
        int ay = std::clamp((int)A.pos[i].y, 0, GRID_HEIGHT - 1);
        for(int dy = -1; dy <= 1; dy++) {
            for(int dx = -1; dx <= 1; dx++) {
                int nx = ax + dx, ny = ay + dy;
                if(nx >= 0 && nx < GRID_WIDTH && ny >= 0 && ny < GRID_HEIGHT) {
                    int cell_idx = ny * GRID_WIDTH + nx;
                    for(int k = spatial_item_head[cell_idx]; k != -1; k = spatial_item_next[k]) {
                        if (I.matter[k][MATTER_INDEX_HARDNESS] < 200.0f) {
                            if (I.anchored[k] && I.owner_id[k] != 0 && I.owner_id[k] != i) continue;
                            if (!I.anchored[k] && I.owner_id[k] != 0) continue; 
                            
                            if (std::abs(I.pos[k].x - A.pos[i].x) <= 1.0f && std::abs(I.pos[k].y - A.pos[i].y) <= 1.0f) {
                                for (int j = 0; j < INV_CAP; j++) {
                                    if (A.inventory[i][j] == 0) {
                                        A.inventory[i][j] = k;
                                        I.owner_id[k] = i; 
                                        goto next_forage;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        next_forage:;
    }
}

void system_reproduction(Agents& A) {
    std::normal_distribution<float> mutation(0.0f, 15.0f); // Mutação Memética L7
    std::uniform_int_distribution<int> dir(-1, 1);
    std::normal_distribution<float> mut(0.0f, 1.0f);

    size_t initial_size = A.energy.size();
    for (size_t i = 1; i < initial_size; i++) {
        if (A.energy[i] <= 0) continue;
        
        float repro_th = A.phenotype[i][GENE_REPRO_TH];
        float min_repro_age = A.phenotype[i][GENE_MIN_REPRO_AGE];

        if (A.intent[i].type == Agents::Reproduce && A.energy[i] > repro_th && A.age[i] > min_repro_age) {
            // Mitose Perfeita Termodinâmica (Energia se divide L1)
            A.energy[i] /= 2.0f;
            A.intent[i].type = Agents::Idle;

            // Busca por reuso de memória via OOM Prevention L10 - O(1) com pool de reciclagem
            size_t child_idx = 0;
            if(!A.dead_ids.empty()) {
                child_idx = A.dead_ids.back();
                A.dead_ids.pop_back();
            }

            std::array<EpistemicNode, 10> empty_mem;
            for(int k=0; k<10; k++) empty_mem[k] = {0, MEM_NONE, {0,0}, 0};

            if (child_idx == 0) {
                // Crescimento real da população além do recorde prévio
                child_idx = A.energy.size();
                A.energy.push_back(0);
                A.basal_cost.push_back(0);
                A.pos.push_back({0,0});
                A.phenotype.push_back({0});
                A.knowledge.push_back(0);
                A.spatial_memory.push_back(empty_mem);
                std::array<EntityID, INV_CAP> empty_inv = {0};
                A.inventory.push_back(empty_inv);
                A.intent.push_back({Agents::Idle, {0,0}, 0, {0}});
                A.age.push_back(0);
                A.lifespan.push_back(0);
            }
            A.spatial_memory[child_idx] = empty_mem; // Tabula rasa da memória espacial

            // Crescimento da População
            A.energy[child_idx] = A.energy[i]; 
            A.pos[child_idx] = {std::clamp(A.pos[i].x + dir(global_rng), 0.0f, (float)GRID_WIDTH - 1),
                                std::clamp(A.pos[i].y + dir(global_rng), 0.0f, (float)GRID_HEIGHT - 1)};
            
            // Genética e Memética Completa
            float mut_life = std::normal_distribution<float>(0.0f, 15.0f)(global_rng);
            float mut_repro = std::normal_distribution<float>(0.0f, 5.0f)(global_rng);
            float mut_meta = std::normal_distribution<float>(0.0f, 0.1f)(global_rng);

            A.phenotype[child_idx] = A.phenotype[i]; // Herda tudo (Genes e Memes copiados do pai)
            
            // Muta apenas os Genes Biológicos
            A.phenotype[child_idx][GENE_LIFESPAN] = std::clamp(A.phenotype[i][GENE_LIFESPAN] + mut_life, 100.0f, 2000.0f);
            A.phenotype[child_idx][GENE_REPRO_TH] = std::clamp(A.phenotype[i][GENE_REPRO_TH] + mut_repro, 40.0f, 150.0f);
            A.phenotype[child_idx][GENE_METABOLISM] = std::clamp(A.phenotype[i][GENE_METABOLISM] + mut_meta, 0.5f, 5.0f);

            A.phenotype[child_idx][GENE_PANIC_TH] = std::clamp(A.phenotype[i][GENE_PANIC_TH] + std::normal_distribution<float>(0.0f, 5.0f)(global_rng), 5.0f, 90.0f);
            A.phenotype[child_idx][GENE_MIN_REPRO_AGE] = std::clamp(A.phenotype[i][GENE_MIN_REPRO_AGE] + std::normal_distribution<float>(0.0f, 2.0f)(global_rng), 0.0f, 100.0f);
            A.phenotype[child_idx][GENE_ATTACK_TH] = std::clamp(A.phenotype[i][GENE_ATTACK_TH] + mut(global_rng), 0.0f, 100.0f);
            A.phenotype[child_idx][GENE_BARTER_DESIRE] = std::clamp(A.phenotype[i][GENE_BARTER_DESIRE] + mut(global_rng), 0.0f, 100.0f);
            A.phenotype[child_idx][GENE_INTELLIGENCE] = std::clamp(A.phenotype[i][GENE_INTELLIGENCE] + (mut(global_rng)/100.0f), 0.0f, 1.0f);
            A.phenotype[child_idx][GENE_INNOVATION] = std::clamp(A.phenotype[i][GENE_INNOVATION] + (mut(global_rng)/100.0f), 0.0f, 1.0f);
            A.phenotype[child_idx][GENE_ARTISTRY] = std::clamp(A.phenotype[i][GENE_ARTISTRY] + (mut(global_rng)/100.0f), 0.0f, 1.0f);
            A.knowledge[child_idx] = 0.0f; // Tabula Rasa


            A.lifespan[child_idx] = (int)A.phenotype[child_idx][GENE_LIFESPAN];
            A.basal_cost[child_idx] = A.phenotype[child_idx][GENE_METABOLISM];
            A.age[child_idx] = 0;
            
            for(int k=0; k<INV_CAP; k++) A.inventory[child_idx][k] = 0;
            A.intent[child_idx].type = Agents::Idle;
            A.intent[child_idx].target_agent = 0;
        }
    }
}


void system_social_maintenance(Agents& A, TrustGraph& G) {
    for(size_t i = 0; i < G.active.size(); i++) {
        if(!G.active[i]) continue;
        EntityID a1 = G.source[i];
        EntityID a2 = G.target[i];
        
        // Se um dos dois morreu ou se odeiam, a aresta quebra
        if(A.energy[a1] <= 0 || A.energy[a2] <= 0 || G.trust[i] <= 0.0f) {
            G.active[i] = false;
            continue;
        }
        
        // Manter laços custa calorias (L8 - Fricção Social)
        A.energy[a1] -= 0.01f;
        A.energy[a2] -= 0.01f;
    }
}

void system_peer_to_peer_trade(Agents& A, Items& I, TrustGraph& G) {
    for(size_t i = 1; i < A.energy.size(); i++) {
        if(A.energy[i] <= 0 || A.intent[i].type != Agents::PlaceBarterOrder) continue;
        
        int x = std::clamp((int)A.pos[i].x, 0, GRID_WIDTH - 1);
        int y = std::clamp((int)A.pos[i].y, 0, GRID_HEIGHT - 1);
        
        bool swapped = false;
        for(int dy = -1; dy <= 1 && !swapped; dy++) {
            for(int dx = -1; dx <= 1 && !swapped; dx++) {
                int nx = x + dx, ny = y + dy;
                if(nx >= 0 && nx < GRID_WIDTH && ny >= 0 && ny < GRID_HEIGHT) {
                    int cell_idx = ny * GRID_WIDTH + nx;
                    for(int j = spatial_head[cell_idx]; j != -1; j = spatial_next[j]) {
                        if(j > i && A.intent[j].type == Agents::PlaceBarterOrder) { // j > i evita trocas espelhadas
                            float match_1_wants_2 = vector_similarity(A.intent[i].demanded_profile, I.matter[A.intent[j].offered_item]);
                            float match_2_wants_1 = vector_similarity(A.intent[j].demanded_profile, I.matter[A.intent[i].offered_item]);
                            
                            // Busca se já existe trust
                            float existing_trust = 0.0f;
                            size_t edge_idx = -1;
                            for(size_t e = 0; e < G.active.size(); e++) {
                                if(G.active[e] && ((G.source[e] == i && G.target[e] == j) || (G.source[e] == j && G.target[e] == i))) {
                                    existing_trust = G.trust[e];
                                    edge_idx = e;
                                    break;
                                }
                            }
                            
                            // Calcula o bônus de Estética (Aesthetics deslumbra e facilita acordos)
                            float tool_aesthetics_i = 0.0f;
                            for(int k=0; k<INV_CAP; k++) {
                                EntityID item = A.inventory[i][k];
                                if(item != 0) tool_aesthetics_i += I.matter[item][MATTER_INDEX_AESTHETICS];
                            }
                            
                            float tool_aesthetics_j = 0.0f;
                            for(int k=0; k<INV_CAP; k++) {
                                EntityID item = A.inventory[j][k];
                                if(item != 0) tool_aesthetics_j += I.matter[item][MATTER_INDEX_AESTHETICS];
                            }
                            
                            float max_aesthetics = std::max(tool_aesthetics_i, tool_aesthetics_j);
                            
                            // Relaxa o limite com base na Confiança e na Estética
                            float relaxed_threshold = THRESHOLD - (existing_trust / 1000.0f) - (max_aesthetics * 0.005f);
                            if(relaxed_threshold < 0.1f) relaxed_threshold = 0.1f;
                            
                            if(match_1_wants_2 > relaxed_threshold && match_2_wants_1 > relaxed_threshold) {
                                swap_items_in_inventory(A, i, A.intent[i].offered_item, j, A.intent[j].offered_item);
                                A.intent[i].type = Agents::Idle;
                                A.intent[j].type = Agents::Idle;
                                
                                if(edge_idx != -1) {
                                    G.trust[edge_idx] += 10.0f;
                                } else {
                                    G.source.push_back(i);
                                    G.target.push_back(j);
                                    G.trust.push_back(10.0f);
                                    G.active.push_back(true);
                                }
                                swapped = true;
                                break; 
                            }
                        }
                    }
                }
            }
        }
    }
}

void system_memetics(Agents& A, const TrustGraph& G) {
    for(size_t i=0; i<G.active.size(); i++) {
        if(!G.active[i]) continue;
        EntityID a1 = G.source[i];
        EntityID a2 = G.target[i];
        if(A.energy[a1] <= 0 || A.energy[a2] <= 0) continue;
        
        // Osmose Memética L7 (Copia quem tem mais de 50 de energia a mais)
        if(A.energy[a1] > A.energy[a2] + 50.0f) {
            A.phenotype[a2][MEME_TRADE_TH] = A.phenotype[a1][MEME_TRADE_TH];
            A.phenotype[a2][MEME_FARM] = A.phenotype[a1][MEME_FARM];
            A.phenotype[a2][MEME_AGGRO] = A.phenotype[a1][MEME_AGGRO];
        } else if(A.energy[a2] > A.energy[a1] + 50.0f) {
            A.phenotype[a1][MEME_TRADE_TH] = A.phenotype[a2][MEME_TRADE_TH];
            A.phenotype[a1][MEME_FARM] = A.phenotype[a2][MEME_FARM];
            A.phenotype[a1][MEME_AGGRO] = A.phenotype[a2][MEME_AGGRO];
        }
    }
}

void system_agriculture(Agents& A, Items& I) {
    for(size_t i = 1; i < A.energy.size(); i++) {
        if(A.energy[i] <= 0) continue;
        if(A.intent[i].type == Agents::Farm) {
            EntityID seed = A.intent[i].offered_item;
            if(seed != 0) {
                // Remove a semente do inventário
                for(int j=0; j<INV_CAP; j++) {
                    if (A.inventory[i][j] == seed) A.inventory[i][j] = 0;
                }
                I.owner_id[seed] = i; 
                I.anchored[seed] = true;
                I.pos[seed] = A.pos[i];
                I.matter[seed][MATTER_INDEX_HARDNESS] = 200.0f; // Vira estrutura agrícola
                I.matter[seed][MATTER_INDEX_CALORIES] = 0.0f; // Semente enterrada não pode ser comida
                A.energy[i] -= 20.0f; // Custo do trabalho
            }
            A.intent[i].type = Agents::Idle;
        }
    }
}

void system_crafting(Agents& A, Items& I) {
    for(size_t i = 1; i < A.energy.size(); i++) {
        if(A.energy[i] <= 0) continue;
        if(A.intent[i].type == Agents::CraftItem) {
            EntityID item1 = 0;
            EntityID item2 = 0;
            int idx1 = -1, idx2 = -1;
            
            // Pega os dois itens menos valiosos
            float v1 = 999999.0f, v2 = 999999.0f;
            for(int j=0; j<INV_CAP; j++) {
                EntityID it = A.inventory[i][j];
                if(it == 0) continue;
                float v = calculate_local_utility(i, it, A, I);
                if(v < v1) { v2 = v1; item2 = item1; idx2 = idx1; v1 = v; item1 = it; idx1 = j; }
                else if(v < v2) { v2 = v; item2 = it; idx2 = j; }
            }
            
            if(item1 != 0 && item2 != 0) {
                float m_in = I.matter[item1][MATTER_INDEX_HARDNESS] + I.matter[item2][MATTER_INDEX_HARDNESS];
                float k = A.knowledge[i];
                float efficiency = k / (k + 100.0f);
                
                float h_out = m_in * efficiency;
                float m_wasted = m_in - h_out;
                
                float info_yield = m_wasted * A.phenotype[i][GENE_INNOVATION];
                float aest_yield = m_wasted * A.phenotype[i][GENE_ARTISTRY];
                
                // Transmuta o item1 para o resultado final
                I.matter[item1][MATTER_INDEX_HARDNESS] = h_out;
                I.matter[item1][MATTER_INDEX_INFORMATION] += info_yield;
                I.matter[item1][MATTER_INDEX_AESTHETICS] += aest_yield;
                I.matter[item1][MATTER_INDEX_CALORIES] = 0.0f;
                I.integrity[item1] = 100.0f; // Restaura integridade
                
                // Destrói o item2
                destroy_item(item2, I);
                A.inventory[i][idx2] = 0;
                
                // Ganho empírico
                A.knowledge[i] += A.phenotype[i][GENE_INTELLIGENCE] * (m_wasted + 1.0f);
                A.energy[i] -= 5.0f; // Custo energético do crafting
            }
            A.intent[i].type = Agents::Idle;
        }
    }
}

void system_combat(Agents& A, Items& I) {
    for (size_t i = 1; i < A.energy.size(); i++) {
        if (A.energy[i] <= 0 || A.intent[i].type != Agents::Attack) continue;
        
        EntityID target = A.intent[i].target_agent;
        if(target == 0 || A.energy[target] <= 0) {
            A.intent[i].type = Agents::Idle;
            continue;
        }
        
        if(std::abs(A.pos[i].x - A.pos[target].x) <= 1.0f && std::abs(A.pos[i].y - A.pos[target].y) <= 1.0f) {
            float my_str = A.energy[i];
            float their_str = A.energy[target];
            for(int j=0; j<INV_CAP; j++) {
                if(A.inventory[i][j]) my_str += I.matter[A.inventory[i][j]][MATTER_INDEX_HARDNESS];
                if(A.inventory[target][j]) their_str += I.matter[A.inventory[target][j]][MATTER_INDEX_HARDNESS];
            }
            
            A.energy[i] -= 5.0f; // Custo do soco (Fricção da guerra)
            A.energy[target] -= 5.0f; 
            
            if(my_str > their_str) {
                float steal = std::min(40.0f, A.energy[target]);
                A.energy[i] += steal; 
                A.energy[target] -= steal; // Assalto não letal automático (Hobbesian Trap fix)
                if (A.energy[target] <= 0) {
                    global_deaths_combat++;
                    for(int j=0; j<INV_CAP; j++) {
                        EntityID item = A.inventory[target][j];
                        if(item != 0) {
                            I.owner_id[item] = 0;
                            I.pos[item] = A.pos[target];
                            A.inventory[target][j] = 0;
                        }
                    }
                    A.dead_ids.push_back(target);
                }
            } else {
                float steal = std::min(40.0f, A.energy[i]);
                A.energy[target] += steal;
                A.energy[i] -= steal; 
                if (A.energy[i] <= 0) {
                    global_deaths_combat++;
                    for(int j=0; j<INV_CAP; j++) {
                        EntityID item = A.inventory[i][j];
                        if(item != 0) {
                            I.owner_id[item] = 0;
                            I.pos[item] = A.pos[i];
                            A.inventory[i][j] = 0;
                        }
                    }
                    A.dead_ids.push_back(i);
                }
            }
            A.intent[i].type = Agents::Idle;
        }
    }
}

void system_entropy(Items& I, Agents& A) {
    for (size_t i = 1; i < I.mass.size(); i++) {
        if (I.mass[i] > 0) {
            bool is_primordial = (I.matter[i][MATTER_INDEX_CALORIES] >= 999.0f || I.matter[i][MATTER_INDEX_HARDNESS] >= 999.0f);
            if (is_primordial) continue; // Proteção para Bosques e Jazidas
            
            I.integrity[i] -= 0.05f; // L10: Ferramentas e Fazendas sofrem desgaste
            if (I.integrity[i] <= 0) {
                // Destruição térmica
                if(I.owner_id[i] != 0) {
                    EntityID owner = I.owner_id[i];
                    for(int j=0; j<INV_CAP; j++) {
                        if(A.inventory[owner][j] == i) {
                            A.inventory[owner][j] = 0;
                        }
                    }
                }
                destroy_item(i, I);
            }
        }
    }
}

void system_trust_decay(TrustGraph& G) {
    for(size_t e = 0; e < G.active.size(); e++) {
        if(G.active[e]) {
            G.trust[e] -= 0.05f; // L8: Relacionamentos morrem de fome se não mantidos
            if(G.trust[e] <= 0) {
                G.active[e] = false;
            }
        }
    }
}

void system_market_emergence(Markets& M, const Agents& A, const TrustGraph& G) {
    // 1. Limpa os mercados do tick anterior
    M.pos.clear();
    for(auto& pool : M.active_orders) pool.clear();
    M.active_orders.clear();

    // 2. Criação do Heatmap Comercial (resolução 10x10 blocos)
    const int cell_size = 10;
    int cols = GRID_WIDTH / cell_size;
    int rows = GRID_HEIGHT / cell_size;
    std::vector<float> heatmap(cols * rows, 0.0f);

    // 3. Irradiação de Calor Institucional via Grafo de Confiança
    for(size_t e = 0; e < G.active.size(); e++) {
        if(G.active[e] && G.trust[e] > 0.5f) { // Alta confiança gera calor
            EntityID src = G.source[e];
            EntityID tgt = G.target[e];
            
            // Adiciona calor na posição do agente source
            int cx = std::clamp((int)(A.pos[src].x / cell_size), 0, cols - 1);
            int cy = std::clamp((int)(A.pos[src].y / cell_size), 0, rows - 1);
            heatmap[cy * cols + cx] += G.trust[e];

            // Adiciona calor na posição do agente target
            cx = std::clamp((int)(A.pos[tgt].x / cell_size), 0, cols - 1);
            cy = std::clamp((int)(A.pos[tgt].y / cell_size), 0, rows - 1);
            heatmap[cy * cols + cx] += G.trust[e];
        }
    }

    // 4. Materialização do Mercado (Limiar de Calor)
    float HEAT_THRESHOLD = 5.0f; // Precisa de pelo menos 5 interações/confiança fortes na mesma célula
    for(int y = 0; y < rows; y++) {
        for(int x = 0; x < cols; x++) {
            if(heatmap[y * cols + x] > HEAT_THRESHOLD) {
                // Instancia o Mercado no centro geográfico da célula de calor
                Vec2 m_pos = { (x * cell_size) + (cell_size / 2.0f), (y * cell_size) + (cell_size / 2.0f) };
                M.pos.push_back(m_pos);
                M.active_orders.push_back({});
            }
        }
    }
}
