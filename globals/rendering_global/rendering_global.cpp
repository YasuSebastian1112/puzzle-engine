// ========================================================================= //
// PUZZLE ENGINE - 2D/3D GAME ENGINE
// ------------------------------------------------------------------------- //
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// ========================================================================= //

#include "rendering_global.h"
#include <algorithm>
#include <iostream>

RenderingGlobal& RenderingGlobal::get() {
    static RenderingGlobal instance;
    return instance;
}

bool RenderingGlobal::init() {
    // La inicialización de contextos de hardware se delega a cada ventana en initialize()
    return true;
}

void RenderingGlobal::poll_events() {
    // Procesar la cola de eventos si existe al menos una ventana activa
    if (!windows.empty() && windows[0]) {
        windows[0]->poll_events();
    }
}

void RenderingGlobal::shutdown() {
    // Cerrar y liberar explícitamente todas las ventanas antes de vaciar el contenedor
    for (auto& win : windows) {
        if (win) {
            win->close();
        }
    }
    windows.clear();
}

std::shared_ptr<Window> RenderingGlobal::create_window(
    const Size2i& p_size,
    const String& p_title,
    const Point2i& p_position,
    Window::Type p_type
) {
    auto new_window = std::make_shared<Window>();

    if (!new_window->initialize(p_title, p_size, p_position, p_type)) {
        std::cerr << "[RenderingGlobal] Error al inicializar ventana: " << p_title.c_str() << std::endl;
        return nullptr;
    }

    windows.push_back(new_window);
    return new_window;
}

void RenderingGlobal::destroy_window(const std::shared_ptr<Window>& p_window) {
    if (!p_window) return;

    auto it = std::find(windows.begin(), windows.end(), p_window);
    if (it != windows.end()) {
        (*it)->close();
        windows.erase(it);
    }
}

std::shared_ptr<Window> RenderingGlobal::get_primary_window() const {
    if (windows.empty()) {
        return nullptr;
    }
    return windows[0];
}

void RenderingGlobal::clear_screen(const Color& p_color) {
    auto primary = get_primary_window();
    if (primary) {
        primary->clear(p_color);
    }
}