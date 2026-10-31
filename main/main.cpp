#include "globals/rendering_global/window.h"
#include "classes/resources/texture_2d.h"
#include <iostream>

int main() {
    Window window;

    if (!window.initialize("Puzzle Engine - Mario Pattern Test", Size2i(1024, 640), Point2i(100, 100), Window::Type::Editor)) {
        std::cerr << "[PuzzleEngine] Error al inicializar Window." << std::endl;
        return -1;
    }

    // Instanciar el recurso con RefCounted
    Ref<Texture2D> mario_tex = new Texture2D();
    if (!mario_tex->load("mario.png", false) && !mario_tex->load("bin/mario.png", false)) {
        std::cerr << "[PuzzleEngine] No se pudo encontrar mario.png" << std::endl;
    }

    // Tamaño de cada celda del patrón
    constexpr float TILE_WIDTH = 48.0f;
    constexpr float TILE_HEIGHT = 60.0f;
    constexpr float TITLE_BAR_HEIGHT = 28.0f;

    window.run([&](double delta) {
        if (!mario_tex.is_valid()) return;

        RenderingGeometry& geom = window.get_rendering_geometry();
        Size2i win_sz = window.get_size();

        float start_y = TITLE_BAR_HEIGHT;
        float max_w = static_cast<float>(win_sz.width);
        float max_h = static_cast<float>(win_sz.height);

        // Rellenar la ventana por filas y columnas
        for (float y = start_y; y < max_h; y += TILE_HEIGHT) {
            for (float x = 0.0f; x < max_w; x += TILE_WIDTH) {
                geom.draw_texture(
                    Rect2(x, y, TILE_WIDTH, TILE_HEIGHT),
                    mario_tex->get_rendering_texture()
                );
            }
        }
    });

    return 0;
}