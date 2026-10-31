// ========================================================================= //
// PUZZLE ENGINE - 2D/3D GAME ENGINE
// ------------------------------------------------------------------------- //
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// ========================================================================= //

#pragma once

#include <vector>
#include <cstdint>
#include "core/variant.h"
#include "rendering_device.h"
#include "globals/rendering_global/font_bitmap_primitive.h"

class RenderingGeometry {
public:
    struct Vertex2D {
        Vector2 position;
        Color color;
        Vector2 uv;
    };

    // Capacidad por lote ajustada para balancear llamadas de dibujo (draw calls) y uso de memoria
    static constexpr size_t MAX_BATCH_VERTICES = 8192;
    static constexpr size_t MAX_BATCH_INDICES  = MAX_BATCH_VERTICES * 3;

private:
    RenderingDevice* rendering_device = nullptr;

    std::vector<Vertex2D> batched_vertices;
    std::vector<uint32_t> batched_indices;

    // Comprobación de capacidad antes de insertar primitivas
    inline bool check_batch_overflow(size_t p_extra_vertices, size_t p_extra_indices) {
        if (batched_vertices.size() + p_extra_vertices >= MAX_BATCH_VERTICES ||
            batched_indices.size() + p_extra_indices >= MAX_BATCH_INDICES) {
            flush();
            return true;
        }
        return false;
    }

public:
    RenderingGeometry() = default;
    explicit RenderingGeometry(RenderingDevice* p_device);
    ~RenderingGeometry() = default;

    void set_rendering_device(RenderingDevice* p_device) { rendering_device = p_device; }

    // Primitivas de renderizado 2D
    void draw_line(const Vector2& p_from, const Vector2& p_to, const Color& p_color, float p_width = 1.0f);
    void draw_rect(const Rect2& p_rect, const Color& p_color, bool p_filled = true, float p_width = 1.0f);
    void draw_circle(const Vector2& p_center, float p_radius, const Color& p_color, bool p_filled = true, int p_segments = 32);
    void draw_polygon(const std::vector<Vector2>& p_points, const Color& p_color);
    void draw_polyline(const std::vector<Vector2>& p_points, const Color& p_color, float p_width = 1.0f);

    void draw_text(const Vector2& p_pos, const String& p_text, const Color& p_color = Color(1.0f, 1.0f, 1.0f, 1.0f), float p_scale = 2.0f);

    // Ciclo de control del batching
    void begin();
    void flush();
    void clear();

    const std::vector<Vertex2D>& get_vertices() const { return batched_vertices; }
    const std::vector<uint32_t>& get_indices() const { return batched_indices; }
};