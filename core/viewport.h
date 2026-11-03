// ========================================================================= //
// PUZZLE ENGINE - 2D/3D GAME ENGINE
// ------------------------------------------------------------------------- //
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// ========================================================================= //

#pragma once

#include "core/mathematics/math_2d.h"
#include "color.h"

class RenderingDevice;
class Camera2D;

class Viewport {
public:
    Viewport() = default;
    Viewport(const Rect2& p_screen_rect) : m_screen_rect(p_screen_rect) {}

    // Configura el área física en la ventana
    void set_screen_rect(const Rect2& p_rect) { m_screen_rect = p_rect; }
    const Rect2& get_screen_rect() const { return m_screen_rect; }

    // Tamaño útil para la escena
    float get_width() const { return m_screen_rect.size.x; }
    float get_height() const { return m_screen_rect.size.y; }
    Size2i get_size() const { 
        return Size2i(static_cast<int>(m_screen_rect.size.x), static_cast<int>(m_screen_rect.size.y)); 
    }

    // Gestión de cámara activa
    void set_current_camera(Camera2D* p_camera) { m_current_camera = p_camera; }
    Camera2D* get_current_camera() const { return m_current_camera; }

    // Activa glViewport, glScissor y la proyección ortográfica adaptada
    void bind(RenderingDevice* p_device, int p_total_window_height);

    // Desactiva el scissor test
    void unbind(RenderingDevice* p_device = nullptr);

    // Transforma una coordenada de pantalla física a coordenada local del Viewport
    Vector2 to_local(const Vector2& p_screen_pos) const {
        return Vector2(
            p_screen_pos.x - m_screen_rect.position.x,
            p_screen_pos.y - m_screen_rect.position.y
        );
    }

    // Transforma una coordenada local del Viewport al espacio de mundo (usando la cámara si existe)
    Vector2 to_world_coordinates(const Vector2& p_local_pos) const;

    // Transforma una coordenada del mundo al espacio local del Viewport
    Vector2 to_local_coordinates(const Vector2& p_world_pos) const;

    // Verifica si un punto en pantalla cae dentro de este Viewport
    bool contains_screen_point(const Vector2& p_screen_pos) const {
        return m_screen_rect.has_point(p_screen_pos);
    }

private:
    Rect2 m_screen_rect;
    Camera2D* m_current_camera = nullptr;
};