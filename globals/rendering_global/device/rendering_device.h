// ========================================================================= //
// PUZZLE ENGINE - 2D/3D GAME ENGINE
// ------------------------------------------------------------------------- //
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// ========================================================================= //

#pragma once

#include "core/variant.h"
#include <functional>

using WindowHandle = void*;

class RenderingDevice {
public:
    using FrameRenderCallback = std::function<void()>;

    enum class WindowMode {
        Windowed,
        Maximized,
        Fullscreen
    };

private:
    WindowHandle native_window = nullptr;

    // Identificadores de objetos de OpenGL
    uint32_t geom_vao = 0;
    uint32_t geom_vbo = 0;
    uint32_t geom_ebo = 0;
    uint32_t geom_shader = 0;
    int32_t geom_proj_loc = -1;

    // Capacidades de búfer para el pipeline geométrico
    size_t current_max_vertex_bytes = 1024 * 1024 * 2; // 2 MB iniciales
    size_t current_max_index_bytes  = 1024 * 512;      // 512 KB iniciales

    // Dimensiones y métricas de ventana
    Size2i current_screen_size = Size2i(1280, 720);
    Size2i windowed_size = Size2i(1280, 720);
    Point2i windowed_pos = Point2i(100, 100);

    // Caché de la matriz de proyección ortográfica
    float m_cached_ortho_mat[16] = {0};
    bool m_proj_dirty = true;

    bool in_modal_sizing = false;
    FrameRenderCallback render_callback = nullptr;

    void update_orthographic_projection();

public:
    RenderingDevice() = default;
    ~RenderingDevice();

    // Creación y ciclo de vida de la ventana
    bool create_window(const String& p_title, const Size2i& p_size, const Point2i& p_pos);
    void destroy_window();
    void poll_events();
    void swap_buffers();
    bool window_should_close() const;

    // Métricas y propiedades de la ventana
    void set_window_size(const Size2i& p_size);
    Size2i get_window_size() const;
    Size2i get_framebuffer_size() const;

    void set_window_position(const Point2i& p_pos);
    Point2i get_window_position() const;

    void set_window_title(const String& p_title);
    void set_vsync(bool p_enabled);

    // Modos de visualización
    void set_window_mode(WindowMode p_mode);
    bool is_window_maximized() const;

    void set_render_callback(FrameRenderCallback p_callback) { render_callback = p_callback; }
    void trigger_frame_update();

    // Operaciones de estado de renderizado y viewport
    void set_viewport(const Point2i& p_position, const Size2i& p_size);
    void set_clear_color(const Color& p_color);
    void clear_buffers(bool p_color = true, bool p_depth = true, bool p_stencil = false);
    void set_scissor(const Point2i& p_position, const Size2i& p_size);
    void set_scissor_enabled(bool p_enabled);
    void set_blend_enabled(bool p_enabled);

    WindowHandle get_native_handle() const { return native_window; }

    // Pipeline geométrico 2D
    void init_geometry_pipeline();
    void render_geometry(const void* p_vertices, size_t p_vertex_bytes, const uint32_t* p_indices, size_t p_index_count);
};