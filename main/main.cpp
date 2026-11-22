#include <iostream>
#include <vector>
#include <random>
#include <string>
#include "globals/rendering_global/rendering_global.h"
#include "classes/register_class_types.h"
#include "classes/2d/sprite_2d.h"
#include "classes/resources/texture_2d.h"
#include "classes/resources/font_resource.h"

struct Bunny {
    Sprite2D sprite;
    Vector2 velocity;
    float rot_speed = 0.0f;
};

int main() {
    register_core_classes();

    auto& rg = RenderingGlobal::get();
    rg.init();

    auto win = rg.create_window(Size2i(800, 600), "Puzzle Engine - HellbunnyMark", Point2i(100, 100), Window::Type::Editor);
    if (!win) {
        return -1;
    }

    // Desactivar VSync para medir rendimiento real sin tope de monitor
    win->get_rendering_device().set_vsync(false);
    win->set_content_scale_mode(Viewport::StretchMode::CanvasItems);
    win->set_content_scale_aspect(Viewport::StretchAspect::Keep);

    // Cargar textura y fuente del HUD
    Ref<Texture2D> tex = new Texture2D();
    if (!tex->load("HELLBUNNY OG.png", false)) {
        if (!tex->load("bin/HELLBUNNY OG.png", false)) {
            std::cerr << "[Main] No se pudo cargar HELLBUNNY OG.png\n";
        }
    }
    Ref<FontResource> hud_font = FontResource::get_default_font(16);

    // Configurar 5,000 sprites (con MAX_BATCH_VERTICES = 8192 caben 2,048 quads por draw call)
    constexpr int SPRITE_COUNT = 100000;
    std::vector<Bunny> bunnies(SPRITE_COUNT); // Construye todos in-place, sin copiar ni mover

    std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist_x(40.0f, 760.0f);
    std::uniform_real_distribution<float> dist_y(40.0f, 560.0f);
    std::uniform_real_distribution<float> dist_vel(-220.0f, 220.0f);
    std::uniform_real_distribution<float> dist_rot(-3.0f, 3.0f);

    for (auto& b : bunnies) {
        b.sprite.set_texture(tex);
        b.sprite.set_centered(true);
        b.sprite.set_position(Vector2(dist_x(rng), dist_y(rng)));
        b.sprite.set_scale(Vector2(0.15f, 0.15f));
        b.velocity = Vector2(dist_vel(rng), dist_vel(rng));
        b.rot_speed = dist_rot(rng);
    }

    double fps_timer = 0.0;
    int frame_counter = 0;
    int current_fps = 0;
    uint32_t last_draw_calls = 0;

    win->set_frame_callback([&](double delta) {
        float dt = static_cast<float>(delta);
        Vector2 vp = win->get_viewport_size();
        auto& geom = win->get_rendering_geometry();

        // Actualizar y dibujar todos los sprites
        for (auto& b : bunnies) {
            Vector2 pos = b.sprite.get_position() + (b.velocity * dt);

            if (pos.x < 0.0f) { pos.x = 0.0f; b.velocity.x = -b.velocity.x; }
            else if (pos.x > vp.x) { pos.x = vp.x; b.velocity.x = -b.velocity.x; }

            if (pos.y < 0.0f) { pos.y = 0.0f; b.velocity.y = -b.velocity.y; }
            else if (pos.y > vp.y) { pos.y = vp.y; b.velocity.y = -b.velocity.y; }

            b.sprite.set_position(pos);
            b.sprite.set_rotation(b.sprite.get_rotation() + b.rot_speed * dt);
            b.sprite._draw(&geom);
        }

        // Medición de FPS cada 0.25 s
        fps_timer += delta;
        frame_counter++;
        if (fps_timer >= 0.25) {
            current_fps = static_cast<int>(frame_counter / fps_timer);
            fps_timer = 0.0;
            frame_counter = 0;
        }

        // HUD encima (rectángulo + texto, todo entra en el mismo batcher)
        geom.draw_rect(Rect2(10.0f, 10.0f, 240.0f, 74.0f), Color(0.05f, 0.05f, 0.08f, 0.85f), true);
        geom.draw_rect(Rect2(10.0f, 10.0f, 240.0f, 74.0f), Color(0.35f, 0.38f, 0.48f, 1.0f), false, 1.0f);

        if (hud_font.is_valid()) {
            std::string stats =
                "FPS: " + std::to_string(current_fps) + "\n" +
                "Sprites: " + std::to_string(SPRITE_COUNT) + "\n" +
                "Draw Calls: " + std::to_string(last_draw_calls);
            hud_font->draw_text(&geom, Vector2(20.0f, 18.0f), stats.c_str(), Color(0.95f, 0.96f, 0.98f, 1.0f));
        }

        // +1 por el flush() final que ejecuta Window al salir de este callback
        last_draw_calls = geom.get_draw_calls() + 1;
    });

    rg.run(nullptr, RenderingGlobal::ClosePolicy::WhenPrimaryClosed);
    return 0;
}