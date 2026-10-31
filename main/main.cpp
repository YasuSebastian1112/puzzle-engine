#include "globals/rendering_global/window.h"
#include "globals/rendering_global/device/rendering_texture.h"
#include <iostream>

int main() {
    Window window;
    
    // Tipo Game: no fuerza el icono del editor
    window.initialize("Mi Super Juego 2D", Size2i(1280, 720), Point2i(100, 100), Window::Type::Editor);

    // El desarrollador define su propio icono (ya sea de archivo o de sus propios assets embebidos):
    //window.set_icon("22.png");

    window.run([&](double delta) {
        // Lógica del juego...
    });

    return 0;
}