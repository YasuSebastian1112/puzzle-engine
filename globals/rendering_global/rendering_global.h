// ========================================================================= //
// PUZZLE ENGINE - 2D/3D GAME ENGINE
// ------------------------------------------------------------------------- //
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// ========================================================================= //

#pragma once

#include "window.h"
#include "core/variant.h"
#include <functional>
#include <memory>
#include <vector>

class RenderingGlobal {
public:

    enum class ClosePolicy {
        WhenAllClosed,
        WhenPrimaryClosed,
        WhenAnyClosed
    };

    using GlobalUpdateCallback = std::function<void(double delta)>;

    // Acceso a la instancia única global (Singleton)
    static RenderingGlobal& get();

    // Ciclo de vida del subsistema
    bool init();
    void poll_events();
    void shutdown();

    // Bucle principal multi-ventana del motor
    void run(GlobalUpdateCallback p_global_update = nullptr,
             ClosePolicy p_close_policy = ClosePolicy::WhenPrimaryClosed);
    void stop() { m_running = false; }

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
    bool has_active_windows() const { return !windows.empty(); }

    // Limpieza de búfer sobre la ventana primaria
    void clear_screen(const Color& p_color);

private:
    RenderingGlobal() = default;
    ~RenderingGlobal();

    RenderingGlobal(const RenderingGlobal&) = delete;
    RenderingGlobal& operator=(const RenderingGlobal&) = delete;

    std::vector<std::shared_ptr<Window>> windows;
    bool m_running = false;
};