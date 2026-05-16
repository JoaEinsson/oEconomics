#pragma once
#include "ecs_types.hpp"

// Protótipos dos sistemas baseados nas Leis L1-L10

float calculate_local_utility(size_t agent_idx, EntityID item_id, const Agents& A, const Items& I);
std::array<float, M_MAT> calculate_deficit_vector(size_t agent_idx, const Agents& A);
float vector_similarity(const std::array<float, M_MAT>& v1, const std::array<float, M_MAT>& v2);
void swap_items_in_inventory(Agents& A, EntityID agent1, EntityID item1, EntityID agent2, EntityID item2);
void destroy_item(EntityID item, Agents& A, Items& I);

std::array<float, M_MAT> calculate_deficit_vector(size_t agent_idx, const Agents& A, const Items& I);

void build_spatial_grid(const Agents& A, const Items& I);
void system_nature(Agents& A, Items& I);
void system_reproduction(Agents& A);
void system_memetics(Agents& A, const TrustGraph& G);
void system_agriculture(Agents& A, Items& I);
void system_combat(Agents& A, Items& I);
void system_crafting(Agents& A, Items& I);
void system_social_maintenance(Agents& A, TrustGraph& G);
void system_peer_to_peer_trade(Agents& A, Items& I, TrustGraph& G);
void system_movement(Agents& A, Items& I, const Markets& M, const TrustGraph& G);
void system_market_submission(Agents& A, Markets& M, Orders& O);
void system_cognition(Agents& A, Items& I);
void system_market_emergence(Markets& M, const Agents& A, const TrustGraph& G);
void system_barter_match(Markets& M, Orders& O, Agents& A, Items& I, TrustGraph& G);
void seed_systems(int seed);
void system_metabolism_and_consumption(Agents& A, Items& I);
void system_entropy(Items& I, Agents& A);
void system_trust_decay(TrustGraph& G);
