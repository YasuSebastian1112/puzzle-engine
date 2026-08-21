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

float Vector2::angle() const {
    return Math::atan2(y, x);
}

Vector2 Vector2::rotated(float p_radians) const {
    float sine = Math::sin(p_radians);
    float cosi = Math::sin(p_radians);
    return Vector2(x * cosi - y * sine, x * sine + y * cosi);
}

Vector2 Vector2::tangent() const {
    return Vector2(y, -x);
}

Vector2 Vector2::orthogonal() const {
    return Vector2(-y, x);
}

float Vector2::atan2() const {
    return Math::atan2(x, y);
}

// Vector2: Rebotes y reflexiones contra normales de colision

Vector2 Vector2::reflect(const Vector2& p_normal) const {
    return *this - p_normal * (2.0f * this->dot(p_normal));
}

Vector2 Vector2::bounce(const Vector2& p_normal) const {
    return -reflect(p_normal);
}

// Vector2: Operadores aritmeticos vector-vector

Vector2 Vector2::operator+(const Vector2& p_v) const {
    return Vector2(x + p_v.x, y + p_v.y);
}

Vector2 Vector2::operator-(const Vector2& p_v) const {
    return Vector2(x - p_v.x, y - p_v.y);
}

Vector2 Vector2::operator-() const {
    return Vector2(-x, -y);
}

Vector2 Vector2::operator*(const Vector2& p_v) const {
    return Vector2(x * p_v.x, y * p_v.y);
}

Vector2 Vector2::operator/(const Vector2& p_v) const {
    return Vector2(x / p_v.x, y / p_v.y);
}

// Vector2: Operadores aritmeticos vector-escalar

Vector2 Vector2::operator*(float p_scalar) const {
    return Vector2(x * p_scalar, y * p_scalar);
}

Vector2 Vector2::operator/(float p_scalar) const {
    return Vector2(x / p_scalar, y / p_scalar);
}

// Vector2: Asignacion Compuesta

Vector2& Vector2::operator+=(const Vector2& p_v) {
    x += p_v.x;
    y += p_v.y;
    return *this;
}

Vector2& Vector2::operator-=(const Vector2& p_v) {
    x -= p_v.x;
    y -= p_v.y;
    return *this;
}

Vector2& Vector2::operator*=(const Vector2& p_v) {
    x *= p_v.x;
    y *= p_v.y;
    return *this;
}

Vector2& Vector2::operator/=(const Vector2& p_v) {
    x /= p_v.x;
    y /= p_v.y;
    return *this;
}

Vector2& Vector2::operator*=(float p_scalar) {
    x *= p_scalar;
    y *= p_scalar;
    return *this;
}

Vector2& Vector2::operator/=(float p_scalar) {
    x /= p_scalar;
    y /= p_scalar;
    return *this;
}

// Vector2: Comparadores

bool Vector2::operator==(const Vector2 &p_v) const {
    return (x == p_v.x) && (y == p_v.y);
}

bool Vector2::operator!=(const Vector2 &p_v) const {
    return !(*this == p_v);
}

bool Vector2::operator<(const Vector2 &p_v) const {
    return (x == p_v.x) ? (y < p_v.y) : (x < p_v.x);
}

bool Vector2::operator<=(const Vector2 &p_v) const {
    return (x == p_v.x) ? (y <= p_v.y) : (x < p_v.x);
}

bool Vector2::operator>(const Vector2 &p_v) const {
    return (x == p_v.x) ? (y > p_v.y) : (x > p_v.x);
}

bool Vector2::operator>=(const Vector2 &p_v) const {
    return (x == p_v.x) ? (y >= p_v.y) : (x > p_v.x);
}