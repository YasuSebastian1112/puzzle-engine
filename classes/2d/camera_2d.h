// ========================================================================= //
// PUZZLE ENGINE - 2D/3D GAME ENGINE
// ------------------------------------------------------------------------- //
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// ========================================================================= //

#pragma once

#include "node_2d.h"
#include "core/mathematics/math_2d.h"
#include "core/mathematics/math_functions.h"

class Viewport;

class Camera2D : public Node2D {
    CLASS_TYPE(Camera2D, Node2D);

public:
    enum class AnchorMode {
        FixedTopLeft,
        DragCenter
    };

private:
    Vector2 m_offset = Vector2(0.0f, 0.0f);
    Vector2 m_zoom = Vector2(1.0f, 1.0f);
    AnchorMode m_anchor_mode = AnchorMode::DragCenter;
    bool m_enabled = true;

    // Suavizado de seguimiento (Smoothing)
    bool m_position_smoothing_enabled = false;
    float m_position_smoothing_speed = 5.0f;
    Vector2 m_smoothed_position = Vector2(0.0f, 0.0f);

    // Límites de la cámara en el espacio del mundo
    int m_limit_left = -10000000;
    int m_limit_top = -10000000;
    int m_limit_right = 10000000;
    int m_limit_bottom = 10000000;

    void clamp_to_limits(Vector2& r_pos, const Size2i& p_viewport_size) const;

public:
    Camera2D();
    virtual ~Camera2D() override = default;

    void set_enabled(bool p_enabled) { m_enabled = p_enabled; }
    bool is_enabled() const { return m_enabled; }

    void set_offset(const Vector2& p_offset) { m_offset = p_offset; }
    Vector2 get_offset() const { return m_offset; }

    void set_zoom(const Vector2& p_zoom);
    Vector2 get_zoom() const { return m_zoom; }

    void set_anchor_mode(AnchorMode p_mode) { m_anchor_mode = p_mode; }
    AnchorMode get_anchor_mode() const { return m_anchor_mode; }

    // Control de suavizado
    void set_position_smoothing_enabled(bool p_enabled) { m_position_smoothing_enabled = p_enabled; }
    bool is_position_smoothing_enabled() const { return m_position_smoothing_enabled; }
    void set_position_smoothing_speed(float p_speed) { m_position_smoothing_speed = p_speed; }
    float get_position_smoothing_speed() const { return m_position_smoothing_speed; }

    // Control de límites
    void set_limit_left(int p_val) { m_limit_left = p_val; }
    int get_limit_left() const { return m_limit_left; }

    void set_limit_top(int p_val) { m_limit_top = p_val; }
    int get_limit_top() const { return m_limit_top; }

    void set_limit_right(int p_val) { m_limit_right = p_val; }
    int get_limit_right() const { return m_limit_right; }

    void set_limit_bottom(int p_val) { m_limit_bottom = p_val; }
    int get_limit_bottom() const { return m_limit_bottom; }

    // Actualización de estado en el bucle principal
    void update_camera(double p_delta, const Size2i& p_viewport_size);

    // Posición del centro focal de la cámara
    Vector2 get_camera_screen_center(const Size2i& p_viewport_size) const;

    // Matriz de vista ortográfica 4x4
    void get_view_matrix(float* out_mat4, const Size2i& p_viewport_size) const;

    // Transformación de coordenadas entre espacio de viewport local y espacio de mundo
    Vector2 screen_to_world(const Vector2& p_screen_pos, const Size2i& p_viewport_size) const;
    Vector2 world_to_screen(const Vector2& p_world_pos, const Size2i& p_viewport_size) const;

    void make_current(Viewport* p_viewport);

};