[ESPECIFICAÇÃO DE INTERFACE / TUI - LER ANTES DE GERAR]Construa a Interface de Usuário no Terminal (TUI) para o motor ECS socioeconômico.HARD PROHIBITIONS (Rupturas Fatais):— NÃO acople a renderização ao loop de simulação. A simulação (tick_world) deve rodar em unlocked framerate (ou em uma thread dedicada). A UI deve rodar a 60 FPS fixos, lendo um estado compactado.— NÃO use std::cout ou printf soltos com system("clear"). Isso causa flickering (piscadas) severo. Utilize uma biblioteca robusta de TUI (Ex: FTXUI se C++, Ratatui se Rust) ou implemente Double Buffering com ANSI escape codes estruturados.— NÃO realize cálculos pesados ou iterações $O(N)$ na thread de renderização. O system_telemetry do ECS é quem mastiga os dados. A UI apenas desenha.ARQUITETURA DE DADOS DA INTERFACE (Double Buffering):Implemente um mecanismo de extração de estado Thread-Safe (ex: Mutex ou Atomic Swap).C++// 1. O PAYLOAD DE TELEMETRIA (Extraído pelo ECS a cada N ticks)
struct UIState {
    uint64_t tick;
    uint32_t pop_alive;
    float avg_energy;
    float total_trade_volume;
    
    // Grid geográfico achatado
    std::vector<char> spatial_grid; // Tamanho: WIDTH * HEIGHT
    
    // Dados para os Histogramas (Distribuição de Renda/Energia)
    std::vector<int> energy_histogram; // Bins de 0 a E_MAX
};

std::mutex ui_mutex;
UIState shared_state; // Acessado por ambas as threads
MÓDULOS DE RENDERIZAÇÃO (Quadrantes da TUI):[VIEW: MACRO-GEOGRAFIA]Desenhar spatial_grid. Entidades: Agente @, Mercado $, Cadáver/Lixo *.Aterramento [A]: Aplique mapa de cores simples baseado em saúde. Agentes com energy < 20% renderizam em vermelho. Instituições em amarelo.[VIEW: HISTOGRAMA DE CLASSES (Distribuição de Valor)]Uma representação gráfica simples usando caracteres de bloco (ex:  , ▂, ▅, █).Adversarial [B]: O eixo X é a energia acumulada, o eixo Y é a contagem de agentes. Isso permite visualizar em tempo real a transição de uma distribuição normal (fase 0) para uma Lei de Potência / Pareto (fase 3 - acúmulo de riqueza no mercado).[CONTROLES DE EXECUÇÃO]Input não-bloqueante na thread da UI.[Space]: Pausa a thread do ECS (útil para auditoria).[ + / - ]: Altera a taxa de Ticks per Second (TPS) alvo.[ TAB ]: Alterna a visualização entre o Mapa Geográfico e os Painéis de Gráficos/Economia.ESTRUTURA DO LOOP ASSÍNCRONO:C++// THREAD 1: MOTOR (Corre o mais rápido possível ou em TPS fixo)
void sim_thread() {
    while(running) {
        tick_world(...);
        if (current_tick % RENDER_RATE == 0) {
            std::lock_guard<std::mutex> lock(ui_mutex);
            extract_telemetry_to_state(shared_state); // ECS mastiga e entrega pronto
        }
    }
}

// THREAD 2: INTERFACE (Corre a 60 FPS fixos)
void ui_thread() {
    while(running) {
        UIState local_state;
        {
            std::lock_guard<std::mutex> lock(ui_mutex);
            local_state = shared_state; // Cópia rápida
        }
        render_tui_layout(local_state); // Desenha sem travar a simulação
        process_user_input(); // Teclado não-bloqueante
        sleep_for(16ms); // Mantém ~60 FPS
    }
}