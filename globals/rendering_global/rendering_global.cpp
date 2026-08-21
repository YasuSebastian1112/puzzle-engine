#include "rendering_global.h"
#include <iostream>

RenderingGlobal& RenderingGlobal::get() {
    static RenderingGlobal instance;
    return instance;
}

bool RenderingGlobal::init() {
    if (!glfwInit()) {
        std::cerr << "[Sierra Engine] Error fatal: No se pudo inicializar GLFW.\n";
        return false;
    }
    return true;
}

std::shared_ptr<Window> RenderingGlobal::create_window(int width, int height, const std::string& title, Window::Type type) {
    auto win = std::make_shared<Window>(width, height, title, type);
    if (!win->is_valid()) {
        return nullptr;
    }

    if (!glad_initialized) {
        win->make_current();
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cerr << "[Sierra Engine] Error fatal: No se pudo cargar GLAD.\n";
            return nullptr;
        }
        glad_initialized = true;

        std::cout << "[Sierra Engine] Contexto OpenGL Inicializado:\n";
        std::cout << "  GPU: " << glGetString(GL_RENDERER) << "\n";
        std::cout << "  Versión: " << glGetString(GL_VERSION) << "\n";
    }

    windows.push_back(win);
    return win;
}

std::shared_ptr<Window> RenderingGlobal::get_primary_window() const {
    if (windows.empty()) return nullptr;
    return windows[0];
}

void RenderingGlobal::poll_events() {
    glfwPollEvents();
}

void RenderingGlobal::clear_screen(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RenderingGlobal::shutdown() {
    windows.clear();
    glfwTerminate();
}
