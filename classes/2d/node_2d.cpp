// ========================================================================= //
// PUZZLE ENGINE - 2D/3D GAME ENGINE
// ------------------------------------------------------------------------- //
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// ========================================================================= //

#include "classes/2d/node_2d.h"
#include "core/mathematics/math_functions.h"
#include "core/class_db.h"

Node2D::Node2D(const String& node_name)
    : Spatial2D(node_name), position(0.0f, 0.0f), rotation(0.0f), scale(1.0f, 1.0f), transform_dirty(true) {
}

void Node2D::set_position(const Vector2& p_pos) {
    if (position != p_pos) {
        position = p_pos;
        transform_dirty = true;
        // Invalida la jerarquía descendente para que los hijos recalculen la matriz global
        _propagate_transform_changed();
        queue_redraw();
    }
}

void Node2D::set_rotation(float p_radians) {
    if (rotation != p_radians) {
        rotation = p_radians;
        transform_dirty = true;
        // Invalida la jerarquía descendente para que los hijos recalculen la rotación global
        _propagate_transform_changed();
        queue_redraw();
    }
}

// Conversión directa Grados <-> Radianes usando Math_PI
void Node2D::set_rotation_degrees(float p_degrees) {
    set_rotation(p_degrees * (MathDefs::PI / 180.0f));
}

float Node2D::get_rotation_degrees() const {
    return rotation * (180.0f / MathDefs::PI);
}

void Node2D::set_scale(const Vector2& p_scale) {
    if (scale != p_scale) {
        scale = p_scale;
        transform_dirty = true;
        _propagate_transform_changed();
        queue_redraw();
    }
}

void Node2D::translate(const Vector2& p_offset) {
    set_position(position + p_offset);
}

void Node2D::rotate(float p_radians) {
    set_rotation(rotation + p_radians);
}

void Node2D::_update_transform() const {
    local_transform.set_rotation_and_scale(rotation, scale);
    local_transform.set_origin(position);
    transform_dirty = false;
}

// Notifica en cascada a todos los nodos hijos Node2D que deben refrescar su matriz
void Node2D::_propagate_transform_changed() {
    for (size_t i = 0; i < get_child_count(); ++i) {
        Node2D* child_node = Object::cast_to<Node2D>(get_child(i));
        if (child_node) {
            child_node->transform_dirty = true;
            child_node->_propagate_transform_changed();
        }
    }
}

Transform2D Node2D::get_transform() const {
    if (transform_dirty) {
        _update_transform();
    }
    return local_transform;
}

Vector2 Node2D::get_global_position() const {
    return get_global_transform().get_origin();
}

void Node2D::set_global_position(const Vector2& p_global_pos) {
    const Node2D* parent_node = Object::cast_to<Node2D>(get_parent());
    if (parent_node) {
        // Multiplicamos por la inversa del padre para obtener la posición relativa local
        set_position(parent_node->get_global_transform().affine_inverse().xform(p_global_pos));
    } else {
        set_position(p_global_pos);
    }
}

float Node2D::get_global_rotation() const {
    return get_global_transform().get_rotation();
}

Vector2 Node2D::get_global_scale() const {
    return get_global_transform().get_scale();
}

Vector2 Node2D::to_local(const Vector2& p_global_point) const {
    return get_global_transform().affine_inverse().xform(p_global_point);
}

Vector2 Node2D::to_global(const Vector2& p_local_point) const {
    return get_global_transform().xform(p_local_point);
}

void Node2D::look_at(const Vector2& p_target) {
    Vector2 diff = p_target - get_global_position();
    set_rotation(Math::atan2(diff.y, diff.x));
}

// -----------------------------------------------------------------------------
// VINCULACIÓN AL SISTEMA DE REFLEXIÓN (ClassDB)
// -----------------------------------------------------------------------------
void Node2D::_bind_methods() {
    // Transformación Local
    ClassDB::add_property("position",         &Node2D::set_position,         &Node2D::get_position);
    ClassDB::add_property("rotation",         &Node2D::set_rotation,         &Node2D::get_rotation);
    // Propiedad expuesta para operar con grados sexagesimales en consola ($Node2D/Sprite2D rotation_degrees 90)
    ClassDB::add_property("rotation_degrees", &Node2D::set_rotation_degrees, &Node2D::get_rotation_degrees);
    ClassDB::add_property("scale",            &Node2D::set_scale,            &Node2D::get_scale);

    // Transformación Global
    ClassDB::add_property("global_position",  &Node2D::set_global_position,  &Node2D::get_global_position);
}