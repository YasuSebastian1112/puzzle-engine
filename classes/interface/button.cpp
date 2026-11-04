// ========================================================================= //
// PUZZLE ENGINE - 2D/3D GAME ENGINE
// ------------------------------------------------------------------------- //
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// ========================================================================= //

#include "classes/interface/button.h"
#include "globals/input/input.h"
#include "core/class_db.h"
#include "globals/rendering_global/device/rendering_geometry.h"

Button::Button(const String& p_name) : Interface(p_name) {
    set_size(Vector2(120.0f, 36.0f));
}

void Button::set_text(const String& p_text) {
    text = p_text;
    queue_redraw();
}

void Button::set_disabled(bool p_disabled) {
    disabled = p_disabled;
    queue_redraw();
}

void Button::_process(float p_delta) {
    if (disabled || !is_visible_in_tree()) {
        current_draw_mode = DrawMode::Disabled;
        return;
    }

    Vector2 mouse_pos = Input::get_mouse_position();
    is_hovered = has_point(mouse_pos);

    if (is_hovered) {
        if (Input::is_mouse_button_pressed(MouseButton::Left)) {
            current_draw_mode = DrawMode::Pressed;
            is_pressed = true;
        } else {
            // Si el botón se acaba de soltar sobre el área del botón, se dispara el evento
            if (is_pressed && Input::is_mouse_button_just_released(MouseButton::Left)) {
                if (on_pressed_callback) {
                    on_pressed_callback();
                }
            }
            is_pressed = false;
            current_draw_mode = DrawMode::Hover;
        }
    } else {
        is_pressed = false;
        current_draw_mode = DrawMode::Normal;
    }
}

void Button::_draw(RenderingGeometry* p_geometry) {
    if (!p_geometry || !is_visible_in_tree()) return;

    Rect2 rect = get_global_rect();
    Color active_color = color_normal;

    switch (current_draw_mode) {
        case DrawMode::Hover:    active_color = color_hover; break;
        case DrawMode::Pressed:  active_color = color_pressed; break;
        case DrawMode::Disabled: active_color = color_disabled; break;
        default: break;
    }

    // Dibujar fondo rectangular del botón modulado por el color global del árbol
    p_geometry->draw_rect(rect, active_color * get_global_modulate());
}

// -----------------------------------------------------------------------------
// VINCULACIÓN AL SISTEMA DE REFLEXIÓN (ClassDB)
// -----------------------------------------------------------------------------
void Button::_bind_methods() {
    ClassDB::add_property("text",     &Button::set_text,     &Button::get_text);
    ClassDB::add_property("disabled", &Button::set_disabled, &Button::is_disabled);
}