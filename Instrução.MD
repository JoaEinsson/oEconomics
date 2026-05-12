[CONTEXTO DO SISTEMA - LER ANTES DE GERAR]
Você está desenvolvendo o núcleo de um motor de simulação socioeconômica orgânica baseado em ECS (Entity Component System) com arquitetura SoA (Structure of Arrays) estrita.
Regras Absolutas de Geração:

ZERO Orientação a Objetos. Sem herança, sem polimorfismo, sem virtual.

A Lei Termodinâmica é absoluta: a energia não pode ser criada em trocas de mercado, apenas consumida via desgaste ou absorvida ao destruir itens orgânicos.

Não crie variáveis de "dinheiro" ou "saldo fiat". O mercado é um Clearing House de escambo multidimensional.

Implemente as seguintes estruturas em C++ (ou Rust equivalente):

C++
using EntityID = uint32_t;
using MarketID = uint32_t;
struct Vec2 { float x, y; };

constexpr int N_TRAITS = 8;
constexpr int M_MAT    = 8;
constexpr int INV_CAP  = 8;

// 1. ONTOLOGIA BASE (SoA)
struct Agents {
  std::vector<float> energy;
  std::vector<float> basal_cost;
  std::vector<Vec2>  pos;
  std::vector<std::array<float, N_TRAITS>> phenotype;
  std::vector<std::array<EntityID, INV_CAP>> inventory; 

  enum IntentionType { Idle, MoveTo, PlaceBarterOrder, ConsumeItem };
  struct Intention {
    IntentionType type;
    Vec2 target_pos;
    EntityID offered_item;
    std::array<float, M_MAT> demanded_profile; // O vetor d_i
  };
  std::vector<Intention> intent;
};

struct Items {
  std::vector<std::array<float, M_MAT>> matter; // Propriedades físicas reais
  std::vector<float> integrity;                 // Entropia
  std::vector<float> mass;                      // Custo de transporte
};

// 2. O MERCADO PRIMORDIAL (Escambo Algorítmico)
struct Orders {
  std::vector<EntityID> agent;
  std::vector<EntityID> offered_item;
  std::vector<std::array<float, M_MAT>> demanded_profile;
  std::vector<uint64_t> ts;
  std::vector<bool> active;
};

struct Markets {
  std::vector<Vec2> pos;
  std::vector<std::vector<uint32_t>> active_orders; // Índices na struct Orders
};

// 3. SISTEMAS DE RESOLUÇÃO FÍSICA E ECONÔMICA

// [A] Cognição: Oferece o que menos precisa, pede o que mais falta
void system_cognition(Agents& A, Items& I) {
  for(size_t i = 0; i < A.energy.size(); i++) {
    if(A.energy[i] <= 0) continue; // Morto

    float worst_score = 1e9f;
    EntityID item_to_discard = 0;
    int inv_slot = -1;

    // Encontra o item de MENOR utilidade no inventário
    for(int j = 0; j < INV_CAP; j++) {
      EntityID item = A.inventory[i][j];
      if(item == 0) continue;
      float v = calculate_local_utility(i, item, A, I);
      if(v < worst_score) {
        worst_score = v;
        item_to_discard = item;
        inv_slot = j;
      }
    }

    if(item_to_discard != 0) {
      A.intent[i].type = Agents::PlaceBarterOrder;
      A.intent[i].offered_item = item_to_discard;
      A.intent[i].demanded_profile = calculate_deficit_vector(i, A); // Gera vetor ideal
    }
  }
}

// [B] A Troca Física (Clearing House)
void system_barter_match(Markets& M, Orders& O, Agents& A, Items& I) {
  for(size_t m = 0; m < M.pos.size(); m++) {
    auto& pool = M.active_orders[m];
    
    // Busca O(N^2) local amortizada (só roda nas ordens ativas daquele mercado)
    for(size_t i = 0; i < pool.size(); i++) {
      for(size_t j = i + 1; j < pool.size(); j++) {
        uint32_t o1 = pool[i];
        uint32_t o2 = pool[j];

        if(!O.active[o1] || !O.active[o2]) continue;

        EntityID agent1 = O.agent[o1];
        EntityID agent2 = O.agent[o2];

        // Match: O item oferecido atende a demanda cruzada? (Produto escalar)
        float match_1_wants_2 = vector_similarity(O.demanded_profile[o1], I.matter[O.offered_item[o2]]);
        float match_2_wants_1 = vector_similarity(O.demanded_profile[o2], I.matter[O.offered_item[o1]]);

        if(match_1_wants_2 > THRESHOLD && match_2_wants_1 > THRESHOLD) {
          // RESOLUÇÃO FÍSICA ESTRITA: SWAP DE IDs NO INVENTÁRIO
          swap_items_in_inventory(A, agent1, O.offered_item[o1], agent2, O.offered_item[o2]);
          
          O.active[o1] = false;
          O.active[o2] = false;
          break; // O1 resolvido, pula para o próximo
        }
      }
    }
  }
}

// [C] Consumo Termodinâmico (A única forma de gerar energia)
void system_metabolism_and_consumption(Agents& A, Items& I) {
  for(size_t i = 0; i < A.energy.size(); i++) {
    A.energy[i] -= A.basal_cost[i];
    
    if (A.intent[i].type == Agents::ConsumeItem) {
        EntityID target = A.intent[i].offered_item; // Reusando o campo
        float extractable_energy = I.matter[target][MATTER_INDEX_CALORIES]; // Ex: Açúcar/Carvão
        A.energy[i] += extractable_energy * 0.8f; // 20% perda entrópica
        destroy_item(target, A, I); // Remove do inventário e libera ID
    }
  }
}