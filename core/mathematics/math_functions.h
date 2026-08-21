// ========================================================================= //
// Puzzle Engine - 2D/3D Hybrid Game Engine
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// ========================================================================= //

#pragma once

#include <cmath>
#include "core/core.h"
#include "math_definitions.h"

class Math {
public:
    Math() = delete;

    // Trigonometría
    static inline double sin(double p_x) noexcept {return std::sin(p_x);}
    static inline double cos(double p_x) noexcept {return std::cos(p_x);}
    static inline double tan(double p_x) noexcept {return std::tan(p_x);}

    // Trigonométricas inversas
    static inline double asin(double p_x) noexcept {return std::asin(p_x);}
    static inline double acos(double p_x) noexcept {return std::acos(p_x);}
    static inline double atan(double p_x) noexcept {return std::atan(p_x);}
    static inline double atan2(double p_y, double p_x) noexcept {return std::atan2(p_y, p_x); }

    // Hiperbólicas
    static inline double sinh(double p_x) noexcept {return std::sinh(p_x);}
    static inline double cosh(double p_x) noexcept {return std::cosh(p_x);}
    static inline double tanh(double p_x) noexcept {return std::tanh(p_x);}

    // Hiperbólicas inversas
    static inline double asinh(double p_x) noexcept {return std::asinh(p_x);}
    static inline double acosh(double p_x) noexcept {return std::acosh(p_x);}
    static inline double atanh(double p_x) noexcept {return std::atanh(p_x);}

    // Conversiones
    static double deg_to_rad(double p_deg);
    static double rad_to_deg(double p_rad);

    // De operacion
    static inline double sqrt(double p_x) noexcept {return std::sqrt(p_x);}
    static inline double pow(double p_x, double p_y) noexcept {return std::pow(p_x, p_y);}
    static inline double log(double p_x) noexcept {return std::log(p_x);}
    static inline double exp(double p_x) noexcept {return std::exp(p_x);}
    static inline double abs(double p_x) noexcept {return std::abs(p_x);}
    static inline double fmod(double p_x, double p_y) noexcept {return std::fmod(p_x, p_y);}
    static double fposmod(double p_x, double p_y);

    // Utilidades
    static inline double round(double p_n) noexcept {return std::round(p_n);}
    static inline double floor(double p_n) noexcept {return std::floor(p_n);}
    static inline double ceil(double p_n) noexcept {return std::ceil(p_n);}
    static bool is_equal_approx(double p_a, double p_b, double p_tolerance = MathDefs::CMP_EPSILON);
    static bool is_zero_approx(double p_val, double p_tolerance = MathDefs::CMP_EPSILON);
    static double angle_diference(double p_from, double p_to);
    static double sign(double p_x);

    // De transicion, ajuste y control
    static double ease(double p_n, double p_c);
    static double lerp(double p_from, double p_to, double p_weight);
    static double inverse_lerp(double p_from, double p_to, double p_val);
    static double lerp_angle(double p_from, double p_to, double p_weight);
    static double clamp(double p_val, double p_min, double p_max);
    static double smoothstep(double p_from, double p_to, double p_val);
    static double snapped(double p_val, double p_step);
    static double wrapf(double p_val, double p_min, double p_max);
    static double remap(double p_val, double p_in_from, double p_in_to, double p_out_from, double p_out_to);

};
