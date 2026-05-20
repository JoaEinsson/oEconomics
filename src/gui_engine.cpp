#include "gui_engine.hpp"
#include "simulation.hpp"
#include "telemetry.hpp"
#include <raylib.h>
#include <raymath.h>
#include <mutex>
#include <string>
#include <thread>

// Utilitários de Interface Imediata (Custom GUI)
bool DrawCustomButton(Rectangle bounds, const char* text) {
    bool clicked = false;
    Vector2 mouse = GetMousePosition();
    bool hover = CheckCollisionPointRec(mouse, bounds);
    
    if (hover && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        clicked = true;
    }
    
    DrawRectangleRec(bounds, hover ? LIGHTGRAY : GRAY);
    DrawRectangleLinesEx(bounds, 2, DARKGRAY);
    
    int tw = MeasureText(text, 20);
    DrawText(text, bounds.x + bounds.width/2 - tw/2, bounds.y + bounds.height/2 - 10, 20, hover ? BLACK : RAYWHITE);
    return clicked;
}

bool DrawCustomCheckbox(Rectangle bounds, const char* text, bool checked) {
    Vector2 mouse = GetMousePosition();
    bool hover = CheckCollisionPointRec(mouse, bounds);
    
    if (hover && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        checked = !checked;
    }
    
    DrawRectangleRec(bounds, checked ? MAROON : DARKGRAY);
    DrawRectangleLinesEx(bounds, 2, BLACK);
    
    DrawText(text, bounds.x + bounds.width + 10, bounds.y + bounds.height/2 - 10, 20, DARKGRAY);
    return checked;
}

bool DrawCustomTextBoxInt(Rectangle bounds, const char* label, int* value, bool* active, bool disabled) {
    Vector2 mouse = GetMousePosition();
    bool hover = CheckCollisionPointRec(mouse, bounds);
    
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        *active = hover && !disabled;
    }
    
    if (*active && !disabled) {
        int charPressed = GetCharPressed();
        while (charPressed > 0) {
            if (charPressed >= '0' && charPressed <= '9') {
                *value = (*value * 10) + (charPressed - '0');
                if (*value > 99999999) *value = 99999999;
            }
            charPressed = GetCharPressed();
        }
        
        if (IsKeyPressed(KEY_BACKSPACE)) {
            *value /= 10;
        }
    }
    
    DrawRectangleRec(bounds, disabled ? GRAY : (*active ? LIGHTGRAY : RAYWHITE));
    DrawRectangleLinesEx(bounds, 2, (*active && !disabled) ? MAROON : BLACK);
    
    std::string textStr = std::to_string(*value);
    if (*active && !disabled) textStr += "_";
    
    DrawText(textStr.c_str(), bounds.x + 10, bounds.y + bounds.height/2 - 10, 20, BLACK);
    DrawText(label, bounds.x - MeasureText(label, 20) - 10, bounds.y + bounds.height/2 - 10, 20, DARKGRAY);
    return *active;
}

int DrawCustomSlider(Rectangle bounds, const char* label, int value, int min, int max) {
    Vector2 mouse = GetMousePosition();
    bool hover = CheckCollisionPointRec(mouse, bounds);
    
    if (hover && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        float pct = (mouse.x - bounds.x) / bounds.width;
        value = min + (int)(pct * (max - min));
        if (value < min) value = min;
        if (value > max) value = max;
    }
    
    DrawRectangleRec(bounds, DARKGRAY);
    float fillPct = (float)(value - min) / (max - min);
    DrawRectangle(bounds.x, bounds.y, bounds.width * fillPct, bounds.height, MAROON);
    DrawRectangleLinesEx(bounds, 2, BLACK);
    
    std::string text = std::string(label) + ": " + std::to_string(value);
    DrawText(text.c_str(), bounds.x + 10, bounds.y + bounds.height/2 - 10, 20, RAYWHITE);
    return value;
}

