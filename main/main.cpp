#include "globals/rendering_global/rendering_global.h"
#include "globals/rendering_global/device/rendering_geometry.h"
#include "core/mathematics/math_definitions.h"
#include "core/mathematics/curve.h" // O "core/mathematics/curve_bezier_2d.h"
#include <chrono>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <thread>
#include <mutex>
#include <algorithm>
#include <cmath>

// =========================================================================
// RENDERIZADOR DE TEXTO VECTORIAL (Stroke Font sin dependencias)
// =========================================================================
void draw_vector_char(RenderingGeometry& geom, char c, const Vector2& pos, float size, const Color& color) {
    auto L = [&](float x1, float y1, float x2, float y2) {
        geom.draw_line(
            Vector2(pos.x + x1 * size, pos.y + y1 * size),
            Vector2(pos.x + x2 * size, pos.y + y2 * size),
            color, 1.5f
        );
    };

    switch (std::toupper(c)) {
        case 'A': L(0,1, 0,0.3f); L(0,0.3f, 0.5f,0); L(0.5f,0, 1,0.3f); L(1,0.3f, 1,1); L(0,0.6f, 1,0.6f); break;
        case 'B': L(0,0, 0,1); L(0,0, 0.8f,0); L(0.8f,0, 0.8f,0.5f); L(0,0.5f, 0.8f,0.5f); L(0.8f,0.5f, 0.8f,1); L(0,1, 0.8f,1); break;
        case 'C': L(1,0, 0,0); L(0,0, 0,1); L(0,1, 1,1); break;
        case 'D': L(0,0, 0,1); L(0,0, 0.7f,0); L(0.7f,0, 1,0.4f); L(1,0.4f, 1,0.6f); L(1,0.6f, 0.7f,1); L(0.7f,1, 0,1); break;
        case 'E': L(1,0, 0,0); L(0,0, 0,1); L(0,1, 1,1); L(0,0.5f, 0.7f,0.5f); break;
        case 'F': L(1,0, 0,0); L(0,0, 0,1); L(0,0.5f, 0.7f,0.5f); break;
        case 'G': L(1,0, 0,0); L(0,0, 0,1); L(0,1, 1,1); L(1,1, 1,0.5f); L(1,0.5f, 0.5f,0.5f); break;
        case 'H': L(0,0, 0,1); L(1,0, 1,1); L(0,0.5f, 1,0.5f); break;
        case 'I': L(0.5f,0, 0.5f,1); L(0.2f,0, 0.8f,0); L(0.2f,1, 0.8f,1); break;
        case 'L': L(0,0, 0,1); L(0,1, 1,1); break;
        case 'M': L(0,1, 0,0); L(0,0, 0.5f,0.6f); L(0.5f,0.6f, 1,0); L(1,0, 1,1); break;
        case 'N': L(0,1, 0,0); L(0,0, 1,1); L(1,1, 1,0); break;
        case 'O': L(0,0, 1,0); L(1,0, 1,1); L(1,1, 0,1); L(0,1, 0,0); break;
        case 'P': L(0,1, 0,0); L(0,0, 1,0); L(1,0, 1,0.5f); L(1,0.5f, 0,0.5f); break;
        case 'R': L(0,1, 0,0); L(0,0, 1,0); L(1,0, 1,0.5f); L(1,0.5f, 0,0.5f); L(0.3f,0.5f, 1,1); break;
        case 'S': L(1,0, 0,0); L(0,0, 0,0.5f); L(0,0.5f, 1,0.5f); L(1,0.5f, 1,1); L(1,1, 0,1); break;
        case 'T': L(0,0, 1,0); L(0.5f,0, 0.5f,1); break;
        case 'U': L(0,0, 0,1); L(0,1, 1,1); L(1,1, 1,0); break;
        case 'V': L(0,0, 0.5f,1); L(0.5f,1, 1,0); break;
        case 'X': L(0,0, 1,1); L(1,0, 0,1); break;
        case 'Y': L(0,0, 0.5f,0.5f); L(1,0, 0.5f,0.5f); L(0.5f,0.5f, 0.5f,1); break;
        case 'Z': L(0,0, 1,0); L(1,0, 0,1); L(0,1, 1,1); break;
        case '0': L(0,0, 1,0); L(1,0, 1,1); L(1,1, 0,1); L(0,1, 0,0); L(0,1, 1,0); break;
        case '1': L(0.2f,0.3f, 0.5f,0); L(0.5f,0, 0.5f,1); L(0.2f,1, 0.8f,1); break;
        case '2': L(0,0, 1,0); L(1,0, 1,0.5f); L(1,0.5f, 0,1); L(0,1, 1,1); break;
        case '3': L(0,0, 1,0); L(1,0, 1,1); L(0,1, 1,1); L(0.3f,0.5f, 1,0.5f); break;
        case '4': L(0,0, 0,0.5f); L(0,0.5f, 1,0.5f); L(0.8f,0, 0.8f,1); break;
        case ':': L(0.5f,0.3f, 0.5f,0.35f); L(0.5f,0.7f, 0.5f,0.75f); break;
        case '-': L(0.2f,0.5f, 0.8f,0.5f); break;
        default: break;
    }
}

