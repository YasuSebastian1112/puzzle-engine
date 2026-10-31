// ========================================================================= //
// PUZZLE ENGINE - 2D/3D GAME ENGINE
// ------------------------------------------------------------------------- //
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// ========================================================================= //

#pragma once

#include "window.h"
#include "core/variant.h"
#include <memory>
#include <vector>

class RenderingGlobal {
public:
    // Acceso a la instancia única global (patrón Singleton)
    static RenderingGlobal& get();

    // Ciclo de vida del subsistema
    bool init();
    void poll_events();
    void shutdown();

    // Creación y administración de ventanas
    std::shared_ptr<Window> create_window(
        const Size2i& p_size,
        const String& p_title,
        const Point2i& p_position = Point2i(100, 100),
        Window::Type p_type = Window::Type::Game
    );

    void destroy_window(const std::shared_ptr<Window>& p_window);
    std::shared_ptr<Window> get_primary_window() const;
    const std::vector<std::shared_ptr<Window>>& get_windows() const { return windows; }

    // Limpieza de búfer sobre la ventana primaria
    void clear_screen(const Color& p_color);

private:
    RenderingGlobal() = default;
    ~RenderingGlobal() = default;

    // Restricción de duplicación y asignación
    RenderingGlobal(const RenderingGlobal&) = delete;
    RenderingGlobal& operator=(const RenderingGlobal&) = delete;

    std::vector<std::shared_ptr<Window>> windows;
};