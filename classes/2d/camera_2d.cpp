// ========================================================================= //
// PUZZLE ENGINE - 2D/3D GAME ENGINE
// ------------------------------------------------------------------------- //
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// ========================================================================= //

#include "camera_2d.h"
#include "core/viewport.h"
#include "core/class_db.h"

Camera2D::Camera2D() {
    m_smoothed_position = get_global_position();
}

void Camera2D::set_zoom(const Vector2& p_zoom) {
    m_zoom.x = (p_zoom.x < 0.05f) ? 0.05f : p_zoom.x;
    m_zoom.y = (p_zoom.y < 0.05f) ? 0.05f : p_zoom.y;
}

void Camera2D::clamp_to_limits(Vector2& r_pos, const Size2i& p_viewport_size) const {
    if (m_anchor_mode == AnchorMode::DragCenter) {
        float half_w = (static_cast<float>(p_viewport_size.width) * 0.5f) / m_zoom.x;
        float half_h = (static_cast<float>(p_viewport_size.height) * 0.5f) / m_zoom.y;

        float min_x = static_cast<float>(m_limit_left) + half_w;
        float max_x = static_cast<float>(m_limit_right) - half_w;
        float min_y = static_cast<float>(m_limit_top) + half_h;
        float max_y = static_cast<float>(m_limit_bottom) - half_h;

        if (min_x <= max_x) {
            r_pos.x = Math::clamp(r_pos.x, min_x, max_x);
        }
        if (min_y <= max_y) {
            r_pos.y = Math::clamp(r_pos.y, min_y, max_y);
        }
    } else {
        float max_x = static_cast<float>(m_limit_right) - (static_cast<float>(p_viewport_size.width) / m_zoom.x);
        float max_y = static_cast<float>(m_limit_bottom) - (static_cast<float>(p_viewport_size.height) / m_zoom.y);

        r_pos.x = Math::clamp(r_pos.x, static_cast<float>(m_limit_left), max_x);
        r_pos.y = Math::clamp(r_pos.y, static_cast<float>(m_limit_top), max_y);
    }
}

void Camera2D::update_camera(double p_delta, const Size2i& p_viewport_size) {
    Vector2 target_pos = get_global_position();

    if (m_position_smoothing_enabled && p_delta > 0.0) {
        float t = 1.0f - Math::exp(-m_position_smoothing_speed * static_cast<float>(p_delta));
        m_smoothed_position = m_smoothed_position + (target_pos - m_smoothed_position) * t;
    } else {
        m_smoothed_position = target_pos;
    }

    clamp_to_limits(m_smoothed_position, p_viewport_size);
}

Vector2 Camera2D::get_camera_screen_center(const Size2i& p_viewport_size) const {
    Vector2 base_pos = m_position_smoothing_enabled ? m_smoothed_position : get_global_position();
    return base_pos + m_offset;
}

void Camera2D::get_view_matrix(float* out_mat4, const Size2i& p_viewport_size) const {
    for (int i = 0; i < 16; ++i) {
        out_mat4[i] = 0.0f;
    }
    out_mat4[0] = 1.0f;
    out_mat4[5] = 1.0f;
    out_mat4[10] = 1.0f;
    out_mat4[15] = 1.0f;

    Vector2 cam_center = get_camera_screen_center(p_viewport_size);
    float rot = get_global_rotation();

    Vector2 screen_offset(0.0f, 0.0f);
    if (m_anchor_mode == AnchorMode::DragCenter) {
        screen_offset = Vector2(
            static_cast<float>(p_viewport_size.width) * 0.5f,
            static_cast<float>(p_viewport_size.height) * 0.5f
        );
    }

    float c = Math::cos(-rot);
    float s = Math::sin(-rot);

    out_mat4[0] = c * m_zoom.x;
    out_mat4[1] = s * m_zoom.x;
    out_mat4[4] = -s * m_zoom.y;
    out_mat4[5] = c * m_zoom.y;

    out_mat4[12] = screen_offset.x - (cam_center.x * out_mat4[0] + cam_center.y * out_mat4[4]);
    out_mat4[13] = screen_offset.y - (cam_center.x * out_mat4[1] + cam_center.y * out_mat4[5]);
}

Vector2 Camera2D::screen_to_world(const Vector2& p_screen_pos, const Size2i& p_viewport_size) const {
    Vector2 screen_offset(0.0f, 0.0f);
    if (m_anchor_mode == AnchorMode::DragCenter) {
        screen_offset = Vector2(
            static_cast<float>(p_viewport_size.width) * 0.5f,
            static_cast<float>(p_viewport_size.height) * 0.5f
        );
    }

    Vector2 rel = p_screen_pos - screen_offset;
    float rot = get_global_rotation();

    if (rot != 0.0f) {
        float c = Math::cos(-rot);
        float s = Math::sin(-rot);
        rel = Vector2(rel.x * c - rel.y * s, rel.x * s + rel.y * c);
    }

    rel.x /= m_zoom.x;
    rel.y /= m_zoom.y;

    Vector2 cam_center = get_camera_screen_center(p_viewport_size);
    return rel + cam_center;
}

Vector2 Camera2D::world_to_screen(const Vector2& p_world_pos, const Size2i& p_viewport_size) const {
    Vector2 cam_center = get_camera_screen_center(p_viewport_size);
    Vector2 rel = p_world_pos - cam_center;

    rel.x *= m_zoom.x;
    rel.y *= m_zoom.y;

    float rot = get_global_rotation();
    if (rot != 0.0f) {
        float c = Math::cos(rot);
        float s = Math::sin(rot);
        rel = Vector2(rel.x * c - rel.y * s, rel.x * s + rel.y * c);
    }

    Vector2 screen_offset(0.0f, 0.0f);
    if (m_anchor_mode == AnchorMode::DragCenter) {
        screen_offset = Vector2(
            static_cast<float>(p_viewport_size.width) * 0.5f,
            static_cast<float>(p_viewport_size.height) * 0.5f
        );
    }

    return rel + screen_offset;
}

void Camera2D::make_current(Viewport* p_viewport) {
    if (p_viewport) {
        p_viewport->set_current_camera(this);
    }
}

// -----------------------------------------------------------------------------
// VINCULACIÓN AL SISTEMA DE REFLEXIÓN (ClassDB)
// -----------------------------------------------------------------------------
void Camera2D::_bind_methods() {
    // Estado y desplazamiento de la vista
    ClassDB::add_property("enabled", &Camera2D::set_enabled, &Camera2D::is_enabled);
    ClassDB::add_property("offset",  &Camera2D::set_offset,  &Camera2D::get_offset);
    ClassDB::add_property("zoom",    &Camera2D::set_zoom,    &Camera2D::get_zoom);

    // Suavizado (Smoothing)
    ClassDB::add_property("position_smoothing_enabled", &Camera2D::set_position_smoothing_enabled, &Camera2D::is_position_smoothing_enabled);
    ClassDB::add_property("position_smoothing_speed",   &Camera2D::set_position_smoothing_speed,   &Camera2D::get_position_smoothing_speed);

    // Límites de mundo (Limits)
    ClassDB::add_property("limit_left",   &Camera2D::set_limit_left,   &Camera2D::get_limit_left);
    ClassDB::add_property("limit_top",    &Camera2D::set_limit_top,    &Camera2D::get_limit_top);
    ClassDB::add_property("limit_right",  &Camera2D::set_limit_right,  &Camera2D::get_limit_right);
    ClassDB::add_property("limit_bottom", &Camera2D::set_limit_bottom, &Camera2D::get_limit_bottom);
}