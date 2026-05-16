#include "gui_engine.hpp"

int main() {
    // O controle central de parâmetros, loop e threads agora reside inteiramente na Interface
    ui_thread(); 
    
    return 0;
}
