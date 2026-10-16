// ========================================================================= //
// Puzzle Engine - 2D/3D Hybrid Game Engine
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// ========================================================================= //

#include "curve.h"

void CurveBezier2D::add_point(const Vector2& p_pos, const Vector2& p_in, const Vector2& p_out) {
    points.emplace_back(p_pos, p_in, p_out);
}

void CurveBezier2D::remove_point(size_t p_index) {
    if (p_index < points.size()) {
        points.erase(points.begin() + p_index);
    }
}

/*
 * Algoritmo de Bézier Cúbico por Segmento:
 * 
 * Para un intervalo entre dos nodos sucesivos (P_A = points[i] y P_B = points[i+1]):
 * - El punto de inicio es p0 = P_A.position
 * - El primer control es  p1 = P_A.position + P_A.out (tangente de salida relativa)
 * - El segundo control es p2 = P_B.position + P_B.in  (tangente de entrada relativa)
 * - El punto final es     p3 = P_B.position
 *
 * Se evalúa mediante la base polinómica de Bernstein de tercer grado para t in [0, 1]:
 *   B(t) = (1 - t)^3 * p0 
 *        + 3 * (1 - t)^2 * t * p1 
 *        + 3 * (1 - t) * t^2 * p2 
 *        + t^3 * p3
 */
Vector2 CurveBezier2D::sample_segment(size_t p_segment_idx, float p_t) const {
    if (p_segment_idx + 1 >= points.size()) {
        return Vector2();
    }

    // Clamp de seguridad para t en el intervalo unitario [0.0, 1.0]
    float t = Math::clamp(p_t, 0.0f, 1.0f);
    float u = 1.0f - t;

    // Puntos de control en coordenadas absolutas del espacio mundo/pantalla
    const Vector2& p0 = points[p_segment_idx].position;
    Vector2 p1 = p0 + points[p_segment_idx].out;
    const Vector2& p3 = points[p_segment_idx + 1].position;
    Vector2 p2 = p3 + points[p_segment_idx + 1].in;

    // Coeficientes polinómicos de Bernstein
    float uu = u * u;
    float uuu = uu * u;
    float tt = t * t;
    float ttt = tt * t;

    float b0 = uuu;              // (1 - t)^3
    float b1 = 3.0f * uu * t;    // 3 * (1 - t)^2 * t
    float b2 = 3.0f * u * tt;    // 3 * (1 - t) * t^2
    float b3 = ttt;              // t^3

    return (p0 * b0) + (p1 * b1) + (p2 * b2) + (p3 * b3);
}

/*
 * Algoritmo de Horneado (Bake):
 * 
 * Transforma la curva analítica continua en un conjunto discreto de vértices (polilínea)
 * optimizado para ser consumido directamente por el batcher de renderizado (RenderingGeometry).
 * 
 * Muestrea uniformemente cada segmento cúbico en 'p_segments_per_curve' sub-tramos,
 * evitando la duplicación del vértice compartido entre el final del segmento i y el
 * inicio del segmento i+1.
 */
std::vector<Vector2> CurveBezier2D::bake(int p_segments_per_curve) const {
    std::vector<Vector2> baked_points;

    if (points.size() < 2) {
        return baked_points;
    }

    int steps = Math::max(p_segments_per_curve, 1);
    size_t segment_count = points.size() - 1;

    // Reserva anticipada de memoria contigua para evitar reasignaciones en el vector
    baked_points.reserve((segment_count * steps) + 1);

    for (size_t i = 0; i < segment_count; ++i) {
        for (int step = 0; step <= steps; ++step) {
            // Se omite t = 0 a partir del segundo segmento para no duplicar vértices adyacentes
            if (step == 0 && !baked_points.empty()) {
                continue;
            }

            float t = static_cast<float>(step) / static_cast<float>(steps);
            baked_points.push_back(sample_segment(i, t));
        }
    }

    return baked_points;
}