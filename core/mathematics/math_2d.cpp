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

/*Vector2i*/

// Constructor de conversion

Vector2i::Vector2i(const Vector2& p_vec2) {
    x = static_cast<int32_t>(p_vec2.x);
    y = static_cast<int32_t>(p_vec2.y);
}

void Vector2i::normalize() {
    float l = length();
    if (!Math::is_zero_approx(l)) {
        x = static_cast<int32_t>(Math::round(static_cast<float>(x) / l));
        y = static_cast<int32_t>(Math::round(static_cast<float>(y) / l));
    }
}

Vector2i Vector2i::normalized() const {
    Vector2i v = *this;
    v.normalize();
    return v;
}

float Vector2i::length() const {
    return Math::sqrt(static_cast<float>(length_squared()));
}

int64_t Vector2i::length_squared() const {
    return static_cast<int64_t>(x) * x + static_cast<int64_t>(y) * y;
}

float Vector2i::get_distance_to(const Vector2i& p_vec2) const {
    return (*this - p_vec2).length();
}

int64_t Vector2i::get_distance_squared_to(const Vector2i& p_vec2) const {
    return (*this - p_vec2).length_squared();
}

float Vector2i::get_angle_to(const Vector2i& p_vec2) const {
    return Math::atan2(static_cast<float>(cross(p_vec2)), static_cast<float>(dot(p_vec2)));
}

float Vector2i::get_angle_to_point(const Vector2i& p_vec2) const {
    return (p_vec2 - *this).angle();
}

// Vector2i: Punto y escalar

int64_t Vector2i::dot(const Vector2i& p_other) const {
    return static_cast<int64_t>(x) * p_other.x + static_cast<int64_t>(y) * p_other.y;
}

int64_t Vector2i::cross(const Vector2i& p_other) const {
    return static_cast<int64_t>(x) * p_other.y - static_cast<int64_t>(y) * p_other.x;
}

Vector2i Vector2i::cross(int32_t p_other) const {
    return Vector2i(p_other * y, -p_other * x);
}

// Vector2i: Angulos y rotacion

float Vector2i::angle() const {
    return Math::atan2(static_cast<float>(y), static_cast<float>(x));
}

Vector2i Vector2i::rotated(float p_radians) const {
    float sine = Math::sin(p_radians);
    float cosi = Math::cos(p_radians);
    return Vector2i(
        static_cast<int32_t>(Math::round(static_cast<float>(x) * cosi - static_cast<float>(y) * sine)),
        static_cast<int32_t>(Math::round(static_cast<float>(x) * sine + static_cast<float>(y) * cosi))
    );
}

Vector2i Vector2i::tangent() const {
    return Vector2i(y, -x);
}

Vector2i Vector2i::orthogonal() const {
    return Vector2i(-y, x);
}

float Vector2i::atan2() const {
    return Math::atan2(static_cast<float>(y), static_cast<float>(x));
}

// Vector2i: Rebotes y reflexiones contra normales de colision

Vector2i Vector2i::reflect(const Vector2i& p_normal) const {
    int64_t d = dot(p_normal);
    return *this - p_normal * static_cast<int32_t>(2 * d);
}

Vector2i Vector2i::bounce(const Vector2i& p_normal) const {
    return -reflect(p_normal);
}

// Vector2i: Operadores aritmeticos vector-vector

Vector2i Vector2i::operator+(const Vector2i& p_v) const {
    return Vector2i(x + p_v.x, y + p_v.y);
}

Vector2i Vector2i::operator-(const Vector2i& p_v) const {
    return Vector2i(x - p_v.x, y - p_v.y);
}

Vector2i Vector2i::operator-() const {
    return Vector2i(-x, -y);
}

Vector2i Vector2i::operator*(const Vector2i& p_v) const {
    return Vector2i(x * p_v.x, y * p_v.y);
}

Vector2i Vector2i::operator/(const Vector2i& p_v) const {
    return Vector2i(x / p_v.x, y / p_v.y);
}

// Vector2i: Operadores aritmeticos vector-escalar

Vector2i Vector2i::operator*(int32_t p_scalar) const {
    return Vector2i(x * p_scalar, y * p_scalar);
}

Vector2i Vector2i::operator/(int32_t p_scalar) const {
    return Vector2i(x / p_scalar, y / p_scalar);
}

// Vector2i: Asignacion Compuesta

Vector2i& Vector2i::operator+=(const Vector2i& p_v) {
    x += p_v.x;
    y += p_v.y;
    return *this;
}

Vector2i& Vector2i::operator-=(const Vector2i& p_v) {
    x -= p_v.x;
    y -= p_v.y;
    return *this;
}

Vector2i& Vector2i::operator*=(const Vector2i& p_v) {
    x *= p_v.x;
    y *= p_v.y;
    return *this;
}

Vector2i& Vector2i::operator/=(const Vector2i& p_v) {
    x /= p_v.x;
    y /= p_v.y;
    return *this;
}

Vector2i& Vector2i::operator*=(int32_t p_scalar) {
    x *= p_scalar;
    y *= p_scalar;
    return *this;
}

Vector2i& Vector2i::operator/=(int32_t p_scalar) {
    x /= p_scalar;
    y /= p_scalar;
    return *this;
}

// Vector2i: Comparadores

bool Vector2i::operator==(const Vector2i& p_v) const {
    return (x == p_v.x) && (y == p_v.y);
}

bool Vector2i::operator!=(const Vector2i& p_v) const {
    return !(*this == p_v);
}

bool Vector2i::operator<(const Vector2i& p_v) const {
    return (x == p_v.x) ? (y < p_v.y) : (x < p_v.x);
}

bool Vector2i::operator<=(const Vector2i& p_v) const {
    return (x == p_v.x) ? (y <= p_v.y) : (x < p_v.x);
}

bool Vector2i::operator>(const Vector2i& p_v) const {
    return (x == p_v.x) ? (y > p_v.y) : (x > p_v.x);
}

bool Vector2i::operator>=(const Vector2i& p_v) const {
    return (x == p_v.x) ? (y >= p_v.y) : (x > p_v.x);
}