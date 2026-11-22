// ========================================================================= //
// PUZZLE ENGINE - 2D/3D GAME ENGINE
// ------------------------------------------------------------------------- //
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// ========================================================================= //

#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <array>
#include <cstdint>
#include "core/variant.h"
#include "rendering_device.h"
#include "rendering_texture.h"
#include "globals/rendering_global/font_bitmap_primitive.h"

class RenderingGeometry {
public:
    struct Vertex2D {
        Vector2 position;
        Color color;
        Vector2 uv;
        float tex_index = 0.0f; // 0.0 = textura blanca por defecto, 1..15 = texturas activas
    };

    static constexpr size_t MAX_BATCH_VERTICES = 8192;
    static constexpr size_t MAX_BATCH_INDICES  = MAX_BATCH_VERTICES * 3;
    static constexpr size_t MAX_TEXTURE_SLOTS  = 16;

private:
    RenderingDevice* rendering_device = nullptr;

    std::vector<Vertex2D> batched_vertices;
    std::vector<uint32_t> batched_indices;

    // Gestión de slots de textura por lote (slot 0 reservado para textura blanca)
    std::array<uint32_t, MAX_TEXTURE_SLOTS> texture_slots{};
    uint32_t texture_slot_index = 1;

    // Métricas del pase actual
    uint32_t m_draw_calls = 0;

    inline bool check_batch_overflow(size_t p_extra_vertices, size_t p_extra_indices) {
        if (batched_vertices.size() + p_extra_vertices >= MAX_BATCH_VERTICES ||
            batched_indices.size() + p_extra_indices >= MAX_BATCH_INDICES) {
            flush();
            return true;
        }
        return false;
    }

    float get_or_add_texture_slot(uint32_t p_texture_id);

public:
    RenderingGeometry() = default;
    explicit RenderingGeometry(RenderingDevice* p_device);
    ~RenderingGeometry() = default;

    // Primitivas de renderizado 2D
    void draw_line(const Vector2& p_from, const Vector2& p_to, const Color& p_color, float p_width = 1.0f);
    void draw_rect(const Rect2& p_rect, const Color& p_color, bool p_filled = true, float p_width = 1.0f);
    void draw_circle(const Vector2& p_center, float p_radius, const Color& p_color, bool p_filled = true, int p_segments = 32);
    void draw_polygon(const std::vector<Vector2>& p_points, const Color& p_color);
    void draw_polyline(const std::vector<Vector2>& p_points, const Color& p_color, float p_width = 1.0f);

    void set_rendering_device(RenderingDevice* p_device) { rendering_device = p_device; }
    RenderingDevice* get_rendering_device() const { return rendering_device; }

    // Inserta un quad genérico en el lote actual (usado por RenderingTexture y RenderingText)
    void push_quad(
        const Vector2& v0, const Vector2& v1, const Vector2& v2, const Vector2& v3,
        const Vector2& uv0, const Vector2& uv1, const Vector2& uv2, const Vector2& uv3,
        const Color& p_color,
        uint32_t p_texture_id = 0
    );

    [[deprecated("Usar RenderingText para dibujar texto tipografico.")]]
    void draw_text(const Vector2& p_pos, const String& p_text, const Color& p_color = Color(1.0f, 1.0f, 1.0f, 1.0f), float p_scale = 2.0f);

    [[deprecated("Usar RenderingTexture::draw() para renderizar texturas.")]]
    void draw_texture(const Rect2& p_rect, uint32_t p_texture_id, const Color& p_modulate = Color(1.0f, 1.0f, 1.0f, 1.0f), const Rect2& p_src_rect = Rect2(0.0f, 0.0f, 1.0f, 1.0f));

    [[deprecated("Usar RenderingTexture::draw() para renderizar texturas.")]]
    void draw_texture(const Rect2& p_rect, const RenderingTexture& p_texture, const Color& p_modulate = Color(1.0f, 1.0f, 1.0f, 1.0f), const Rect2& p_src_rect = Rect2(0.0f, 0.0f, 1.0f, 1.0f));

    [[deprecated("Usar RenderingTexture::draw_quad() para renderizar quads texturizados.")]]
    void draw_quad_textured(
        const Vector2& v0, const Vector2& v1, const Vector2& v2, const Vector2& v3,
        const Vector2& uv0, const Vector2& uv1, const Vector2& uv2, const Vector2& uv3,
        const RenderingTexture& p_texture,
        const Color& p_color
    );

    // Ciclo de control del batching
    void begin();
    void flush();
    void clear();

    uint32_t get_draw_calls() const { return m_draw_calls; }
    const std::vector<Vertex2D>& get_vertices() const { return batched_vertices; }
    const std::vector<uint32_t>& get_indices() const { return batched_indices; }
};