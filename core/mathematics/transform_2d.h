// ========================================================================= //
// Puzzle Engine - 2D/3D Hybrid Game Engine
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// ========================================================================= //


#pragma once
#include "core/mathematics/math_2d.h"
#include "core/mathematics/math_functions.h"

struct Transform2D {
    Vector2 columns[3]; // columns[0] = Eje X, columns[1] = Eje Y, columns[2] = Origen

    // Constructores e inlines directos
    constexpr Transform2D() : columns{
        Vector2(1.0f, 0.0f),
        Vector2(0.0f, 1.0f),
        Vector2(0.0f, 0.0f)
    } {}

    constexpr Transform2D(const Vector2& p_x, const Vector2& p_y, const Vector2& p_origin) : columns{
        p_x, p_y, p_origin
    } {}

    Transform2D(float p_rotation, const Vector2& p_pos);

    // Getters y Setters
    Vector2& get_x() { return columns[0]; }
    const Vector2& get_x() const { return columns[0]; }

    Vector2& get_y() { return columns[1]; }
    const Vector2& get_y() const { return columns[1]; }

    Vector2& get_origin() { return columns[2]; }
    const Vector2& get_origin() const { return columns[2]; }

    void set_origin(const Vector2& p_origin) { columns[2] = p_origin; }

    // Transformación rápida de puntos
    Vector2 xform(const Vector2& p_point) const {
        return columns[2] + columns[0] * p_point.x + columns[1] * p_point.y;
    }

    Vector2 basis_xform(const Vector2& p_vec) const {
        return columns[0] * p_vec.x + columns[1] * p_vec.y;
    }

    float determinant() const {
        return columns[0].x * columns[1].y - columns[0].y * columns[1].x;
    }

    Vector2 operator*(const Vector2& p_point) const {
        return xform(p_point);
    }

    // Métodos complejos implementados en transform_2d.cpp
    void set_rotation_and_scale(float p_rot, const Vector2& p_scale);
    float get_rotation() const;
    Vector2 get_scale() const;
    Transform2D affine_inverse() const;
    Transform2D operator*(const Transform2D& p_other) const;

    // Direcciones canónicas locales en el espacio global (los versores i y j como en fisica)
    Vector2 get_i() const { return columns[0]; }
    Vector2 get_j() const { return columns[1]; }

    // Versores normalizados (dirección pura sin escala)
    Vector2 get_forward() const { return columns[0].normalized(); }
    Vector2 get_up() const { return columns[1].normalized(); }

};