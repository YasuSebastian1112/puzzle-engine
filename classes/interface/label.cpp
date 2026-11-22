// ========================================================================= //
// PUZZLE ENGINE - 2D/3D GAME ENGINE
// ------------------------------------------------------------------------- //
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// ========================================================================= //

#include "classes/interface/label.h"
#include "globals/rendering_global/device/rendering_geometry.h"
#include "licenses_generated/default_font.h"
#include "core/class_db.h"
#include <cmath>

Label::Label(const String& p_name)
    : Interface(p_name) {
}

void Label::_ensure_default_font() const {
    if (!font.is_valid()) {
        const_cast<Label*>(this)->font = FontResource::get_default_font(16);
    }
}

void Label::set_text(const String& p_text) {
    if (text != p_text) {
        text = p_text;
        queue_redraw();
    }
}

void Label::set_font(const Ref<FontResource>& p_font) {
    if (font != p_font) {
        font = p_font;
        queue_redraw();
    }
}

void Label::set_font_color(const Color& p_color) {
    font_color = p_color;
    queue_redraw();
}

void Label::set_horizontal_alignment(HorizontalAlignment p_alignment) {
    if (h_align != p_alignment) {
        h_align = p_alignment;
        queue_redraw();
    }
}

void Label::set_vertical_alignment(VerticalAlignment p_alignment) {
    if (v_align != p_alignment) {
        v_align = p_alignment;
        queue_redraw();
    }
}

Vector2 Label::get_minimum_size() const {
    _ensure_default_font();
    if (font.is_valid() && !text.empty()) {
        return font->get_string_size(text);
    }
    return Vector2(0.0f, 0.0f);
}

void Label::_draw(RenderingGeometry* p_geometry) {
    if (text.empty() || !p_geometry) return;

    _ensure_default_font();
    if (!font.is_valid()) return;

    Rect2 global_rect = get_global_rect();
    Vector2 text_size = font->get_string_size(text);
    Vector2 draw_pos = global_rect.position;

    switch (h_align) {
        case ALIGN_CENTER: draw_pos.x += (global_rect.size.x - text_size.x) * 0.5f; break;
        case ALIGN_RIGHT:  draw_pos.x += global_rect.size.x - text_size.x; break;
        case ALIGN_LEFT:
        default: break;
    }

    switch (v_align) {
        case VALIGN_CENTER: draw_pos.y += (global_rect.size.y - text_size.y) * 0.5f; break;
        case VALIGN_BOTTOM: draw_pos.y += global_rect.size.y - text_size.y; break;
        case VALIGN_TOP:
        default: break;
    }

    // Alinear a píxeles enteros para evitar desenfoque en glifos
    draw_pos.x = std::floor(draw_pos.x);
    draw_pos.y = std::floor(draw_pos.y);

    font->draw_text(p_geometry, draw_pos, text, font_color * get_global_modulate());
}

void Label::_bind_methods() {
    ClassDB::add_property("text", &Label::set_text, &Label::get_text);
    ClassDB::add_property("font_color", &Label::set_font_color, &Label::get_font_color);

    ClassDB::add_property("font", &Label::set_font, &Label::get_font);
}