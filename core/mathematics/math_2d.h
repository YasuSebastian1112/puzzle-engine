// ========================================================================= //
// Puzzle Engine - 2D/3D Hybrid Game Engine
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// ========================================================================= //

#include "math_functions.h"

struct Vector2 {
    
    // Ejes
    union {
        float x;
        float width;
    };
    
    union {
        float y;
        float height;
    };
    
    // Acceder como si fuesen indices
    _FORCE_INLINE_ float& operator[](int p_idx) {return p_idx?y:x;}
    _FORCE_INLINE_ const float& operator[](int p_idx) const {return p_idx?y:x;}


    void normalize();
    Vector2 normalized() const;

    float lenght() const;
    float lenght_squared() const;

    float get_distance_to(const Vector2& p_vec2) const;
    float get_distance_squared_to(const Vector2& p_vec2) const;
    float get_angle_to(const Vector2& p_vec2) const;
    float get_angle_to_point(const Vector2& p_vec2) const;

    // Punto y escalar 2D
    float dot(const Vector2& p_other) const;
    float cross(const Vector2& p_other) const;

    // Angulo y rotacion
    float angle() const;
    Vector2 rotated(float p_radians) const;
    Vector2 orthogonal() const;

    // Rebotes y reflexiones contra normales de colision
    Vector2 reflect(const Vector2& p_normal) const;
    Vector2 bounce(const Vector2& p_normal) const;

    // Constantes
    static const Vector2 ZERO;


    // Operadores aritmeticos de vector-vector
    Vector2 operator+(const Vector2& p_v) const;
    Vector2 operator-(const Vector2& p_v) const;
    Vector2 operator*(const Vector2& p_v) const;
    Vector2 operator/(const Vector2& p_v) const;
    Vector2 operator-(const Vector2& p_v) const;
    
    // Operadores aritmeticos vector-escalar
    Vector2 operator*(float p_scalar) const;
    Vector2 operator/(float p_scalar) const;

    // Asignacion compuesta
    Vector2 operator+=(const Vector2& p_v);
    Vector2 operator-=(const Vector2& p_v);
    Vector2 operator*=(const Vector2& p_v);
    Vector2 operator/=(const Vector2& p_v);

    // Comparadores
    bool operator==(const Vector2& p_v);
    bool operator!=(const Vector2& p_v);
    bool operator<(const Vector2& p_v);
    bool operator<=(const Vector2& p_v);
    bool operator>(const Vector2& p_v);
    bool operator>=(const Vector2& p_v);

    // Constructores
    Vector2() : x(0.0f), y(0.0f) {}
    Vector2(float p_x, float p_y) : x(p_x), y(p_y) {}
};
