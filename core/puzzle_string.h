// ========================================================================= //
// PUZZLE ENGINE - 2D/3D GAME ENGINE
// ------------------------------------------------------------------------- //
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// ========================================================================= //

#pragma once

#include <string>
#include <string_view>
#include <functional>
#include <ostream>

#include <cstdint>
// Tipo de cadena estándar principal del motor
using String = std::string;

// Estructura para cadenas inmutables e identificadores (estilo Godot)
// Compara hashes de 64 bits en O(1) en lugar de recorrer cadenas completas
struct StringName {
private:
    String m_name;
    uint64_t m_hash = 0;

    // Función hash FNV-1a en 64 bits
    static constexpr uint64_t compute_fnv1a(std::string_view p_str) {
        uint64_t hash = 14695981039346656037ULL;
        for (char c : p_str) {
            hash ^= static_cast<uint8_t>(c);
            hash *= 1099511628211ULL;
        }
        return hash;
    }

public:
    constexpr StringName() : m_name(""), m_hash(compute_fnv1a("")) {}

    StringName(const char* p_str)
        : m_name(p_str ? p_str : ""), m_hash(compute_fnv1a(p_str ? p_str : "")) {}

    StringName(const String& p_str)
        : m_name(p_str), m_hash(compute_fnv1a(p_str)) {}

    StringName(std::string_view p_str)
        : m_name(p_str), m_hash(compute_fnv1a(p_str)) {}

    // Acceso y consultas
    const String& get_string() const { return m_name; }
    const char* c_str() const { return m_name.c_str(); }
    uint64_t get_hash() const { return m_hash; }
    bool is_empty() const { return m_name.empty(); }

    // Conversión implícita a String estándar
    operator const String&() const { return m_name; }

    // Comparaciones instantáneas por hash
    bool operator==(const StringName& p_other) const {
        return m_hash == p_other.m_hash;
    }

    bool operator!=(const StringName& p_other) const {
        return m_hash != p_other.m_hash;
    }

    bool operator<(const StringName& p_other) const {
        return m_hash < p_other.m_hash;
    }

    friend std::ostream& operator<<(std::ostream& os, const StringName& sn) {
        return os << sn.m_name;
    }
};

// Especialización de std::hash para permitir usar StringName en std::unordered_map
struct StringNameHasher {
    size_t operator()(const StringName& sn) const noexcept {
        return static_cast<size_t>(sn.get_hash());
    }
};