void draw_vector_text(RenderingGeometry& geom, const std::string& text, const Vector2& start_pos, float char_height, const Color& color) {
    float char_width = char_height * 0.65f;
    float spacing = char_height * 0.25f;
    Vector2 cur = start_pos;
    for (char c : text) {
        if (c != ' ') {
            draw_vector_char(geom, c, cur, char_width, color);
        }
        cur.x += char_width + spacing;
    }
}

// =========================================================================
// GESTOR DE CURVAS Y EVALUACIÓN Y(X)
// =========================================================================
struct CurveSlot {
    std::vector<BezierPoint2D> points;

    void add_point_sorted(const BezierPoint2D& pt) {
        points.push_back(pt);
        std::sort(points.begin(), points.end(), [](const BezierPoint2D& a, const BezierPoint2D& b) {
            return a.position.x < b.position.x;
        });
    }

    void clear() {
        points.clear();
    }

    // Evalúa el valor Y para un progreso normalizado de X entre [0, 1]
    float sample_y_at_x(float target_x) const {
        if (points.empty()) return 0.0f;
        if (points.size() == 1) return points[0].position.y;
        if (target_x <= points.front().position.x) return points.front().position.y;
        if (target_x >= points.back().position.x) return points.back().position.y;

        // Localizar el segmento correspondiente
        for (size_t i = 0; i < points.size() - 1; ++i) {
            if (target_x >= points[i].position.x && target_x <= points[i + 1].position.x) {
                float seg_w = points[i + 1].position.x - points[i].position.x;
                if (seg_w <= 0.00001f) return points[i].position.y;

                float local_t = (target_x - points[i].position.x) / seg_w;
                float u = 1.0f - local_t;

                float p0 = points[i].position.y;
                float p1 = points[i].position.y + points[i].out.y;
                float p3 = points[i + 1].position.y;
                float p2 = points[i + 1].position.y + points[i + 1].in.y;

                return (u * u * u * p0) + (3.0f * u * u * local_t * p1) + (3.0f * u * local_t * local_t * p2) + (local_t * local_t * local_t * p3);
            }
        }
        return points.back().position.y;
    }
};

// Slots de curvas globales compartidos
std::mutex g_curve_mutex;
CurveSlot g_curves[4];
bool g_app_running = true;

