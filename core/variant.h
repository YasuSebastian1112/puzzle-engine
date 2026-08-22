// ========================================================================= //
// Puzzle Engine - 2D/3D Hybrid Game Engine
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// ========================================================================= //

#pragma once
#include <string>
#include <cstdint>

#include "mathematics/math_2d.h"
#include "color.h"

class Object;

class Variant {
public:
    
    enum class Type {
        
        // Variables Comunes

        NIL,
        BOOL,
        INT,
        FLOAT,
        STRING,

        // Espaciales

        VECTOR2,
        VECTOR2I,

        // Otros tipos

        OBJECT,
        COLOR

    };

    // Construir los tipos
    Variant() : type(Type::NIL) {}
    Variant(bool v) : type(Type::BOOL), val_bool(v) {}
    Variant(int64_t v) : type(Type::INT), val_int(v) {}
    Variant(double v) : type(Type::FLOAT), val_float(v) {}
    Variant(const std::string& v) : type(Type::STRING), val_string(v) {}
    
    Variant(const Vector2& v) : type(Type::VECTOR2), val_vector2(v) {}
    Variant(const Vector2i& v) : type(Type::VECTOR2I), val_vector2i(v) {}

    Variant(Object* v) : type(Type::OBJECT), val_object(v) {}
    

    // Obtener los tipos

    Type get_type() const {return type;}

    int64_t as_int() const {return val_int;}
    double as_float() const {return val_float;}
    bool as_bool() const {return val_bool;}
    std::string as_string() const {return val_string;}

    Vector2 as_vector2() const { return val_vector2; }
    Vector2i as_vector2i() const { return val_vector2i; }


    Object* as_object() const {return val_object;}

private:
    
    Type type;

    union {
        bool val_bool;
        int64_t val_int;
        double val_float;

        Vector2 val_vector2;
        Vector2i val_vector2i;  

        Object* val_object;
    };
    
    std::string val_string;
};