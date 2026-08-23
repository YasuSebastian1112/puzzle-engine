#include "globals/rendering_global/window.h"
#include <chrono>

int main() {
    Window window;

    // Inicializar la ventana con título y resolución usando Size2i
    if (!window.initialize("Puzzle Engine", Size2i(1280, 720), Point2i(100, 100))) {
        return -1;
    }

    auto previous_time = std::chrono::high_resolution_clock::now();

    // Bucle principal del motor
    while (!window.should_close()) {
        // 1. Calcular delta time
        auto current_time = std::chrono::high_resolution_clock::now();
        float delta_time = std::chrono::duration<float, std::chrono::seconds::period>(current_time - previous_time).count();
        previous_time = current_time;

        // 2. Procesar eventos del sistema y entradas
        window.poll_events();

        // 3. Limpiar buffer de pantalla
        window.clear(Color(0.12f, 0.12f, 0.15f, 1.0f));

        // TODO: window.update(delta_time) y renderizado de nodos aquí

        // 4. Intercambiar buffers
        window.swap_buffers();
    }

    return 0;
}