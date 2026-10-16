// ========================================================================= //
// Sierra Engine - 2D/3D Hybrid Game Engine
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// ======================================================================

#pragma once
#include "window.h"
#include "core/variant.h"
#include <memory>
#include <vector>

class RenderingGlobal {
public:
    // Acceso Singleton (instancia única estática)
    static RenderingGlobal& get();

    // Ciclo de vida global
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

    std::shared_ptr<Window> get_primary_window() const;
    const std::vector<std::shared_ptr<Window>>& get_windows() const { return windows; }

    // Limpieza de pantalla sobre la ventana primaria
    void clear_screen(const Color& p_color);

private:
    RenderingGlobal() = default;
    ~RenderingGlobal() = default;

    // Evitar copias accidentales del Singleton
    RenderingGlobal(const RenderingGlobal&) = delete;
    RenderingGlobal& operator=(const RenderingGlobal&) = delete;

    std::vector<std::shared_ptr<Window>> windows;
};