// Hilo de escucha en la consola
void console_worker() {
    std::cout << "\n=======================================================\n";
    std::cout << " COMANDOS DISPONIBLES:\n";
    std::cout << "  curveN clear\n";
    std::cout << "  curveN point x y in out\n";
    std::cout << "  (Ejemplos: curve1 point 0 0 0 0.5 | curve1 point 1 1 -0.5 0)\n";
    std::cout << "=======================================================\n\n";

    while (g_app_running) {
        std::cout << "> ";
        std::string line;
        if (!std::getline(std::cin, line)) break;
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string cmd_target, action;
        ss >> cmd_target >> action;

        int curve_idx = -1;
        if (cmd_target == "curve1") curve_idx = 0;
        else if (cmd_target == "curve2") curve_idx = 1;
        else if (cmd_target == "curve3") curve_idx = 2;
        else if (cmd_target == "curve4") curve_idx = 3;

        if (curve_idx == -1) {
            std::cout << "[Error] Curva desconocida. Usa curve1, curve2, curve3 o curve4\n";
            continue;
        }

        std::lock_guard<std::mutex> lock(g_curve_mutex);

        if (action == "clear") {
            g_curves[curve_idx].clear();
            std::cout << "[OK] " << cmd_target << " limpiada.\n";
        } else if (action == "point") {
            std::vector<float> values;
            float val;
            while (ss >> val) values.push_back(val);

            if (values.size() >= 4) {
                float px = values[0];
                float py = values[1];
                Vector2 in_vec(-0.1f, -values[2] * 0.1f);
                Vector2 out_vec(0.1f, values[3] * 0.1f);

                if (values.size() >= 6) {
                    in_vec = Vector2(values[2], values[3]);
                    out_vec = Vector2(values[4], values[5]);
                }

                g_curves[curve_idx].add_point_sorted(BezierPoint2D(Vector2(px, py), in_vec, out_vec));
                std::cout << "[OK] Punto agregado a " << cmd_target << " en (" << px << ", " << py << ")\n";
            } else {
                std::cout << "[Error] Formato: " << cmd_target << " point x y in out\n";
            }
        }
    }
}

// Dibuja un plano cartesiano unitario [0, 1] y la curva horneada
void render_plane(RenderingGeometry& geom, const Vector2& origin, const Vector2& size, const CurveSlot& slot, float current_x, float current_y) {
    const Color bg_color(0.12f, 0.13f, 0.16f, 1.0f);
    const Color grid_color(0.20f, 0.22f, 0.28f, 1.0f);
    const Color axis_color(0.45f, 0.48f, 0.58f, 1.0f);

    geom.draw_rect(Rect2(origin, size), bg_color, true);
    

    // Cuadrícula interior (subdivisiones cada 0.2)
    for (int i = 1; i < 5; ++i) {
        float f = i * 0.2f;
        geom.draw_line(Vector2(origin.x + f * size.x, origin.y), Vector2(origin.x + f * size.x, origin.y + size.y), grid_color, 1.0f);
        geom.draw_line(Vector2(origin.x, origin.y + f * size.y), Vector2(origin.x + size.x, origin.y + f * size.y), grid_color, 1.0f);
    }

    // Marco exterior
    geom.draw_rect(Rect2(origin, size), axis_color, false, 1.5f);

    // Dibujar la curva
    if (slot.points.size() >= 2) {
        std::vector<Vector2> screen_pts;
        const int samples = 50;
        for (int i = 0; i <= samples; ++i) {
            float tx = static_cast<float>(i) / static_cast<float>(samples);
            float ty = slot.sample_y_at_x(tx);
            screen_pts.push_back(Vector2(origin.x + tx * size.x, origin.y + (1.0f - ty) * size.y));
        }
        geom.draw_polyline(screen_pts, Color(0.15f, 0.85f, 1.0f, 1.0f), 2.2f);
    }

    // Dibujar puntos de anclaje (nodos)
    for (const auto& pt : slot.points) {
        Vector2 screen_pos(origin.x + pt.position.x * size.x, origin.y + (1.0f - pt.position.y) * size.y);
        geom.draw_circle(screen_pos, 4.0f, Color(1.0f, 0.8f, 0.2f, 1.0f), true, 12);
    }

    // Líneas de rastreo en X e Y
    if (!slot.points.empty()) {
        float track_screen_x = origin.x + current_x * size.x;
        float track_screen_y = origin.y + (1.0f - current_y) * size.y;

        // Línea vertical (X)
        geom.draw_line(Vector2(track_screen_x, origin.y), Vector2(track_screen_x, origin.y + size.y), Color(1.0f, 0.3f, 0.3f, 0.7f), 1.2f);
        // Línea horizontal (Y)
        geom.draw_line(Vector2(origin.x, track_screen_y), Vector2(origin.x + size.x, track_screen_y), Color(0.3f, 1.0f, 0.4f, 0.7f), 1.2f);
        // Punto de intersección actual
        geom.draw_circle(Vector2(track_screen_x, track_screen_y), 4.5f, Color(1.0f, 1.0f, 1.0f, 1.0f), true, 12);
    }
}

