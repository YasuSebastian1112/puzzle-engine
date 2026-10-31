// ========================================================================= //
// PUZZLE ENGINE - 2D/3D GAME ENGINE
// ------------------------------------------------------------------------- //
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// ========================================================================= //

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "rendering_geometry.h"


RenderingGeometry::RenderingGeometry(RenderingDevice* p_device)
    : rendering_device(p_device) {
    // Preasignar capacidad para evitar reallocs continuos en el heap durante el renderizado
    batched_vertices.reserve(MAX_BATCH_VERTICES);
    batched_indices.reserve(MAX_BATCH_INDICES);
}

void RenderingGeometry::begin() {
    clear();
}

void RenderingGeometry::flush() {
    if (batched_vertices.empty() || batched_indices.empty() || !rendering_device) {
        return;
    }

    rendering_device->render_geometry(
        batched_vertices.data(),
        batched_vertices.size() * sizeof(Vertex2D),
        batched_indices.data(),
        batched_indices.size()
    );

    clear();
}

void RenderingGeometry::clear() {
    batched_vertices.clear();
    batched_indices.clear();
}

void RenderingGeometry::draw_line(const Vector2& p_from, const Vector2& p_to, const Color& p_color, float p_width) {
    check_batch_overflow(4, 6);

    Vector2 delta = p_to - p_from;
    float len = delta.length();
    if (len <= 0.0001f) return;

    Vector2 dir = delta / len;
    Vector2 normal = Vector2(-dir.y, dir.x) * (p_width * 0.5f);

    uint32_t base_index = static_cast<uint32_t>(batched_vertices.size());

    batched_vertices.push_back({ p_from + normal, p_color, Vector2(0.0f, 0.0f) });
    batched_vertices.push_back({ p_to + normal,   p_color, Vector2(1.0f, 0.0f) });
    batched_vertices.push_back({ p_to - normal,   p_color, Vector2(1.0f, 1.0f) });
    batched_vertices.push_back({ p_from - normal, p_color, Vector2(0.0f, 1.0f) });

    batched_indices.push_back(base_index + 0);
    batched_indices.push_back(base_index + 1);
    batched_indices.push_back(base_index + 2);

    batched_indices.push_back(base_index + 2);
    batched_indices.push_back(base_index + 3);
    batched_indices.push_back(base_index + 0);
}

void RenderingGeometry::draw_polyline(const std::vector<Vector2>& p_points, const Color& p_color, float p_width) {
    if (p_points.size() < 2) return;

    for (size_t i = 0; i < p_points.size() - 1; ++i) {
        draw_line(p_points[i], p_points[i + 1], p_color, p_width);
    }
}

void RenderingGeometry::draw_rect(const Rect2& p_rect, const Color& p_color, bool p_filled, float p_width) {
    Rect2 r = p_rect.abs();

    Vector2 top_left = r.position;
    Vector2 top_right = r.position + Vector2(r.size.x, 0.0f);
    Vector2 bottom_right = r.position + r.size;
    Vector2 bottom_left = r.position + Vector2(0.0f, r.size.y);

    if (p_filled) {
        check_batch_overflow(4, 6);

        uint32_t base_index = static_cast<uint32_t>(batched_vertices.size());

        batched_vertices.push_back({ top_left,     p_color, Vector2(0.0f, 0.0f) });
        batched_vertices.push_back({ top_right,    p_color, Vector2(1.0f, 0.0f) });
        batched_vertices.push_back({ bottom_right, p_color, Vector2(1.0f, 1.0f) });
        batched_vertices.push_back({ bottom_left,  p_color, Vector2(0.0f, 1.0f) });

        batched_indices.push_back(base_index + 0);
        batched_indices.push_back(base_index + 1);
        batched_indices.push_back(base_index + 2);

        batched_indices.push_back(base_index + 2);
        batched_indices.push_back(base_index + 3);
        batched_indices.push_back(base_index + 0);
    } else {
        draw_line(top_left, top_right, p_color, p_width);
        draw_line(top_right, bottom_right, p_color, p_width);
        draw_line(bottom_right, bottom_left, p_color, p_width);
        draw_line(bottom_left, top_left, p_color, p_width);
    }
}

void RenderingGeometry::draw_circle(const Vector2& p_center, float p_radius, const Color& p_color, bool p_filled, int p_segments) {
    if (p_segments < 3) p_segments = 3;

    if (p_filled) {
        size_t required_verts = p_segments + 2;
        size_t required_indices = p_segments * 3;
        check_batch_overflow(required_verts, required_indices);

        uint32_t center_idx = static_cast<uint32_t>(batched_vertices.size());
        batched_vertices.push_back({ p_center, p_color, Vector2(0.5f, 0.5f) });

        float step = MathDefs::TAU / static_cast<float>(p_segments);

        for (int i = 0; i <= p_segments; ++i) {
            float angle = static_cast<float>(i) * step;
            Vector2 pos = p_center + Vector2(Math::cos(angle) * p_radius, Math::sin(angle) * p_radius);
            batched_vertices.push_back({ pos, p_color, Vector2(0.0f, 0.0f) });
        }

        for (int i = 1; i <= p_segments; ++i) {
            batched_indices.push_back(center_idx);
            batched_indices.push_back(center_idx + static_cast<uint32_t>(i));
            batched_indices.push_back(center_idx + static_cast<uint32_t>(i + 1));
        }
    } else {
        float step = MathDefs::TAU / static_cast<float>(p_segments);
        for (int i = 0; i < p_segments; ++i) {
            float a1 = static_cast<float>(i) * step;
            float a2 = static_cast<float>(i + 1) * step;

            Vector2 p1 = p_center + Vector2(Math::cos(a1) * p_radius, Math::sin(a1) * p_radius);
            Vector2 p2 = p_center + Vector2(Math::cos(a2) * p_radius, Math::sin(a2) * p_radius);

            draw_line(p1, p2, p_color, 1.0f);
        }
    }
}

