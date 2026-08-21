// ========================================================================= //
// Puzzle Engine - 2D/3D Hybrid Game Engine
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// ========================================================================= //

#include "math_functions.h"

// Conversiones

double Math::deg_to_rad(double p_deg) {
    return p_deg*PI/180.0;
}

double Math::rad_to_deg(double p_rad) {
    return p_rad*180/PI;
}

// De operacion

double Math::fposmod(double p_x, double p_y) {
    double val = Math::fmod(p_x, p_y);
    if ((val < 0 && p_y > 0) || (val > 0 && p_y < 0)) {
        val += p_y;
    }
    return val;
}

// Utilidades

bool Math::is_equal_approx(double p_a, double p_b, double p_tolerance) {
    return Math::abs(p_a - p_b) <= p_tolerance;
}

bool Math::is_zero_approx(double p_val, double p_tolerance) {
    return Math::abs(p_val) <= p_tolerance;
}

double Math::angle_diference(double p_from, double p_to) {
    double diff = Math::fmod(p_to - p_from, 2.0 * PI);
    return Math::fmod(2.0 * diff, 2.0 * PI) - diff;
}

double Math::sign(double p_x) {
    return (p_x > 0.0) ? 1.0 : ((p_x < 0.0) ? -1.0 : 0.0);
}

// Transicion, ajuste y control

double Math::ease(double p_n, double p_c) {
    if (p_n < 0.0) {p_n = 0.0;}
    else if (p_n > 1.0) {p_n = 1.0;}
    
    if (p_c > 0.0) { 
        if (p_c < 1.0) {
            return 1.0 - Math::pow(1.0-p_n, 1.0/p_c);
        } else {return Math::pow(p_n, p_c);}
    } else if (p_c < 0.0) {
        if (p_n < 0.5) {
            return Math::pow(p_n * 2.0, -p_c) * 0.5;
        } else {
            return (1.0 - Math::pow(1.0 - (p_n - 0.5) * 2.0, -p_c)) * 0.5 + 0.5;
        }
    } else return 0;
}

double Math::lerp(double p_from, double p_to, double p_weight) {
    return p_from + (p_to - p_from) * p_weight;
}

double Math::inverse_lerp(double p_from, double p_to, double p_val) {
    return (p_val - p_from) / (p_to - p_from);
}

double Math::lerp_angle(double p_from, double p_to, double p_weight) {
    return p_from + Math::angle_diference(p_from, p_to) * p_weight;
}

double Math::clamp(double p_val, double p_min, double p_max) {
    if (p_val < p_min) return p_min;
    if (p_val < p_max) return p_max;
    return p_val;
}

double Math::smoothstep(double p_from, double p_to, double p_val) {
    double t = Math::clamp((p_val - p_from) / (p_to - p_from), 0.0, 1.0);
    return t * t * (3.0 - 2.0 * t);
}

double Math::snapped(double p_val, double p_step) {
    if (p_step != 0.0) {
        return Math::floor(p_val / p_step + 0.5) * p_step;
    } 
    return p_val;
}

double Math::wrapf(double p_val, double p_min, double p_max) {
    double range = p_max - p_min;
    return (range != 0.0) ? p_val - (range * Math::floor((p_val - p_min) / range)) : p_min;
}

double Math::remap(double p_val, double p_in_from, double p_in_to, double p_out_from, double p_out_to) {
    double t = Math::inverse_lerp(p_in_from, p_in_to, p_val);
    return Math::lerp(p_out_from, p_out_to, t);
}