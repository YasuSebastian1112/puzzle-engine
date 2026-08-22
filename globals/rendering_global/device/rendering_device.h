// ========================================================================= //
// Sierra Engine - 2D/3D Hybrid Game Engine
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// ========================================================================= //

#pragma once

#include "core/variant.h"

class RenderingDevice { // Hacer llamadas para manejar opengl
public:

// Inicialización del loader (GLAD)
    bool initialize();

    // Traducción de transformaciones y viewport
    void set_viewport(const Point2i &p_position, const Size2i &p_size);
    void set_clear_color(const Color &p_color);
    void clear_buffers(bool p_color = true, bool p_depth = true, bool p_stencil = false);

    // Ajustes de estado comunes
    void set_scissor(const Point2i &p_position, const Size2i &p_size);
    void set_scissor_enabled(bool p_enabled);
    void set_blend_enabled(bool p_enabled);

};