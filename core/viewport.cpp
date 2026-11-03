// ========================================================================= //
// PUZZLE ENGINE - 2D/3D GAME ENGINE
// ------------------------------------------------------------------------- //
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// ========================================================================= //

#include "core/viewport.h"
#include "globals/rendering_global/device/rendering_device.h"
#include "classes/2d/camera_2d.h"
#include <glad/glad.h>

void Viewport::bind(RenderingDevice* p_device, int p_total_window_height) {
    if (!p_device || p_total_window_height <= 0 || m_screen_rect.size.x <= 0.0f || m_screen_rect.size.y <= 0.0f) {
        return;
    }

    int vx = static_cast<int>(m_screen_rect.position.x);
    int vw = static_cast<int>(m_screen_rect.size.x);
    int vh = static_cast<int>(m_screen_rect.size.y);

    if (vw <= 0 || vh <= 0) {
        return;
    }

    // Conversión de coordenadas de arriba-izquierda a abajo-izquierda para OpenGL
    int vy_gl = p_total_window_height - (static_cast<int>(m_screen_rect.position.y) + vh);

    // 1. Delimitar el área gráfica mediante el wrapper del RenderingDevice
    p_device->set_scissor_enabled(true);
    p_device->set_viewport(Point2i(vx, vy_gl), Size2i(vw, vh));
    p_device->set_scissor(Point2i(vx, vy_gl), Size2i(vw, vh));

    // 2. Establecer la proyección ortográfica local de este Viewport
    // (0, 0) queda exactamente en la esquina superior izquierda de este rectángulo
    p_device->set_orthographic_projection(
        0.0f,
        static_cast<float>(vw),
        static_cast<float>(vh),
        0.0f
    );
}

void Viewport::unbind(RenderingDevice* p_device) {
    if (p_device) {
        p_device->set_scissor_enabled(false);
    } else {
        glDisable(GL_SCISSOR_TEST);
    }
}

Vector2 Viewport::to_world_coordinates(const Vector2& p_local_pos) const {
    if (m_current_camera && m_current_camera->is_enabled()) {
        return m_current_camera->screen_to_world(p_local_pos, get_size());
    }
    return p_local_pos;
}

Vector2 Viewport::to_local_coordinates(const Vector2& p_world_pos) const {
    if (m_current_camera && m_current_camera->is_enabled()) {
        return m_current_camera->world_to_screen(p_world_pos, get_size());
    }
    return p_world_pos;
}