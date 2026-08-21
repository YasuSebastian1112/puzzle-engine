// ========================================================================= //
// Puzzle Engine - 2D/3D Hybrid Game Engine
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// ========================================================================= //

#pragma once
#include <string>
#include <cstdint>

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

        // Otros tipos

        OBJECT
    };

    // Construir los tipos
    Variant() : type(Type::NIL) {}
    Variant(bool v) : type(Type::BOOL), val_bool(v) {}
    Variant(int64_t v) : type(Type::INT), val_int(v) {}
    Variant(double v) : type(Type::FLOAT), val_float(v) {}
    Variant(const std::string& v) : type(Type::STRING), val_string(v) {}
    Variant(Object* v) : type(Type::OBJECT), val_object(v) {}

    // Obtener los tipos

    Type get_type() const {return type;}

    int64_t as_int() const {return val_int;}
    double as_float() const {return val_float;}
    bool as_bool() const {return val_bool;}
    std::string as_string() const {return val_string;}

    Object* as_object() const {return val_object;}

private:
    
    Type type;

    union {
        bool val_bool;
        int64_t val_int;
        double val_float;
        Object* val_object;
    };
    
    std::string val_string;
};