// =========================================================================
// MAIN
// =========================================================================
int main() {
    RenderingGlobal& renderer = RenderingGlobal::get();

    const int screen_w = 1280;
    const int screen_h = 720;

    auto primary_window = renderer.create_window(
        Size2i(screen_w, screen_h),
        "Puzzle Engine - Test de curvas",
        Point2i(60, 60),
        Window::Type::Game
    );

    if (!primary_window) return -1;

    RenderingGeometry geometry(&primary_window->get_rendering_device());

    // Curvas iniciales predeterminadas para que no inicie vacío
    {
        std::lock_guard<std::mutex> lock(g_curve_mutex);
        // Curva 1: Ease-in-out suave
        g_curves[0].add_point_sorted(BezierPoint2D(Vector2(0, 0), Vector2(0, 0), Vector2(0.15f, 0.0f)));
        g_curves[0].add_point_sorted(BezierPoint2D(Vector2(1, 1), Vector2(-0.15f, 0.0f), Vector2(0, 0)));

        // Curva 2: Rebote / Anticipación
        g_curves[1].add_point_sorted(BezierPoint2D(Vector2(0, 0), Vector2(0, 0), Vector2(0.1f, -0.2f)));
        g_curves[1].add_point_sorted(BezierPoint2D(Vector2(0.7f, 1.15f), Vector2(-0.1f, 0.0f), Vector2(0.1f, 0.0f)));
        g_curves[1].add_point_sorted(BezierPoint2D(Vector2(1, 1), Vector2(-0.1f, 0.0f), Vector2(0, 0)));

        // Curva 3: Pulso de color
        g_curves[2].add_point_sorted(BezierPoint2D(Vector2(0, 0), Vector2(0, 0), Vector2(0.2f, 0.8f)));
        g_curves[2].add_point_sorted(BezierPoint2D(Vector2(1, 1), Vector2(-0.2f, 0.0f), Vector2(0, 0)));

        // Curva 4: Rotación ondulada
        g_curves[3].add_point_sorted(BezierPoint2D(Vector2(0, 0), Vector2(0, 0), Vector2(0.1f, 0.6f)));
        g_curves[3].add_point_sorted(BezierPoint2D(Vector2(0.5f, 0.8f), Vector2(-0.1f, 0.0f), Vector2(0.1f, 0.0f)));
        g_curves[3].add_point_sorted(BezierPoint2D(Vector2(1, 0.2f), Vector2(-0.1f, 0.0f), Vector2(0, 0)));
    }

    std::thread input_thread(console_worker);

    auto prev_time = std::chrono::high_resolution_clock::now();
    float anim_timer = 0.0f;
    const float duration_active = 3.0f;
    const float duration_pause = 1.0f;
    const float total_period = duration_active + duration_pause;

    while (!primary_window->should_close()) {
        auto cur_time = std::chrono::high_resolution_clock::now();
        float delta = std::chrono::duration<float, std::chrono::seconds::period>(cur_time - prev_time).count();
        prev_time = cur_time;

        anim_timer += delta;
        if (anim_timer >= total_period) {
            anim_timer = std::fmod(anim_timer, total_period);
        }

        // Progreso horizontal x entre [0.0, 1.0]
        float progress_x = (anim_timer <= duration_active) ? (anim_timer / duration_active) : 1.0f;

        renderer.poll_events();
        renderer.clear_screen(Color(0.07f, 0.08f, 0.10f, 1.0f));

        geometry.begin();

        // Configuración del grid 2x2 para los 4 planos
        const float margin_left = 60.0f;
        const float margin_top = 50.0f;
        const float plane_w = 210.0f;
        const float plane_h = 210.0f;
        const float col_stride = 620.0f;
        const float row_stride = 330.0f;
        const float demo_offset_x = 290.0f; // Espacio a la derecha del plano para el objeto

        CurveSlot local_slots[4];
        {
            std::lock_guard<std::mutex> lock(g_curve_mutex);
            for (int i = 0; i < 4; ++i) local_slots[i] = g_curves[i];
        }

        for (int i = 0; i < 4; ++i) {
            int col = i % 2;
            int row = i / 2;

            Vector2 plane_pos(margin_left + col * col_stride, margin_top + row * row_stride);
            Vector2 plane_size(plane_w, plane_h);
            Vector2 demo_center(plane_pos.x + demo_offset_x + 90.0f, plane_pos.y + plane_h * 0.5f);

            float eval_y = local_slots[i].sample_y_at_x(progress_x);

            // 1. Títulos Vectoriales
            std::string title = (i == 0) ? "1: TRANSLACION EN Y" :
                                (i == 1) ? "2: ESCALA" :
                                (i == 2) ? "3: COLOR" :
                                        "4: ROTACION";
            draw_vector_text(geometry, title, Vector2(plane_pos.x, plane_pos.y - 25.0f), 13.0f, Color(0.85f, 0.88f, 0.95f, 1.0f));

            // 2. Plano cartesiano unitario con líneas guía de posición
            render_plane(geometry, plane_pos, plane_size, local_slots[i], progress_x, eval_y);

            // 3. Demostración interactiva a la derecha
            if (!local_slots[i].points.empty()) {
                switch (i) {
                    case 0: { // Plano 1: Movimiento arriba - abajo
                        float move_travel = 160.0f;
                        float rect_y = (plane_pos.y + plane_h - 30.0f) - (eval_y * move_travel);
                        geometry.draw_rect(
                            Rect2(Vector2(demo_center.x - 30.0f, rect_y - 30.0f), Vector2(60.0f, 60.0f)),
                            Color(0.2f, 0.7f, 1.0f, 1.0f),
                            true
                        );
                        break;
                    }
                    case 1: { // Plano 2: Escalado
                        float clamped_scale = Math::clamp(eval_y, 0.0f, 1.8f);
                        float base_side = 110.0f * clamped_scale;
                        geometry.draw_rect(
                            Rect2(
                                Vector2(demo_center.x - base_side * 0.5f, demo_center.y - base_side * 0.5f),
                                Vector2(base_side, base_side)
                            ),
                            Color(1.0f, 0.45f, 0.2f, 1.0f),
                            true
                        );
                        break;
                    }
                    case 2: { // Plano 3: Color
                        float intensity = Math::clamp(eval_y, 0.0f, 1.0f);
                        Color dynamic_col(0.2f * intensity, 0.95f * intensity, 0.45f * intensity, 1.0f);
                        geometry.draw_rect(
                            Rect2(Vector2(demo_center.x - 45.0f, demo_center.y - 45.0f), Vector2(90.0f, 90.0f)),
                            dynamic_col,
                            true
                        );
                        break;
                    }
                    case 3: { // Plano 4: Rotación y oscilación
                        float angle = eval_y * MathDefs::TAU;
                        float half_sz = 35.0f;
                        Vector2 corners[4] = {
                            Vector2(-half_sz, -half_sz),
                            Vector2(half_sz, -half_sz),
                            Vector2(half_sz, half_sz),
                            Vector2(-half_sz, half_sz)
                        };

                        float cos_a = Math::cos(angle);
                        float sin_a = Math::sin(angle);
                        std::vector<Vector2> rotated_poly;
                        rotated_poly.reserve(5);

                        for (int k = 0; k < 4; ++k) {
                            float rx = corners[k].x * cos_a - corners[k].y * sin_a;
                            float ry = corners[k].x * sin_a + corners[k].y * cos_a;
                            rotated_poly.push_back(Vector2(demo_center.x + rx, demo_center.y + ry));
                        }
                        // Cerramos el cuadrado conectando de vuelta al primer vértice
                        rotated_poly.push_back(rotated_poly[0]);

                        // Usamos draw_polyline que ya está en tu batcher
                        geometry.draw_polyline(rotated_poly, Color(0.85f, 0.35f, 0.95f, 1.0f), 2.5f);
                        break;
                    }
                }
            }
        }

        geometry.flush();
        primary_window->swap_buffers();
    }

    g_app_running = false;
    input_thread.detach();

    renderer.shutdown();
    return 0;
}