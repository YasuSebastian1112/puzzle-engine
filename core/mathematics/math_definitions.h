// ========================================================================= //
// Puzzle Engine - 2D/3D Hybrid Game Engine
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// ===============================================

#pragma once

#include <limits>

namespace MathDefs {

    inline constexpr double PI = 3.14159265358979323846;
    inline constexpr double TAU = 6.28318530717958647692; // 2 * PI
    inline constexpr double HALF_PI = 1.57079632679489661923; // PI / 2
    inline constexpr double SQRT2 = 1.41421356237309504880;
    inline constexpr double E = 2.71828182845904523536;

    inline constexpr double CMP_EPSILON = 0.00001;
    inline constexpr double CMP_EPSILON2 = CMP_EPSILON * CMP_EPSILON;
    inline constexpr float  CMP_EPSILON_F = 0.0001f;
    
    inline constexpr double INF_DOUBLE = std::numeric_limits<double>::infinity();
    inline constexpr float  INF_FLOAT = std::numeric_limits<float>::infinity();
    inline constexpr double NAN_DOUBLE = std::numeric_limits<double>::quiet_NaN();
};