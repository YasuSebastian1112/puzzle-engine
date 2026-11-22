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
#include "licenses_generated/default_font.h"
#include <cmath>

Button::Button(const String& p_name) : Interface(p_name) {
    set_size(Vector2(120.0f, 36.0f));
}

void Button::_ensure_default_font() const {
    if (!font.is_valid()) {
        const_cast<Button*>(this)->font = FontResource::get_default_font(16);
    }
}

void Button::set_text(const String& p_text) {
    if (text != p_text) {
        text = p_text;
        queue_redraw();
    }
}

void Button::set_font(const Ref<FontResource>& p_font) {
    if (font != p_font) {
        font = p_font;
        queue_redraw();
    }
}

void Button::set_disabled(bool p_disabled) {
    if (disabled != p_disabled) {
        disabled = p_disabled;
        queue_redraw();
    }
}

Vector2 Button::get_minimum_size() const {
    _ensure_default_font();
    if (font.is_valid() && !text.empty()) {
        Vector2 text_sz = font->get_string_size(text);
        return Vector2(text_sz.x + 24.0f, text_sz.y + 12.0f);
    }
    return Vector2(40.0f, 28.0f);
}

void Button::_process(float p_delta) {
    (void)p_delta;

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
    Color mod = get_global_modulate();
    Color active_bg = color_normal;

    switch (current_draw_mode) {
        case DrawMode::Hover:    active_bg = color_hover; break;
        case DrawMode::Pressed:  active_bg = color_pressed; break;
        case DrawMode::Disabled: active_bg = color_disabled; break;
        default: break;
    }

    // 1. Fondo y borde del botón
    p_geometry->draw_rect(rect, active_bg * mod, true);
    p_geometry->draw_rect(rect, color_border * mod, false, 1.0f);

    // 2. Texto centrado dentro del rectángulo
    if (!text.empty()) {
        _ensure_default_font();
        if (font.is_valid()) {
            Vector2 text_sz = font->get_string_size(text);
            float press_offset = (current_draw_mode == DrawMode::Pressed) ? 1.0f : 0.0f;

            Vector2 text_pos(
                std::floor(rect.position.x + (rect.size.x - text_sz.x) * 0.5f),
                std::floor(rect.position.y + (rect.size.y - text_sz.y) * 0.5f + press_offset)
            );

            Color active_text = (current_draw_mode == DrawMode::Disabled)
                ? Color(color_text.r, color_text.g, color_text.b, color_text.a * 0.45f)
                : color_text;

            font->draw_text(p_geometry, text_pos, text, active_text * mod);
        }
    }
}

void Button::_bind_methods() {
    ClassDB::add_property("text", &Button::set_text, &Button::get_text);
    ClassDB::add_property("disabled", &Button::set_disabled, &Button::is_disabled);

    ClassDB::add_property("font", &Button::set_font, &Button::get_font);

}