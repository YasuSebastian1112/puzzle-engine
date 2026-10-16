#include "rendering_geometry.h"

RenderingGeometry::RenderingGeometry(RenderingDevice* p_device)
    : rendering_device(p_device) {}

void RenderingGeometry::begin() {
    clear();
}

void RenderingGeometry::draw_line(const Vector2& p_from, const Vector2& p_to, const Color& p_color, float p_width) {
    Vector2 dir = (p_to - p_from).normalized();
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


void RenderingGeometry::draw_rect(const Rect2& p_rect, const Color& p_color, bool p_filled, float p_width) {
    Rect2 r = p_rect.abs(); // Asegura coordenadas positivas antes de triangular

    Vector2 top_left = r.position;
    Vector2 top_right = r.position + Vector2(r.size.x, 0.0f);
    Vector2 bottom_right = r.position + r.size;
    Vector2 bottom_left = r.position + Vector2(0.0f, r.size.y);

    if (p_filled) {
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

    float step = (MathDefs::PI * 2) / static_cast<float>(p_segments);

    if (p_filled) {
        uint32_t center_idx = static_cast<uint32_t>(batched_vertices.size());
        batched_vertices.push_back({ p_center, p_color, Vector2(0.5f, 0.5f) });

        for (int i = 0; i <= p_segments; ++i) {
            float angle = static_cast<float>(i) * step;
            Vector2 pos = p_center + Vector2(Math::cos(angle) * p_radius, Math::sin(angle) * p_radius);
            batched_vertices.push_back({ pos, p_color, Vector2(0.0f, 0.0f) });
        }

        for (int i = 1; i <= p_segments; ++i) {
            batched_indices.push_back(center_idx);
            batched_indices.push_back(center_idx + i);
            batched_indices.push_back(center_idx + i + 1);
        }
    } else {
        for (int i = 0; i < p_segments; ++i) {
            float a1 = static_cast<float>(i) * step;
            float a2 = static_cast<float>(i + 1) * step;

            Vector2 p1 = p_center + Vector2(Math::cos(a1) * p_radius, Math::sin(a1) * p_radius);
            Vector2 p2 = p_center + Vector2(Math::cos(a2) * p_radius, Math::sin(a2) * p_radius);

            draw_line(p1, p2, p_color, 1.0f);
        }
    }
}

void RenderingGeometry::draw_polyline(const std::vector<Vector2>& p_points, const Color& p_color, float p_width) {
    if (p_points.size() < 2) return;

    for (size_t i = 0; i < p_points.size() - 1; ++i) {
        draw_line(p_points[i], p_points[i + 1], p_color, p_width);
    }
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