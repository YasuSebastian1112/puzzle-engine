// ============================================================================ //
// Puzzle Engine - 2D/3D Hybrid Game Engine
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// ============================================================================ //

#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <functional>
#include "core/variant.h"

// Macro de RTTI e introspección rápida inspirada en la arquitectura de Godot
#define CLASS_TYPE(m_class, m_inherits)                                            \
private:                                                                        \
    void operator=(const m_class &);                                            \
public:                                                                         \
    virtual const char* get_class_name() const override {                       \
        return #m_class;                                                        \
    }                                                                           \
    virtual bool is_class(std::string_view p_name) const override {             \
        if (p_name == #m_class) return true;                                    \
        return m_inherits::is_class(p_name);                                    \
    }                                                                           \
    static const char* get_class_static() {                                     \
        return #m_class;                                                        \
    }                                                                           \
    static bool is_class_static(std::string_view p_name) {                      \
        if (p_name == #m_class) return true;                                    \
        return m_inherits::is_class_static(p_name);                             \
    }

class Object {
public:
    using ObjectID = uint64_t;
    using SignalCallback = std::function<void(const std::vector<Variant>&)>;

    
    // RTTI BÁSICO DE LA RAÍZ
    
    virtual const char* get_class_name() const { return "Object"; }
    virtual bool is_class(std::string_view p_name) const { return p_name == "Object"; }
    static const char* get_class_static() { return "Object"; }
    static bool is_class_static(std::string_view p_name) { return p_name == "Object"; }
    
    // CASTING SEGURO
    
    template <typename T>
    static T* cast_to(Object* p_object) {
        if (!p_object) return nullptr;
        if (p_object->is_class(T::get_class_static())) {
            return static_cast<T*>(p_object);
        }
        return nullptr;
    }

    template <typename T>
    static const T* cast_to(const Object* p_object) {
        if (!p_object) return nullptr;
        if (p_object->is_class(T::get_class_static())) {
            return static_cast<const T*>(p_object);
        }
        return nullptr;
    }

    ObjectID get_instance_id() const { return instance_id; }

    // Destrucción diferida segura
    void queue_free();
    bool is_queued_for_deletion() const { return queued_for_deletion; }

        // SISTEMA DE SEÑALES / EVENTOS (Observer Pattern)
    
    void connect(const std::string& p_signal, const SignalCallback& p_callback);
    void emit_signal(const std::string& p_signal, const std::vector<Variant>& p_args = {});
    void disconnect_all(const std::string& p_signal);

    // METADATOS Y PROPIEDADES DINÁMICAS (Variants)
    
    void set_meta(const std::string& p_name, const Variant& p_value);
    Variant get_meta(const std::string& p_name, const Variant& p_default = Variant()) const;
    bool has_meta(const std::string& p_name) const;
    void remove_meta(const std::string& p_name);

    // NOTIFICACIONES INTERNAS DEL MOTOR (Ciclos de procesamiento, inicialización)

    virtual void notification(int p_notification) {}

    // CONSTRUCTOR Y CICLO DE VIDA
    
    Object();
    virtual ~Object();

private:
    ObjectID instance_id;
    bool queued_for_deletion = false;

    // Almacén de señales: Mapa de Nombre de Señal -> Lista de callbacks
    std::unordered_map<std::string, std::vector<SignalCallback>> signal_map;

    // Metadatos arbitrarios
    std::unordered_map<std::string, Variant> metadata;

    static inline ObjectID next_instance_id = 1;
};