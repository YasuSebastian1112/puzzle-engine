// ========================================================================= //
// Puzzle Engine - 2D/3D Hybrid Game Engine
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// ========================================================================= //

#pragma once


#include "math_2d.h"
#include "math_functions.h"
#include "math_definitions.h"
#include <vector>

/*Esctructuras Bezier*/


struct BezierPoint2D {
    Vector2 position;
    Vector2 in;
    Vector2 out;

    BezierPoint2D(const Vector2& p_pos = Vector2(), const Vector2& p_in = Vector2(), const Vector2& p_out = Vector2())
        : position(p_pos), in(p_in), out(p_out) {}
};

class CurveBezier2D {
private:
    std::vector<BezierPoint2D> points;

public:
    CurveBezier2D() = default;

    // Métodos inline de acceso y manipulación básica
    void add_point(const Vector2& p_pos, const Vector2& p_in = Vector2(), const Vector2& p_out = Vector2());
    void remove_point(size_t p_index);
    void clear() { points.clear(); }
    
    size_t get_point_count() const { return points.size(); }
    size_t get_segment_count() const { return points.empty() ? 0 : points.size() - 1; }

    const BezierPoint2D& get_point(size_t p_index) const { return points[p_index]; }
    void set_point(size_t p_index, const BezierPoint2D& p_point) { points[p_index] = p_point; }

    // Evaluación matemática y horneado de vértices
    Vector2 sample_segment(size_t p_segment_idx, float p_t) const;
    std::vector<Vector2> bake(int p_segments_per_curve = 24) const;
};
