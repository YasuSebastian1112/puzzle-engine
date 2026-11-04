// ========================================================================= //
// PUZZLE ENGINE - 2D/3D GAME ENGINE
// ------------------------------------------------------------------------- //
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// ========================================================================= //

#include "classes/interface/interface.h"
#include "core/class_db.h"
#include "globals/rendering_global/device/rendering_geometry.h"

Interface::Interface(const String& p_name)
    : Node(p_name), position(0.0f, 0.0f), size(40.0f, 40.0f) {
}

void Interface::set_position(const Vector2& p_pos) {
    if (position != p_pos) {
        position = p_pos;
        queue_redraw();
    }
}

void Interface::set_size(const Vector2& p_size) {
    if (size != p_size) {
        size = p_size;
        queue_redraw();
    }
}

void Interface::set_rotation(float p_radians) {
    if (rotation != p_radians) {
        rotation = p_radians;
        queue_redraw();
    }
}

void Interface::set_scale(const Vector2& p_scale) {
    if (scale != p_scale) {
        scale = p_scale;
        queue_redraw();
    }
}

void Interface::set_pivot_offset(const Vector2& p_pivot) {
    pivot_offset = p_pivot;
    queue_redraw();
}

void Interface::set_visible(bool p_visible) {
    visible = p_visible;
}

bool Interface::is_visible_in_tree() const {
    if (!visible) return false;
    const Interface* parent_ui = Object::cast_to<Interface>(get_parent());
    if (parent_ui) {
        return parent_ui->is_visible_in_tree();
    }
    return true;
}

void Interface::set_modulate(const Color& p_color) {
    modulate = p_color;
    queue_redraw();
}

Color Interface::get_global_modulate() const {
    const Interface* parent_ui = Object::cast_to<Interface>(get_parent());
    if (parent_ui) {
        return parent_ui->get_global_modulate() * modulate;
    }
    return modulate;
}

Rect2 Interface::get_global_rect() const {
    Vector2 global_pos = position;
    const Interface* parent_ui = Object::cast_to<Interface>(get_parent());
    if (parent_ui) {
        global_pos = global_pos + parent_ui->get_global_rect().position;
    }
    return Rect2(global_pos, size);
}

bool Interface::has_point(const Vector2& p_point) const {
    return get_global_rect().has_point(p_point);
}

void Interface::set_anchors_preset(LayoutPreset p_preset) {
    switch (p_preset) {
        case LayoutPreset::TopLeft:
            anchor_left = 0.0f; anchor_top = 0.0f; anchor_right = 0.0f; anchor_bottom = 0.0f;
            break;
        case LayoutPreset::TopRight:
            anchor_left = 1.0f; anchor_top = 0.0f; anchor_right = 1.0f; anchor_bottom = 0.0f;
            break;
        case LayoutPreset::BottomLeft:
            anchor_left = 0.0f; anchor_top = 1.0f; anchor_right = 0.0f; anchor_bottom = 1.0f;
            break;
        case LayoutPreset::BottomRight:
            anchor_left = 1.0f; anchor_top = 1.0f; anchor_right = 1.0f; anchor_bottom = 1.0f;
            break;
        case LayoutPreset::Center:
            anchor_left = 0.5f; anchor_top = 0.5f; anchor_right = 0.5f; anchor_bottom = 0.5f;
            break;
        case LayoutPreset::FullRect:
            anchor_left = 0.0f; anchor_top = 0.0f; anchor_right = 1.0f; anchor_bottom = 1.0f;
            break;
        default:
            break;
    }
    queue_redraw();
}

void Interface::propagate_draw(RenderingGeometry* p_geometry) {
    if (!is_visible_in_tree() || !p_geometry) return;

    _draw(p_geometry);

    for (size_t i = 0; i < get_child_count(); ++i) {
        Node* child = get_child(i);
        Interface* child_ui = Object::cast_to<Interface>(child);
        if (child_ui) {
            child_ui->propagate_draw(p_geometry);
        }
    }
}

// -----------------------------------------------------------------------------
// VINCULACIÓN AL SISTEMA DE REFLEXIÓN (ClassDB)
// -----------------------------------------------------------------------------
void Interface::_bind_methods() {
    ClassDB::add_property("position",     &Interface::set_position,     &Interface::get_position);
    ClassDB::add_property("size",         &Interface::set_size,         &Interface::get_size);
    ClassDB::add_property("scale",        &Interface::set_scale,        &Interface::get_scale);
    ClassDB::add_property("rotation",     &Interface::set_rotation,     &Interface::get_rotation);
    ClassDB::add_property("pivot_offset", &Interface::set_pivot_offset, &Interface::get_pivot_offset);
    ClassDB::add_property("visible",      &Interface::set_visible,      &Interface::is_visible);
    ClassDB::add_property("modulate",     &Interface::set_modulate,     &Interface::get_modulate);
}