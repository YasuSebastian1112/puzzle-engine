// ========================================================================= //
// Puzzle Engine - 2D/3D Hybrid Game Engine
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// ========================================================================= //

#pragma once

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

    
    void normalize(); // Convertir el vector en algo unitario
    Vector2 normalized() const; // Devuelve el vector de manera unitaria sin modificarla

    float length() const; // Calcula la magnitud/longitud total del vector
    float length_squared() const; // Retorna la logitud al cuadrado

    float get_distance_to(const Vector2& p_vec2) const; // Obtiene la distancia directa de un punto a otro
    float get_distance_squared_to(const Vector2& p_vec2) const; // Obtiene la distancia cuadrada directa de un punto a otro
    float get_angle_to(const Vector2& p_vec2) const; // Calcula el angulo relativo de diferencia de este vector a otro
    float get_angle_to_point(const Vector2& p_vec2) const; // Devuelve el angulo hacia una posicion destino

    // Punto y escalar 2D
    float dot(const Vector2& p_other) const; // Mide la alineacion de dos vectores
    float cross(const Vector2& p_other) const; // Devuelve un escalar que indica si otro vector esta a la izquierda o derecha
    Vector2 cross(float p_other) const; // Producto cruz entre un vector2D y un escalar; rota el vector 90° escalado

    // Angulos y rotacion
    float angle() const; // Retorna el angulo en radianes respecto al eje positivo X
    Vector2 rotated(float p_radians) const; // Devuelve una copia del vector girada en el angulo especificado
    Vector2 tangent() const; // Devuelve un vector perpendicular a tangente a la direccion actual
    Vector2 orthogonal() const; // Devuelve un vector perpendicular a 90°
    float atan2() const; // Obtiene el angulo del vector

    // Rebotes y reflexiones contra normales de colision
    Vector2 reflect(const Vector2& p_normal) const; // Calcula la direccion reflejada
    Vector2 bounce(const Vector2& p_normal) const; // Calcula la velocidad resultante de un rebote

    // Constantes
    static const Vector2 ZERO;

    // Operadores aritmeticos de vector-vector
    Vector2 operator+(const Vector2& p_v) const;
    Vector2 operator-(const Vector2& p_v) const;
    Vector2 operator-() const;
    Vector2 operator*(const Vector2& p_v) const;
    Vector2 operator/(const Vector2& p_v) const;

    // Operadores aritmeticos vector-escalar
    Vector2 operator*(float p_scalar) const;
    Vector2 operator/(float p_scalar) const;

    // Asignacion compuesta
    Vector2& operator+=(const Vector2& p_v);
    Vector2& operator-=(const Vector2& p_v);
    Vector2& operator*=(const Vector2& p_v);
    Vector2& operator/=(const Vector2& p_v);
    Vector2& operator*=(float p_scalar);
    Vector2& operator/=(float p_scalar);

    // Comparadores
    bool operator==(const Vector2& p_v) const;
    bool operator!=(const Vector2& p_v) const;
    bool operator<(const Vector2& p_v) const;
    bool operator<=(const Vector2& p_v) const;
    bool operator>(const Vector2& p_v) const;
    bool operator>=(const Vector2& p_v) const;

    // Constructores
    Vector2() : x(0.0f), y(0.0f) {}
    Vector2(float p_x, float p_y) : x(p_x), y(p_y) {}
};

inline Vector2 operator*(float p_scalar, const Vector2 &p_v) { // return p_scalar * p_v
    return p_v * p_scalar;
}

using Point2 = Vector2;
using Size = Vector2;