// ========================================================================= //
// Puzzle Engine - 2D/3D Hybrid Game Engine
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// ========================================================================= //

#pragma once

#include <cstdint>
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

// Sobrenombres para Vector2
using Point2 = Vector2;
using Size2 = Vector2;


struct Vector2i {
    // Ejes
    union {
        int32_t x;
        int32_t width;
    };

    union {
        int32_t y;
        int32_t height;
    };

    // Acceder como si fuesen indices
    _FORCE_INLINE_ int32_t& operator[](int p_idx) { return p_idx ? y : x; }
    _FORCE_INLINE_ const int32_t& operator[](int p_idx) const { return p_idx ? y : x; }

    void normalize();
    Vector2i normalized() const;

    // Magnitud y distancias
    float length() const; // Calcula la magnitud total
    int64_t length_squared() const; // Retorna la longitud al cuadrado en 64 bits para evitar overflow

    float get_distance_to(const Vector2i& p_vec2) const; // Distancia euclidiana directa
    int64_t get_distance_squared_to(const Vector2i& p_vec2) const; // Distancia cuadrada directa
    int32_t distance_manhattan_to(const Vector2i& p_vec2) const; // Distancia Manhattan en celdas (|dx| + |dy|)
    float get_angle_to(const Vector2i& p_vec2) const; // Angulo relativo respecto a otro vector
    float get_angle_to_point(const Vector2i& p_vec2) const; // Angulo hacia una celda destino

    // Operaciones vectoriales 2D
    int64_t dot(const Vector2i& p_other) const; // Producto punto
    int64_t cross(const Vector2i& p_other) const; // Pseudo-producto cruz 2D
    Vector2i cross(int32_t p_other) const;

    // Angulos y rotacion
    float angle() const; // Retorna el angulo en radianes
    Vector2i rotated(float p_radians) const;
    Vector2i tangent() const;
    Vector2i orthogonal() const; // Devuelve un vector perpendicular a 90 grados (-y, x)
    float atan2() const;
    Vector2i abs() const; // Devuelve el vector con componentes absolutos
    Vector2i sign() const; // Devuelve la direccion de signos (-1, 0, 1)

    // Rebotes y reflexiones contra normales de colision
    Vector2i reflect(const Vector2i& p_normal) const;
    Vector2i bounce(const Vector2i& p_normal) const;

    // Constantes
    static const Vector2i ZERO;
    static const Vector2i ONE;
    static const Vector2i UP;
    static const Vector2i DOWN;
    static const Vector2i LEFT;
    static const Vector2i RIGHT;

    // Operadores aritmeticos vector-vector
    Vector2i operator+(const Vector2i& p_v) const;
    Vector2i operator-(const Vector2i& p_v) const;
    Vector2i operator-() const;
    Vector2i operator*(const Vector2i& p_v) const;
    Vector2i operator/(const Vector2i& p_v) const;
    Vector2i operator%(const Vector2i& p_v) const;

    // Operadores aritmeticos vector-escalar
    Vector2i operator*(int32_t p_scalar) const;
    Vector2i operator/(int32_t p_scalar) const;
    Vector2i operator%(int32_t p_scalar) const;

    // Asignacion compuesta
    Vector2i& operator+=(const Vector2i& p_v);
    Vector2i& operator-=(const Vector2i& p_v);
    Vector2i& operator*=(const Vector2i& p_v);
    Vector2i& operator/=(const Vector2i& p_v);
    Vector2i& operator%=(const Vector2i& p_v);
    Vector2i& operator*=(int32_t p_scalar);
    Vector2i& operator/=(int32_t p_scalar);
    Vector2i& operator%=(int32_t p_scalar);

    // Comparadores
    bool operator==(const Vector2i& p_v) const;
    bool operator!=(const Vector2i& p_v) const;
    bool operator<(const Vector2i& p_v) const;
    bool operator<=(const Vector2i& p_v) const;
    bool operator>(const Vector2i& p_v) const;
    bool operator>=(const Vector2i& p_v) const;

    // Constructores
    constexpr Vector2i() : x(0), y(0) {}
    constexpr Vector2i(int32_t p_x, int32_t p_y) : x(p_x), y(p_y) {}
    explicit Vector2i(const Vector2& p_vec2);
};

// Multiplicacion escalar conmutativa libre
inline Vector2i operator*(int32_t p_scalar, const Vector2i& p_v) {
    return p_v * p_scalar;
}

using Point2i = Vector2i;
using Size2i  = Vector2i;