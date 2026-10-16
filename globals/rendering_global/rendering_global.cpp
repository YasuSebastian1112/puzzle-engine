// ========================================================================= //
// Sierra Engine - 2D/3D Hybrid Game Engine
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// ======================================================================

#include "rendering_global.h"
#include <iostream>

#include "rendering_global.h"

RenderingGlobal& RenderingGlobal::get() {
    static RenderingGlobal instance;
    return instance;
}

bool RenderingGlobal::init() {
    // La inicialización por hardware se delega a las ventanas al crearse
    return true;
}

void RenderingGlobal::poll_events() {
    if (!windows.empty() && windows[0]) {
        windows[0]->poll_events();
    }
}

void RenderingGlobal::shutdown() {
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
        return nullptr;
    }

    windows.push_back(new_window);
    return new_window;
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