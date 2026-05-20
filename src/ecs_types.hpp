#pragma once
#include <vector>
#include <array>
#include <cstdint>

using EntityID = uint32_t;
using MarketID = uint32_t;

struct Vec2 { 
    float x, y; 
};

constexpr int N_TRAITS = 13;
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
constexpr int GENE_INTELLIGENCE = 10;
constexpr int GENE_INNOVATION = 11;
constexpr int GENE_ARTISTRY = 12;

// Tipos de Conhecimento Espacial
enum EpistemicType {
    MEM_NONE = 0,
    MEM_AGENT = 1,
    MEM_MARKET = 2,
    MEM_RESOURCE = 3
};

struct EpistemicNode {
    EntityID id;
    uint8_t type;
    Vec2 pos;
    uint32_t ts; // Timestamp de quando a informação foi gerada
};

// L1-L10 AXIOMS: Estritamente PODs e Vetores (Zero OOP)

struct Agents {
    std::vector<std::array<EpistemicNode, 10>> spatial_memory; // L2: Fog of War
    std::vector<float> energy;
    std::vector<float> basal_cost;
    std::vector<Vec2>  pos;
    std::vector<std::array<float, N_TRAITS>> phenotype;
    std::vector<std::array<EntityID, INV_CAP>> inventory; 
    std::vector<int> age;
    std::vector<int> lifespan;

    std::vector<float> knowledge; // Conhecimento empírico acumulado [0, Infinito)

    enum IntentionType { Idle, MoveTo, PlaceBarterOrder, ConsumeItem, Reproduce, CraftItem, Farm, Attack };
    struct Intention {
        IntentionType type;
        Vec2 target_pos;
        EntityID offered_item;
        std::array<float, M_MAT> demanded_profile; // Vetor d_i
        EntityID target_agent; // Para o sistema de Guerra
    };
    std::vector<Intention> intent;
    std::vector<EntityID> dead_ids; // Recycled agent slots
};

struct Items {
    std::vector<std::array<float, M_MAT>> matter; // Propriedades físicas reais
    std::vector<float> integrity;                 // Entropia
    std::vector<float> mass;                      // Custo de transporte
    std::vector<EntityID> owner_id;               // 0 = sem dono (chão)
    std::vector<Vec2> pos;                        // Importante para fricção espacial
    std::vector<bool> anchored;                   // Propriedade Privada
    std::vector<EntityID> dead_ids;               // Recycled item slots
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
