// ========================================================================= //
// Sierra Engine - 2D/3D Hybrid Game Engine
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// ========================================================================= //

#pragma once

#include <cstdint>

struct Color {

    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
    float a = 1.0f;

    // 1. Constructor por defecto (blanco opaco)
    constexpr Color() = default;

    // 2. RGB normalizado (0.0f a 1.0f) con alpha opcional
    constexpr Color(float p_r, float p_g, float p_b, float p_a = 1.0f)
        : r(p_r), g(p_g), b(p_b), a(p_a) {}

    // 3. RGBA en bytes enteros (0 a 255) tipo uint8_t
    Color(uint8_t p_r, uint8_t p_g, uint8_t p_b, uint8_t p_a = 255)
        : r(p_r / 255.0f), g(p_g / 255.0f), b(p_b / 255.0f), a(p_a / 255.0f) {}

    // 4. Desde entero hexadecimal en 32 bits (0xRRGGBBAA o 0xRRGGBB)
    explicit Color(uint32_t p_rgba) {
        r = ((p_rgba >> 24) & 0xFF) / 255.0f;
        g = ((p_rgba >> 16) & 0xFF) / 255.0f;
        b = ((p_rgba >> 8) & 0xFF) / 255.0f;
        a = (p_rgba & 0xFF) / 255.0f;
    }

};
