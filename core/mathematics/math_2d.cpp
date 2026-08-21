// ========================================================================= //
// Puzzle Engine - 2D/3D Hybrid Game Engine
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// ===============================================

#include "math_2d.h"

/*Vector2*/

void Vector2::normalize() {
    float lenght = x*x + y*y;
    if (lenght != 0) {
        lenght = Math::sqrt(lenght);
        x /= lenght;
        y /= lenght;
    }
}

Vector2 Vector2::normalized() const {
    Vector2 vec = *this;
    vec.normalize();
    return vec;
}

float Vector2::length() const {
    return Math::sqrt(x*x + y*y);
}

float Vector2::length_squared() const {
    return x*x + y*y;
}

float Vector2::get_distance_to(const Vector2& p_vec2) const {
    return Math::sqrt((x - p_vec2.x) * (x - p_vec2.x) + (y - p_vec2.y) * (y - p_vec2.y));
}

float Vector2::get_distance_squared_to(const Vector2& p_vec2) const {
    return (x - p_vec2.x) * (x - p_vec2.x) + (y - p_vec2.y) * (y - p_vec2.y);
}

float Vector2::get_angle_to(const Vector2& p_vec2) const {
    return Math::atan2(Vector2::tangent().dot(p_vec2), Vector2::dot(p_vec2));
}

float Vector2::get_angle_to_point(const Vector2& p_vec2) const {
    return Math::atan2(x- p_vec2.x, y - p_vec2.y);
}

// Vector2: Punto y escalar

float Vector2::dot(const Vector2& p_other) const {
    return x * p_other.x + y * p_other.y;
}

float Vector2::cross(const Vector2& p_other) const {
    return x * p_other.y - y * p_other.x;
}

Vector2 Vector2::cross(float p_other) const {
    return Vector2(p_other * y, -p_other * x);
}

// Vector2: Angulos y rotacion



Vector2 Vector2::tangent() const {
    return Vector2(y, -x);
}