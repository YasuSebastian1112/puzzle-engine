// ========================================================================= //
// PUZZLE ENGINE - 2D/3D GAME ENGINE
// ------------------------------------------------------------------------- //
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// ========================================================================= //

#pragma once

#include "classes/interface/interface.h"
#include <functional>

class Button : public Interface {
    CLASS_TYPE(Button, Interface);

public:
    enum class DrawMode {
        Normal,
        Hover,
        Pressed,
        Disabled
    };

    void set_text(const String& p_text);
    String get_text() const { return text; }

    void set_disabled(bool p_disabled);
    bool is_disabled() const { return disabled; }

    DrawMode get_draw_mode() const { return current_draw_mode; }

    // Callbacks estilo señal
    void set_on_pressed(std::function<void()> p_callback) { on_pressed_callback = p_callback; }

    // Procesamiento por frame para interactividad del cursor
    virtual void _process(float p_delta) override;
    virtual void _draw(RenderingGeometry* p_geometry) override;

    Button(const String& p_name = "Button");
    virtual ~Button() override = default;

private:
    String text = "Button";
    bool disabled = false;
    bool is_hovered = false;
    bool is_pressed = false;

    DrawMode current_draw_mode = DrawMode::Normal;
    std::function<void()> on_pressed_callback;

    // Colores de estado por defecto basados en la paleta del motor
    Color color_normal   = Color(0.18f, 0.20f, 0.25f, 1.0f);
    Color color_hover    = Color(0.24f, 0.28f, 0.35f, 1.0f);
    Color color_pressed  = Color(0.12f, 0.50f, 0.85f, 1.0f);
    Color color_disabled = Color(0.12f, 0.12f, 0.15f, 0.6f);
};