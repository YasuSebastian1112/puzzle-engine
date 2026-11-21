#include <iostream>
#include "globals/rendering_global/rendering_global.h"
#include "classes/register_class_types.h"
#include "classes/2d/sprite_2d.h"
#include "classes/resources/texture_2d.h"

int main() {
    register_core_classes();

    auto& rg = RenderingGlobal::get();
    rg.init();

    auto win = rg.create_window(Size2i(800, 600), "Puzzle Engine - Sprite2D", Point2i(100, 100), Window::Type::Editor);
    if (!win) {
        return -1;
    }

    // Configurar Stretch estilo Godot (mantiene proporción al redimensionar)
    win->set_content_scale_mode(Viewport::StretchMode::CanvasItems);
    win->set_content_scale_aspect(Viewport::StretchAspect::Keep);

    // Cargar la textura (con el contexto OpenGL ya activo)
    Ref<Texture2D> tex = new Texture2D();
    bool loaded = tex->load("HELLBUNNY OG.png", false); // false = GL_NEAREST (pixel art nítido)
    if (!loaded) {
        // Respaldo si el directorio de trabajo es la raíz del proyecto
        loaded = tex->load("bin/HELLBUNNY OG.png", false);
    }

    if (!loaded) {
        std::cerr << "[Main] No se pudo cargar HELLBUNNY OG.png\n";
    }

    Sprite2D sprite("Hellbunny");
    sprite.set_texture(tex);
    sprite.set_centered(true);

    win->set_frame_callback([&](double delta) {
        (void)delta;

        // Mantener el sprite centrado en el espacio lógico del viewport
        Vector2 vp_size = win->get_viewport_size();
        sprite.set_position(vp_size * 0.5f);

        sprite._draw(&win->get_rendering_geometry());
    });

    rg.run(nullptr, RenderingGlobal::ClosePolicy::WhenPrimaryClosed);
    return 0;
}