// ========================================================================= //
// PUZZLE ENGINE - 2D/3D GAME ENGINE
// ------------------------------------------------------------------------- //
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// ========================================================================= //

#pragma once

#include "classes/node.h"
#include "core/mathematics/math_2d.h"
#include "core/color.h"

class RenderingGeometry;

class Interface : public Node {
    CLASS_TYPE(Interface, Node);

public:
    enum class LayoutPreset {
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight,
        CenterLeft,
        CenterTop,
        CenterRight,
        CenterBottom,
        Center,
        FullRect
    };

    enum class FocusMode {
        None,
        Click,
        All
    };

    // --- Rectángulo de Posición y Tamaño ---
    void set_position(const Vector2& p_pos);
    Vector2 get_position() const { return position; }

    void set_size(const Vector2& p_size);
    Vector2 get_size() const { return size; }

    void set_rotation(float p_radians);
    float get_rotation() const { return rotation; }

    void set_scale(const Vector2& p_scale);
    Vector2 get_scale() const { return scale; }

    void set_pivot_offset(const Vector2& p_pivot);
    Vector2 get_pivot_offset() const { return pivot_offset; }

    Rect2 get_rect() const { return Rect2(position, size); }
    Rect2 get_global_rect() const;

    // --- Anclas Relativas (0.0 a 1.0 respecto al contenedor padre) ---
    void set_anchor(int p_side, float p_ratio);
    void set_anchors_preset(LayoutPreset p_preset);

    // --- Modulación y Visibilidad ---
    void set_visible(bool p_visible);
    bool is_visible() const { return visible; }
    bool is_visible_in_tree() const;

    void set_modulate(const Color& p_color);
    Color get_modulate() const { return modulate; }
    Color get_global_modulate() const;

    // --- Ciclo de Dibujo de UI ---
    virtual void _draw(RenderingGeometry* p_geometry) {}
    void propagate_draw(RenderingGeometry* p_geometry);
    void queue_redraw() { redraw_requested = true; }

    // --- Detección de Entrada (Mouse / Click) ---
    virtual bool has_point(const Vector2& p_point) const;

    Interface(const String& p_name = "Interface");
    virtual ~Interface() override = default;

private:
    Vector2 position = Vector2(0.0f, 0.0f);
    Vector2 size = Vector2(0.0f, 0.0f);
    Vector2 scale = Vector2(1.0f, 1.0f);
    Vector2 pivot_offset = Vector2(0.0f, 0.0f);
    float rotation = 0.0f;

    // Anclas relativas al rectángulo del padre
    float anchor_left = 0.0f;
    float anchor_top = 0.0f;
    float anchor_right = 0.0f;
    float anchor_bottom = 0.0f;

    // Márgenes/Offsets absolutos en píxeles
    float offset_left = 0.0f;
    float offset_top = 0.0f;
    float offset_right = 0.0f;
    float offset_bottom = 0.0f;

    Color modulate = Color(1.0f, 1.0f, 1.0f, 1.0f);
    bool visible = true;
    bool redraw_requested = true;
    FocusMode focus_mode = FocusMode::None;
};