// ========================================================================= //
// PUZZLE ENGINE - 2D/3D GAME ENGINE
// ------------------------------------------------------------------------- //
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// ========================================================================= //

#include "rendering_global.h"
#include "core/time/engine_time.h"
#include <algorithm>
#include <iostream>
#include "classes/resources/font_resource.h"

#if defined(_WIN32)
#include "platform/windows/os_windows.h"
#endif


RenderingGlobal& RenderingGlobal::get() {
    static RenderingGlobal instance;
    return instance;
}

RenderingGlobal::~RenderingGlobal() {
    shutdown();
}

bool RenderingGlobal::init() {
    // La inicialización de contextos de hardware se gestiona por referencia en RenderingDevice
    return true;
}

void RenderingGlobal::poll_events() {
    if (!windows.empty() && windows[0]) {
        windows[0]->poll_events();
    }
}

void RenderingGlobal::shutdown() {
    m_running = false;
    for (auto& win : windows) {
        if (win) {
            win->close();
        }
    }

    FontResource::clear_cache();
    for (auto& win : windows) {
        if (win) win->close();
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
    return windows.empty() ? nullptr : windows[0];
}

void RenderingGlobal::clear_screen(const Color& p_color) {
    if (auto primary = get_primary_window()) {
        primary->clear(p_color);
    }
}

void RenderingGlobal::run(GlobalUpdateCallback p_global_update, ClosePolicy p_close_policy) {
    if (windows.empty()) return;

#if defined(_WIN32)
    OSWindows::initialize_system_timer();

    static uint32_t startup_warmup_frames = 0;
    if (startup_warmup_frames <= 3) {
        startup_warmup_frames++;
        if (startup_warmup_frames == 3) {
            OSWindows::trim_memory();
        }
    }

#endif

    Time::init();
    m_running = true;

    while (m_running && !windows.empty()) {
        Time::update();
        double delta = Time::get_process_delta();

        poll_events();

        // Evaluar política de cierre antes de eliminar ventanas del vector
        if (p_close_policy == ClosePolicy::WhenPrimaryClosed) {
            if (!windows[0] || windows[0]->should_close()) {
                break;
            }
        } else if (p_close_policy == ClosePolicy::WhenAnyClosed) {
            bool any_closed = std::any_of(windows.begin(), windows.end(),
                [](const std::shared_ptr<Window>& w) { return !w || w->should_close(); });
            if (any_closed) {
                break;
            }
        }

        // Eliminar ventanas secundarias cerradas manualmente
        windows.erase(
            std::remove_if(windows.begin(), windows.end(),
                [](const std::shared_ptr<Window>& w) { return !w || w->should_close(); }),
            windows.end()
        );

        if (windows.empty()) break;

        if (p_global_update) {
            p_global_update(delta);
        }

        auto active_windows = windows;
        for (auto& win : active_windows) {
            if (win && !win->should_close()) {
                win->process_frame(delta);
            }
        }

        Time::delay_to_target_fps();
    }

    shutdown();

#if defined(_WIN32)
    OSWindows::restore_system_timer();
#endif
}