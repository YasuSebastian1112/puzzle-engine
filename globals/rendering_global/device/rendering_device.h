// ========================================================================= //
// Sierra Engine - 2D/3D Hybrid Game Engine
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// ========================================================================= //

#pragma once
#include "core/variant.h"

using WindowHandle = void*; // Puntero Hacia GLFWwindow

class RenderingDevice { // Hacer llamadas para manejar opengl
private:

    WindowHandle native_window = nullptr;

    uint32_t geom_vao = 0;
    uint32_t geom_vbo = 0;
    uint32_t geom_ebo = 0;
    uint32_t geom_shader = 0;

public:

    // Creacion de ventanas
    bool create_window(const String& p_title, const Size2i& p_size, const Point2i& p_pos);
    void destroy_window();
    void poll_events();
    void swap_buffers();
    bool window_should_close() const;

    // Control de ventana desde el backend gráfico
    void set_window_size(const Size2i& p_size);
    void set_window_position(const Point2i& p_pos);
    void set_window_title(const String& p_title);
    void set_vsync(bool p_enabled);

    // Traducción de transformaciones y viewport
    void set_viewport(const Point2i &p_position, const Size2i &p_size);
    void set_clear_color(const Color &p_color);
    void clear_buffers(bool p_color = true, bool p_depth = true, bool p_stencil = false);
    void set_scissor(const Point2i &p_position, const Size2i &p_size);
    void set_scissor_enabled(bool p_enabled);
    void set_blend_enabled(bool p_enabled);

    WindowHandle get_native_handle() const { return native_window; }

    // Renderizado de formas

    void init_geometry_pipeline();
    void render_geometry(const void* p_vertices, size_t p_vertex_size, const uint32_t* p_indices, size_t p_index_count);    

    RenderingDevice() = default;
    ~RenderingDevice();
};