// ========================================================================= //
// Puzzle Engine - 2D/3D Hybrid Game Engine
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// ========================================================================= //

#include "transform_2d.h"

Transform2D::Transform2D(float p_rotation, const Vector2& p_pos) {
    set_rotation_and_scale(p_rotation, Vector2(1.0f, 1.0f));
    columns[2] = p_pos;
}

void Transform2D::set_rotation_and_scale(float p_rot, const Vector2& p_scale) {
    float cr = Math::cos(p_rot);
    float sr = Math::sin(p_rot);
    columns[0] = Vector2(cr, sr) * p_scale.x;
    columns[1] = Vector2(-sr, cr) * p_scale.y;
}

float Transform2D::get_rotation() const {
    return Math::atan2(columns[0].y, columns[0].x);
}

Vector2 Transform2D::get_scale() const {
    float det_sign = (determinant() < 0.0f) ? -1.0f : 1.0f;
    return Vector2(
        columns[0].length(),
        columns[1].length() * det_sign
    );
}

Transform2D Transform2D::affine_inverse() const {
    float det = determinant();
    if (Math::abs(det) < 0.00001f) {
        return Transform2D();
    }

    float inv_det = 1.0f / det;
    Transform2D inv;
    inv.columns[0].x = columns[1].y * inv_det;
    inv.columns[0].y = -columns[0].y * inv_det;
    inv.columns[1].x = -columns[1].x * inv_det;
    inv.columns[1].y = columns[0].x * inv_det;

    inv.columns[2] = inv.basis_xform(-columns[2]);
    return inv;
}

Transform2D Transform2D::operator*(const Transform2D& p_other) const {
    Transform2D result;
    result.columns[0] = basis_xform(p_other.columns[0]);
    result.columns[1] = basis_xform(p_other.columns[1]);
    result.columns[2] = xform(p_other.columns[2]);
    return result;
}