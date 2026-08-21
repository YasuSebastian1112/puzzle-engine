// ========================================================================= //
// Puzzle Engine - 2D/3D Hybrid Game Engine
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// ========================================================================= //

#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <functional>
#include <vector>


// La arquitectura de nodos se basa en la de Godot
#define CLASS_TYPE(m_class, m_inherits) \
private: \
public: \
    virtual const char* get_class_name() const override {  \
        return #m_class;  \
    }  \
    \
    virtual bool is_class(const std::string& name) const override {  \
        if (name==#m_class) return true;  \
        return m_inherits::is_class(name);  \
    }  \
    \
    static const char* get_class_static() { \
        return #m_class; \
    }  \


// De esta clase parten las demas
class Object {
public:

    uint64_t get_instance_id() const {return instance_id;}
    virtual const char* get_class_name() const {return "Object";}
    virtual bool is_class(const std::string& name) const {return name == "Object";}
    static const char* get_class_static() { return "Object"; }

    Object() : instance_id(++next_id) {}
    virtual ~Object() = default;

private:

    uint64_t instance_id;
    static inline uint64_t next_id = 0;

};