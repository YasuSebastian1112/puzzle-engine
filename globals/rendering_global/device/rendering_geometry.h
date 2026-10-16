// ========================================================================= //
// Sierra Engine - 2D/3D Hybrid Game Engine
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// ========================================================================= //

#pragma once

#include <vector>

#include "core/variant.h"
#include "rendering_device.h"

class RenderingGeometry {
public:
    struct Vertex2D {
        Vector2 position;
        Color color;
        Vector2 uv;
    };

private:
    RenderingDevice* rendering_device = nullptr;
    std::vector<Vertex2D> batched_vertices;
    std::vector<uint32_t> batched_indices;

public:
    RenderingGeometry() = default;
    explicit RenderingGeometry(RenderingDevice* p_device);
    ~RenderingGeometry() = default;

    void set_rendering_device(RenderingDevice* p_device) { rendering_device = p_device; }

    // Primitivas de dibujado
    void draw_line(const Vector2& p_from, const Vector2& p_to, const Color& p_color, float p_width = 1.0f);
    void draw_rect(const Rect2& p_rect, const Color& p_color, bool p_filled = true, float p_width = 1.0f);
    void draw_circle(const Vector2& p_center, float p_radius, const Color& p_color, bool p_filled = true, int p_segments = 32);
    void draw_polygon(const std::vector<Vector2>& p_points, const Color& p_color);
    void draw_polyline(const std::vector<Vector2>& p_points, const Color& p_color, float p_width = 1.0f);

    // Ciclo de batching
    void begin();
    void flush();
    void clear();

    const std::vector<Vertex2D>& get_vertices() const { return batched_vertices; }
    const std::vector<uint32_t>& get_indices() const { return batched_indices; }
};