// =============================================================================
// PUZZLE ENGINE — 2D/3D GAME ENGINE
// -----------------------------------------------------------------------------
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// =============================================================================

#pragma once

#include <string>
#include <cstdint>

#include "mathematics/math_2d.h"
#include "mathematics/transform_2d.h"
#include "mathematics/curve.h"
#include "color.h"

using String = std::string;

class Object;

class Variant {
public:
    enum class Type {
        NIL,
        BOOL,
        INT,
        FLOAT,
        STRING,

        // Espaciales 2D
        VECTOR2,
        VECTOR2I,
        RECT2,
        TRANSFORM2D,
        CURVE2D,

        // Otros
        COLOR,
        OBJECT
    };

    // Constructores
    Variant() : type(Type::NIL) {}
    Variant(bool v) : type(Type::BOOL), val_bool(v) {}
    Variant(int64_t v) : type(Type::INT), val_int(v) {}
    Variant(int32_t v) : type(Type::INT), val_int(static_cast<int64_t>(v)) {}
    Variant(double v) : type(Type::FLOAT), val_float(v) {}
    Variant(float v) : type(Type::FLOAT), val_float(static_cast<double>(v)) {}
    Variant(const String& v) : type(Type::STRING), val_string(v) {}
    Variant(const char* v) : type(Type::STRING), val_string(v ? v : "") {}

    Variant(const Vector2& v) : type(Type::VECTOR2), val_vector2(v) {}
    Variant(const Vector2i& v) : type(Type::VECTOR2I), val_vector2i(v) {}
    Variant(const Rect2& v) : type(Type::RECT2), val_rect2(v) {}
    Variant(const Transform2D& v) : type(Type::TRANSFORM2D), val_transform2d(v) {}
    Variant(const Color& v) : type(Type::COLOR), val_color(v) {}
    Variant(const CurveBezier2D& v) : type(Type::CURVE2D), val_curve(new CurveBezier2D(v)) {}
    Variant(Object* v) : type(Type::OBJECT), val_object(v) {}

    // Constructor de copia y movimiento
    Variant(const Variant& other) : type(Type::NIL) {
        copy_from(other);
    }

    Variant& operator=(const Variant& other) {
        if (this != &other) {
            clear();
            copy_from(other);
        }
        return *this;
    }

    ~Variant() {
        clear();
    }

    // Tipo y Getters
    Type get_type() const { return type; }

    bool as_bool() const { return val_bool; }
    int64_t as_int() const { return val_int; }
    double as_float() const { return val_float; }
    String as_string() const { return val_string; }

    Vector2 as_vector2() const { return val_vector2; }
    Vector2i as_vector2i() const { return val_vector2i; }
    Rect2 as_rect2() const { return val_rect2; }
    Transform2D as_transform2d() const { return val_transform2d; }
    Color as_color() const { return val_color; }
    const CurveBezier2D& as_curve2d() const { return *val_curve; }
    Object* as_object() const { return val_object; }

private:
    void clear() {
        if (type == Type::CURVE2D && val_curve) {
            delete val_curve;
            val_curve = nullptr;
        }
        type = Type::NIL;
    }

    void copy_from(const Variant& other) {
        type = other.type;
        val_string = other.val_string;

        switch (other.type) {
            case Type::BOOL: val_bool = other.val_bool; break;
            case Type::INT: val_int = other.val_int; break;
            case Type::FLOAT: val_float = other.val_float; break;
            case Type::VECTOR2: val_vector2 = other.val_vector2; break;
            case Type::VECTOR2I: val_vector2i = other.val_vector2i; break;
            case Type::RECT2: val_rect2 = other.val_rect2; break;
            case Type::TRANSFORM2D: val_transform2d = other.val_transform2d; break;
            case Type::COLOR: val_color = other.val_color; break;
            case Type::CURVE2D: val_curve = other.val_curve ? new CurveBezier2D(*other.val_curve) : nullptr; break;
            case Type::OBJECT: val_object = other.val_object; break;
            default: break;
        }
    }

    Type type;

    union {
        bool val_bool;
        int64_t val_int;
        double val_float;

        Vector2 val_vector2;
        Vector2i val_vector2i;
        Rect2 val_rect2;
        Transform2D val_transform2d;
        Color val_color;
        CurveBezier2D* val_curve;

        Object* val_object;
    };

    String val_string;
};