void RenderingGeometry::draw_polygon(const std::vector<Vector2>& p_points, const Color& p_color) {
    if (p_points.size() < 3) return;

    size_t vert_count = p_points.size();
    size_t index_count = (vert_count - 2) * 3;
    check_batch_overflow(vert_count, index_count);

    uint32_t base_index = static_cast<uint32_t>(batched_vertices.size());

    for (const auto& pt : p_points) {
        batched_vertices.push_back({ pt, p_color, Vector2(0.0f, 0.0f) });
    }

    // Triangulación tipo abanico (Triangle Fan)
    for (size_t i = 1; i < vert_count - 1; ++i) {
        batched_indices.push_back(base_index);
        batched_indices.push_back(base_index + static_cast<uint32_t>(i));
        batched_indices.push_back(base_index + static_cast<uint32_t>(i + 1));
    }
}

void RenderingGeometry::draw_text(const Vector2& p_pos, const String& p_text, const Color& p_color, float p_scale) {
    float cur_x = p_pos.x;
    float cur_y = p_pos.y;

    for (size_t i = 0; i < p_text.length(); ++i) {
        char c = p_text[i];

        if (c == '\n') {
            cur_x = p_pos.x;
            cur_y += (DefaultFont::GLYPH_HEIGHT + 2) * p_scale;
            continue;
        }

        if (c < 32 || c > 126) {
            c = '?';
        }

        int glyph_idx = c - DefaultFont::FIRST_CHAR;
        const uint8_t* glyph = DefaultFont::FONT_8X8[glyph_idx];

        for (int row = 0; row < DefaultFont::GLYPH_HEIGHT; ++row) {
            uint8_t row_bits = glyph[row];
            if (!row_bits) continue;

            // Optimización: agrupar bits consecutivos en tiras horizontales continuas
            for (int col = 0; col < DefaultFont::GLYPH_WIDTH; ++col) {
                if (row_bits & (0x80 >> col)) {
                    int run_len = 1;
                    while ((col + run_len < DefaultFont::GLYPH_WIDTH) && (row_bits & (0x80 >> (col + run_len)))) {
                        run_len++;
                    }

                    Vector2 run_pos(cur_x + static_cast<float>(col) * p_scale, cur_y + static_cast<float>(row) * p_scale);
                    Vector2 run_size(static_cast<float>(run_len) * p_scale, p_scale);
                    draw_rect(Rect2(run_pos, run_size), p_color, true);

                    col += run_len - 1;
                }
            }
        }

        cur_x += (DefaultFont::GLYPH_WIDTH + 1) * p_scale;
    }
}

void RenderingGeometry::draw_texture(const Rect2& p_rect, uint32_t p_texture_id, const Color& p_modulate, const Rect2& p_src_rect) {
    // Si la textura solicitada cambia respecto a lo que hay en el lote actual,
    // se vacía el buffer para no pintar geometrías previas con la textura nueva
    flush();

    // Enlazar la textura en el slot 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, p_texture_id);

    Rect2 r = p_rect.abs();
    Vector2 top_left     = r.position;
    Vector2 top_right    = r.position + Vector2(r.size.x, 0.0f);
    Vector2 bottom_right = r.position + r.size;
    Vector2 bottom_left  = r.position + Vector2(0.0f, r.size.y);

    Vector2 uv_tl = p_src_rect.position;
    Vector2 uv_tr = p_src_rect.position + Vector2(p_src_rect.size.x, 0.0f);
    Vector2 uv_br = p_src_rect.position + p_src_rect.size;
    Vector2 uv_bl = p_src_rect.position + Vector2(0.0f, p_src_rect.size.y);

    check_batch_overflow(4, 6);

    uint32_t base_index = static_cast<uint32_t>(batched_vertices.size());

    batched_vertices.push_back({ top_left,     p_modulate, uv_tl });
    batched_vertices.push_back({ top_right,    p_modulate, uv_tr });
    batched_vertices.push_back({ bottom_right, p_modulate, uv_br });
    batched_vertices.push_back({ bottom_left,  p_modulate, uv_bl });

    batched_indices.push_back(base_index + 0);
    batched_indices.push_back(base_index + 1);
    batched_indices.push_back(base_index + 2);

    batched_indices.push_back(base_index + 2);
    batched_indices.push_back(base_index + 3);
    batched_indices.push_back(base_index + 0);

    // Vaciar este quad con su textura enlazada
    flush();

    // Restaurar la textura blanca para que las llamadas regulares de draw_rect/draw_line sigan funcionando
    glBindTexture(GL_TEXTURE_2D, rendering_device->get_default_white_texture());
}

void RenderingGeometry::draw_texture(const Rect2& p_rect, const RenderingTexture& p_texture, const Color& p_modulate, const Rect2& p_src_rect) {
    draw_texture(p_rect, p_texture.get_id(), p_modulate, p_src_rect);
}