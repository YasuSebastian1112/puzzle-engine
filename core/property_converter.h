// =============================================================================
// PUZZLE ENGINE — 2D/3D GAME ENGINE
// -----------------------------------------------------------------------------
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// =============================================================================

#pragma once

#include <string>
#include <vector>
#include "core/mathematics/math_2d.h"
#include "core/mathematics/transform_2d.h"
#include "core/color.h"

template <typename T>
struct PropertyConverter;

// float
template <>
struct PropertyConverter<float> {
    static float from_args(const std::vector<std::string>& args) {
        return args.empty() ? 0.0f : std::stof(args[0]);
    }
    static std::string to_string(float val) {
        return std::to_string(val);
    }
};

// int
template <>
struct PropertyConverter<int> {
    static int from_args(const std::vector<std::string>& args) {
        return args.empty() ? 0 : std::stoi(args[0]);
    }
    static std::string to_string(int val) {
        return std::to_string(val);
    }
};

// bool
template <>
struct PropertyConverter<bool> {
    static bool from_args(const std::vector<std::string>& args) {
        if (args.empty()) return false;
        return (args[0] == "true" || args[0] == "1");
    }
    static std::string to_string(bool val) {
        return val ? "true" : "false";
    }
};

// std::string
template <>
struct PropertyConverter<std::string> {
    static std::string from_args(const std::vector<std::string>& args) {
        return args.empty() ? "" : args[0];
    }
    static std::string to_string(const std::string& val) {
        return val;
    }
};

// Vector2
template <>
struct PropertyConverter<Vector2> {
    static Vector2 from_args(const std::vector<std::string>& args) {
        if (args.empty()) return Vector2(0.0f, 0.0f);
        if (args.size() == 1) {
            float v = std::stof(args[0]);
            return Vector2(v, v);
        }
        return Vector2(std::stof(args[0]), std::stof(args[1]));
    }
    static std::string to_string(const Vector2& v) {
        return "(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ")";
    }
};

// Vector2i
template <>
struct PropertyConverter<Vector2i> {
    static Vector2i from_args(const std::vector<std::string>& args) {
        if (args.empty()) return Vector2i(0, 0);
        if (args.size() == 1) {
            int32_t v = std::stoi(args[0]);
            return Vector2i(v, v);
        }
        return Vector2i(std::stoi(args[0]), std::stoi(args[1]));
    }
    static std::string to_string(const Vector2i& v) {
        return "(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ")";
    }
};

// Rect2: pos_x pos_y size_w size_h
template <>
struct PropertyConverter<Rect2> {
    static Rect2 from_args(const std::vector<std::string>& args) {
        if (args.size() < 4) return Rect2();
        return Rect2(std::stof(args[0]), std::stof(args[1]), std::stof(args[2]), std::stof(args[3]));
    }
    static std::string to_string(const Rect2& r) {
        return "[P: (" + std::to_string(r.position.x) + ", " + std::to_string(r.position.y) +
               ") S: (" + std::to_string(r.size.x) + ", " + std::to_string(r.size.y) + ")]";
    }
};

// Especialización: Color
template <>
struct PropertyConverter<Color> {
    static Color from_args(const std::vector<std::string>& args) {
        if (args.empty()) return Color(1.0f, 1.0f, 1.0f, 1.0f);
        if (args.size() == 1) {
            float v = std::stof(args[0]);
            return Color(v, v, v, 1.0f);
        }
        if (args.size() == 3) {
            return Color(std::stof(args[0]), std::stof(args[1]), std::stof(args[2]), 1.0f);
        }
        return Color(std::stof(args[0]), std::stof(args[1]), std::stof(args[2]), std::stof(args[3]));
    }

    static std::string to_string(const Color& c) {
        return "Color(" + std::to_string(c.r) + ", " + std::to_string(c.g) + ", " + 
               std::to_string(c.b) + ", " + std::to_string(c.a) + ")";
    }
};