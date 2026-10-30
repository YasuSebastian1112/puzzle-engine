#include "globals/rendering_global/window.h"
#include "globals/rendering_global/device/rendering_geometry.h"
#include "core/threading/job_system.h"
#include "core/mathematics/math_functions.h"
#include <vector>

int main() {
    Window window;
    if (!window.initialize("Puzzle Engine - Clean Architecture", Size2i(1280, 720), Point2i(100, 100), Window::Type::Game)) {
        return -1;
    }

    window.get_rendering_device().init_geometry_pipeline();
    RenderingGeometry geometry(&window.get_rendering_device());
    JobSystem::get().initialize();

    float rotation_angle = 0.0f;
    const float quad_half_size = 40.0f;

    std::vector<Vector2> rotated_quad(4);

    window.run([&](double delta) {
        Size2i win_sz = window.get_size();

        // 1. Simulación
        rotation_angle += static_cast<float>(delta) * 2.0f;
        float center_x = win_sz.width * 0.5f;
        float center_y = win_sz.height * 0.5f;

        Vector2 local_corners[4] = {
            Vector2(-quad_half_size, -quad_half_size),
            Vector2( quad_half_size, -quad_half_size),
            Vector2( quad_half_size,  quad_half_size),
            Vector2(-quad_half_size,  quad_half_size)
        };

        float cos_r = static_cast<float>(Math::cos(rotation_angle));
        float sin_r = static_cast<float>(Math::sin(rotation_angle));

        for (int i = 0; i < 4; ++i) {
            float rx = local_corners[i].x * cos_r - local_corners[i].y * sin_r;
            float ry = local_corners[i].x * sin_r + local_corners[i].y * cos_r;
            rotated_quad[i] = Vector2(center_x + rx, center_y + ry);
        }

        // 2. Renderizado
        window.clear(Color(0.06f, 0.06f, 0.09f, 1.0f));

        geometry.begin();
        geometry.draw_polygon(rotated_quad, Color(0.15f, 0.75f, 1.0f, 1.0f));
        window.draw_title_bar(geometry);
        geometry.flush();
    });

    JobSystem::get().shutdown();
    return 0;
}