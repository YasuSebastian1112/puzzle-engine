#include "globals/rendering_global/window.h"
#include "globals/rendering_global/rendering_global.h"

int main() {
    auto& rg = RenderingGlobal::get();
    rg.init();

    auto win_main = rg.create_window(Size2i(900, 700), "Campos 3D", Point2i(50, 50), Window::Type::Editor);
    auto win_table = rg.create_window(Size2i(800, 700), "Tabla Periodica", Point2i(970, 50), Window::Type::Editor);

    win_main->set_frame_callback([&](double delta) {
        // Dibujo exclusivo de la ventana 1
    });

    win_table->set_frame_callback([&](double delta) {
        // Dibujo exclusivo de la ventana 2
    });

    // Bucle global: maneja tiempo, eventos, ambas ventanas y cierre automático
    rg.run([&](double delta) {
        // Lógica global compartida (física, CLI, etc.)
    });

    return 0;
}