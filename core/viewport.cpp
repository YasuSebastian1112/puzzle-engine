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
#include <algorithm>
#include <cmath>

Viewport::Viewport(const Rect2& p_screen_rect) : m_screen_rect(p_screen_rect) {
    _recalculate_layout();
}

void Viewport::set_screen_rect(const Rect2& p_rect) {
    m_screen_rect = p_rect;
    _recalculate_layout();
}

void Viewport::set_base_size(const Size2i& p_size) {
    m_base_size = p_size;
    _recalculate_layout();
}

void Viewport::set_stretch_mode(StretchMode p_mode) {
    m_stretch_mode = p_mode;
    _recalculate_layout();
}

void Viewport::set_stretch_aspect(StretchAspect p_aspect) {
    m_stretch_aspect = p_aspect;
    _recalculate_layout();
}

void Viewport::set_stretch_scale(float p_scale) {
    m_stretch_scale = (p_scale > 0.1f) ? p_scale : 0.1f;
    _recalculate_layout();
}

void Viewport::set_stretch_scale_mode(StretchScaleMode p_mode) {
    m_stretch_scale_mode = p_mode;
    _recalculate_layout();
}

void Viewport::_recalculate_layout() {
    float avail_w = m_screen_rect.size.x;
    float avail_h = m_screen_rect.size.y;

    if (avail_w <= 0.0f || avail_h <= 0.0f) {
        m_render_rect = m_screen_rect;
        m_logical_size = Vector2(0.0f, 0.0f);
        return;
    }

    float user_scale = m_stretch_scale;
    if (m_stretch_scale_mode == StretchScaleMode::Integer) {
        user_scale = std::max(1.0f, std::floor(user_scale));
    }

    // Modo Disabled: 1:1 con los píxeles físicos (dividido solo por Scale)
    if (m_stretch_mode == StretchMode::Disabled) {
        m_render_rect = m_screen_rect;
        m_logical_size = Vector2(avail_w / user_scale, avail_h / user_scale);
        return;
    }

    float base_w = (m_base_size.width > 0) ? static_cast<float>(m_base_size.width) : avail_w;
    float base_h = (m_base_size.height > 0) ? static_cast<float>(m_base_size.height) : avail_h;

    float scale_x = avail_w / base_w;
    float scale_y = avail_h / base_h;

    auto apply_scale_mode = [this](float s) {
        if (m_stretch_scale_mode == StretchScaleMode::Integer) {
            return std::max(1.0f, std::floor(s));
        }
        return s;
    };

    switch (m_stretch_aspect) {
        case StretchAspect::Ignore: {
            m_render_rect = m_screen_rect;
            m_logical_size = Vector2(base_w / user_scale, base_h / user_scale);
            break;
        }
        case StretchAspect::Keep: {
            float s = apply_scale_mode(std::min(scale_x, scale_y));
            float rw = base_w * s;
            float rh = base_h * s;
            float rx = m_screen_rect.position.x + std::floor((avail_w - rw) * 0.5f);
            float ry = m_screen_rect.position.y + std::floor((avail_h - rh) * 0.5f);

            m_render_rect = Rect2(Vector2(rx, ry), Vector2(rw, rh));
            m_logical_size = Vector2(base_w / user_scale, base_h / user_scale);
            break;
        }
        case StretchAspect::KeepWidth: {
            float s = apply_scale_mode(scale_x);
            m_render_rect = m_screen_rect;
            m_logical_size = Vector2(base_w / user_scale, (avail_h / s) / user_scale);
            break;
        }
        case StretchAspect::KeepHeight: {
            float s = apply_scale_mode(scale_y);
            m_render_rect = m_screen_rect;
            m_logical_size = Vector2((avail_w / s) / user_scale, base_h / user_scale);
            break;
        }
        case StretchAspect::Expand: {
            float s = apply_scale_mode(std::min(scale_x, scale_y));
            m_render_rect = m_screen_rect;
            m_logical_size = Vector2((avail_w / s) / user_scale, (avail_h / s) / user_scale);
            break;
        }
    }
}

void Viewport::bind(RenderingDevice* p_device, int p_total_window_height) {
    if (!p_device || p_total_window_height <= 0 || m_render_rect.size.x <= 0.0f || m_render_rect.size.y <= 0.0f) {
        return;
    }

    int vx = static_cast<int>(m_render_rect.position.x);
    int vw = static_cast<int>(m_render_rect.size.x);
    int vh = static_cast<int>(m_render_rect.size.y);

    if (vw <= 0 || vh <= 0) return;

    // Conversión de coordenadas de arriba-izquierda a abajo-izquierda para OpenGL
    int vy_gl = p_total_window_height - (static_cast<int>(m_render_rect.position.y) + vh);

    p_device->set_scissor_enabled(true);
    p_device->set_viewport(Point2i(vx, vy_gl), Size2i(vw, vh));
    p_device->set_scissor(Point2i(vx, vy_gl), Size2i(vw, vh));

    // Proyección ortográfica en espacio lógico (0, 0) -> (logical_w, logical_h)
    p_device->set_orthographic_projection(0.0f, m_logical_size.x, m_logical_size.y, 0.0f);
}

void Viewport::unbind(RenderingDevice* p_device) {
    if (p_device) {
        p_device->set_scissor_enabled(false);
    } else {
        glDisable(GL_SCISSOR_TEST);
    }
}

Vector2 Viewport::to_local(const Vector2& p_screen_pos) const {
    if (m_render_rect.size.x <= 0.0f || m_render_rect.size.y <= 0.0f) {
        return Vector2(0.0f, 0.0f);
    }

    float nx = (p_screen_pos.x - m_render_rect.position.x) / m_render_rect.size.x;
    float ny = (p_screen_pos.y - m_render_rect.position.y) / m_render_rect.size.y;

    return Vector2(nx * m_logical_size.x, ny * m_logical_size.y);
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