void ui_thread() {
    const int screenWidth = 1400;
    const int screenHeight = 900;

    InitWindow(screenWidth, screenHeight, "OECONOMICS - Engine Visualization");
    SetTargetFPS(60);

    enum class GUIState { MENU, SIMULATION };
    GUIState current_state = GUIState::MENU;

    SimulationConfig config;
    config.seed = 42;
    config.initial_pop = 300;
    config.initial_forests = 30;
    config.initial_veins = 15;
    config.initial_ruins = 15;
    config.initial_exotic = 15;
    
    int local_tps = 10;
    std::thread* t_sim = nullptr;
    
    bool seed_random = true;
    bool seed_active = false;

    Camera2D camera = { 0 };
    camera.target = Vector2{ (float)GRID_WIDTH * 4.0f / 2.0f, (float)GRID_HEIGHT * 4.0f / 2.0f };
    camera.offset = Vector2{ screenWidth / 2.0f, screenHeight / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 2.0f;

    UIState local_state;

    while (!WindowShouldClose()) {
        if (!simulation_running) break;
        
        if (current_state == GUIState::MENU) {
            BeginDrawing();
            ClearBackground(RAYWHITE);
            
            DrawText("OECONOMICS", screenWidth/2 - MeasureText("OECONOMICS", 60)/2, 100, 60, MAROON);
            DrawText("Organic Socioeconomic Simulation Engine", screenWidth/2 - MeasureText("Organic Socioeconomic Simulation Engine", 20)/2, 170, 20, DARKGRAY);
            
            int sx = screenWidth/2 - 200;
            
            // Controle da Semente
            seed_random = DrawCustomCheckbox({(float)sx, 220, 20, 20}, "Seed Aleatória", seed_random);
            DrawCustomTextBoxInt({(float)sx + 100, 250, 300, 40}, "Seed", &config.seed, &seed_active, seed_random);
            
            // Sliders Restantes
            config.initial_pop = DrawCustomSlider({(float)sx, 310, 400, 40}, "Initial Population", config.initial_pop, 2, 5000);
            config.initial_forests = DrawCustomSlider({(float)sx, 370, 400, 40}, "Forests (Calories)", config.initial_forests, 0, 100);
            config.initial_veins = DrawCustomSlider({(float)sx, 430, 400, 40}, "Ore Veins (Hardness)", config.initial_veins, 0, 100);
            config.initial_ruins = DrawCustomSlider({(float)sx, 490, 400, 40}, "Ruins (Information)", config.initial_ruins, 0, 100);
            config.initial_exotic = DrawCustomSlider({(float)sx, 550, 400, 40}, "Oasis (Aesthetics)", config.initial_exotic, 0, 100);
            
            local_tps = DrawCustomSlider({(float)sx, 630, 400, 40}, "Target TPS", local_tps, 1, 60);

            if (DrawCustomButton({(float)sx, 710, 400, 50}, "START SIMULATION")) {
                target_tps = local_tps;
                if (seed_random) config.seed = (int)(GetTime() * 1000.0) % 999999;
                t_sim = new std::thread(sim_thread, config);
                current_state = GUIState::SIMULATION;
            }
            
            EndDrawing();
            continue;
        }

        // Atualizar estado da simulação
        {
            std::lock_guard<std::mutex> lock(ui_mutex);
            local_state = shared_state;
        }

        // --- Controles da Câmera ---
        // Pan (Drag & Drop)
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            Vector2 delta = GetMouseDelta();
            delta = Vector2Scale(delta, -1.0f / camera.zoom);
            camera.target.x += delta.x;
            camera.target.y += delta.y;
        }
        
        // Pan (Setas)
        if (IsKeyDown(KEY_RIGHT)) camera.target.x += 10.0f / camera.zoom;
        if (IsKeyDown(KEY_LEFT)) camera.target.x -= 10.0f / camera.zoom;
        if (IsKeyDown(KEY_DOWN)) camera.target.y += 10.0f / camera.zoom;
        if (IsKeyDown(KEY_UP)) camera.target.y -= 10.0f / camera.zoom;

        // Zoom (Wheel)
        float wheel = GetMouseWheelMove();
        if (wheel != 0) {
            Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);
            camera.offset = GetMousePosition();
            camera.target = mouseWorldPos;
            camera.zoom += wheel * 0.125f;
            if (camera.zoom < 0.125f) camera.zoom = 0.125f;
        }
        
        // Zoom (PgUp / PgDn)
        if (IsKeyPressed(KEY_PAGE_UP)) {
            Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);
            camera.offset = GetMousePosition();
            camera.target = mouseWorldPos;
            camera.zoom += 0.5f;
        }
        if (IsKeyPressed(KEY_PAGE_DOWN)) {
            Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);
            camera.offset = GetMousePosition();
            camera.target = mouseWorldPos;
            camera.zoom -= 0.5f;
            if(camera.zoom < 0.125f) camera.zoom = 0.125f;
        }

        // --- Renderização ---
        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode2D(camera);

        // Desenhar Mundo
        float cellSize = 8.0f;
        if (local_state.spatial_grid.size() > 0) {
            for (int y = 0; y < GRID_HEIGHT; y++) {
                for (int x = 0; x < GRID_WIDTH; x++) {
                    int idx = y * GRID_WIDTH + x;
                    char c = local_state.spatial_grid[idx];
                    int h = local_state.spatial_health[idx];
                    
                    Color cellColor = BLANK;
                    if (c == 'T') cellColor = DARKGREEN; // Floresta
                    else if (c == 'M') cellColor = DARKGRAY; // Jazida
                    else if (c == 'R') cellColor = PURPLE; // Ruínas
                    else if (c == 'O') cellColor = ORANGE; // Oásis
                    else if (c == 'H') cellColor = MAROON; // Fazenda/Casa
                    else if (c == '*') cellColor = LIME; // Comida (Drop)
                    else if (c == '.') cellColor = GRAY; // Pedra (Drop)
                    else if (c == '?') cellColor = MAGENTA; // Tábua (Drop)
                    else if (c == 'J') cellColor = GOLD; // Joia (Drop)
                    else if (c == '$') cellColor = { 255, 200, 0, 150 }; // Mercado Emergente (Brilho translúcido)
                    
                    if (c == '@') {
                        // Agente
                        Color agentColor = BLUE;
                        if (h == 1) agentColor = RED; // Com fome
                        DrawCircle(x * cellSize + cellSize/2, y * cellSize + cellSize/2, cellSize/2 + 0.5f, agentColor);
                    } else if (cellColor.a != 0) {
                        if(c == '$') {
                            DrawCircle(x * cellSize + cellSize/2, y * cellSize + cellSize/2, cellSize * 2, cellColor);
                        } else {
                            DrawRectangle(x * cellSize, y * cellSize, cellSize, cellSize, cellColor);
                            DrawRectangleLines(x * cellSize, y * cellSize, cellSize, cellSize, Fade(BLACK, 0.2f));
                        }
                    }
                }
            }
        }
        
        // Bordas do Mundo
        DrawRectangleLines(0, 0, GRID_WIDTH * cellSize, GRID_HEIGHT * cellSize, RED);

        EndMode2D();

        // --- Telemetria Overlay ---
        DrawRectangle(10, 10, 300, 270, Fade(BLACK, 0.85f));
        DrawText("OECONOMICS - FASE 10", 20, 20, 20, WHITE);
        
        if (local_state.spatial_grid.size() > 0) {
            DrawText(TextFormat("TICK: %d", local_state.tick), 20, 50, 20, GREEN);
            DrawText(TextFormat("POP ALIVE: %d", local_state.pop_alive), 20, 75, 20, RAYWHITE);
            DrawText(TextFormat("AVG ENERGY: %.1f", local_state.avg_energy), 20, 100, 20, RAYWHITE);
            DrawText(TextFormat("TRUST EDGES: %d", local_state.active_edges), 20, 125, 20, GOLD);
            // Bens Construídos
            DrawText(TextFormat("FARMS: %d", local_state.farms_count), 20, 155, 20, ORANGE);
            DrawText(TextFormat("TOOLS: %d  (Crafted: %d)", local_state.active_tools, local_state.tools_crafted), 20, 180, 20, SKYBLUE);
        } else {
            DrawText("Carregando...", 20, 50, 20, GRAY);
        }
        
        DrawText("ZOOM: Wheel | PgUp/PgDn", 20, 210, 10, LIGHTGRAY);
        DrawText("PAN: Drag L-Click | Arrows", 20, 225, 10, LIGHTGRAY);
        DrawText("[P] Pause  [ESC] Sair", 20, 240, 10, LIGHTGRAY);
        
        // Controle de TPS In-Game
        DrawRectangle(10, 290, 300, 60, Fade(BLACK, 0.85f));
        int current_tps_val = target_tps.load();
        int new_tps = DrawCustomSlider({20, 300, 280, 40}, "TPS", current_tps_val, 1, 60);
        if (new_tps != current_tps_val) {
            target_tps.store(new_tps);
        }

        EndDrawing();
    }

    CloseWindow();
    simulation_running = false;
    if (t_sim) {
        t_sim->join();
        delete t_sim;
    }
}
