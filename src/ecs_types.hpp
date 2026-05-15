#pragma once
#include <vector>
#include <array>
#include <cstdint>

using EntityID = uint32_t;
using MarketID = uint32_t;

struct Vec2 { 
    float x, y; 
};

constexpr int N_TRAITS = 12;
constexpr int M_MAT    = 8;
constexpr int INV_CAP  = 8;

constexpr float THRESHOLD = 0.5f;
constexpr int MATTER_INDEX_CALORIES = 0;
constexpr int MATTER_INDEX_HARDNESS = 1;
constexpr int MATTER_INDEX_AESTHETICS = 2;
constexpr int MATTER_INDEX_INFORMATION = 3;
constexpr int GRID_WIDTH = 200;
constexpr int GRID_HEIGHT = 100;

// Genoma e Memética
constexpr int GENE_LIFESPAN = 0;
constexpr int GENE_REPRO_TH = 1;
constexpr int GENE_METABOLISM = 2;
constexpr int MEME_TRADE_TH = 3;
constexpr int MEME_FARM = 4;
constexpr int MEME_AGGRO = 5;
constexpr int GENE_PANIC_TH = 6;
constexpr int GENE_MIN_REPRO_AGE = 7;
constexpr int GENE_ATTACK_TH = 8;
constexpr int GENE_BARTER_DESIRE = 9;

// L1-L10 AXIOMS: Estritamente PODs e Vetores (Zero OOP)

struct Agents {
    std::vector<float> energy;
    std::vector<float> basal_cost;
    std::vector<Vec2>  pos;
    std::vector<std::array<float, N_TRAITS>> phenotype;
    std::vector<std::array<EntityID, INV_CAP>> inventory; 
    std::vector<int> age;
    std::vector<int> lifespan;

    enum IntentionType { Idle, MoveTo, PlaceBarterOrder, ConsumeItem, Reproduce, CraftItem, Farm, Attack };
    struct Intention {
        IntentionType type;
        Vec2 target_pos;
        EntityID offered_item;
        std::array<float, M_MAT> demanded_profile; // Vetor d_i
        EntityID target_agent; // Para o sistema de Guerra
    };
    std::vector<Intention> intent;
};

struct Items {
    std::vector<std::array<float, M_MAT>> matter; // Propriedades físicas reais
    std::vector<float> integrity;                 // Entropia
    std::vector<float> mass;                      // Custo de transporte
    std::vector<EntityID> owner_id;               // 0 = sem dono (chão)
    std::vector<Vec2> pos;                        // Importante para fricção espacial
    std::vector<bool> anchored;                   // Propriedade Privada
};

struct Orders {
    std::vector<EntityID> agent;
    std::vector<EntityID> offered_item;
    std::vector<std::array<float, M_MAT>> demanded_profile;
    std::vector<uint64_t> ts;
    std::vector<bool> active;
};

struct Markets {
    std::vector<Vec2> pos;
    std::vector<std::vector<uint32_t>> active_orders; 
};

struct TrustGraph {
    std::vector<EntityID> source;
    std::vector<EntityID> target;
    std::vector<float> trust;
    std::vector<bool> active;
};
