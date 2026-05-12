#include "gui_engine.hpp"
#include "simulation.hpp"
#include "telemetry.hpp"
#include <raylib.h>
#include <raymath.h>
#include <mutex>
#include <string>

void ui_thread() {
    const int screenWidth = 1400;
    const int screenHeight = 900;

    InitWindow(screenWidth, screenHeight, "OECONOMICS - Raylib GUI");
    SetTargetFPS(60);

    Camera2D camera = { 0 };
    camera.target = Vector2{ (float)GRID_WIDTH * 4.0f / 2.0f, (float)GRID_HEIGHT * 4.0f / 2.0f };
    camera.offset = Vector2{ screenWidth / 2.0f, screenHeight / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 2.0f;

    UIState local_state;

    while (!WindowShouldClose()) {
        if (!simulation_running) break;

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
        DrawRectangle(10, 10, 300, 180, Fade(BLACK, 0.85f));
        DrawText("OECONOMICS - FASE 10", 20, 20, 20, WHITE);
        
        if (local_state.spatial_grid.size() > 0) {
            DrawText(TextFormat("TICK: %d", local_state.tick), 20, 50, 20, GREEN);
            DrawText(TextFormat("POP ALIVE: %d", local_state.pop_alive), 20, 75, 20, RAYWHITE);
            DrawText(TextFormat("AVG ENERGY: %.1f", local_state.avg_energy), 20, 100, 20, RAYWHITE);
            DrawText(TextFormat("TRUST EDGES: %d", local_state.active_edges), 20, 125, 20, GOLD);
        } else {
            DrawText("Carregando...", 20, 50, 20, GRAY);
        }
        
        DrawText("ZOOM: Wheel | PgUp/PgDn", 20, 155, 10, LIGHTGRAY);
        DrawText("PAN: Drag L-Click | Arrows", 20, 170, 10, LIGHTGRAY);

        EndDrawing();
    }

    CloseWindow();
    simulation_running = false;
}
