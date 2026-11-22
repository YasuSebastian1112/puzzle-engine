#include <iostream>
#include "globals/rendering_global/rendering_global.h"
#include "classes/register_class_types.h"
#include "classes/node.h"
#include "classes/2d/sprite_2d.h"
#include "classes/resources/texture_2d.h"
#include "script/puzzle_cli/engine_console.h"

int main() {
    register_core_classes();

    auto& rg = RenderingGlobal::get();
    rg.init();

    auto win = rg.create_window(Size2i(800, 600), "Puzzle Engine - CLI + Sprite2D", Point2i(100, 100), Window::Type::Editor);
    if (!win) {
        return -1;
    }

    win->set_content_scale_mode(Viewport::StretchMode::CanvasItems);
    win->set_content_scale_aspect(Viewport::StretchAspect::Keep);

    
    Node root("Root");

    Ref<Texture2D> tex = new Texture2D();
    if (!tex->load("HELLBUNNY OG.png", false)) {
        tex->load("bin/HELLBUNNY OG.png", false);
    }

    Sprite2D sprite("Hellbunny");
    sprite.set_texture(tex);
    sprite.set_centered(true);
    sprite.set_position(Vector2(400.0f, 300.0f));
    root.add_child(&sprite);

    EngineConsole console;
    console.set_scene_root(&root);
    console.start();

    win->set_frame_callback([&](double delta) {
        (void)delta;
        console.process_commands();

        sprite._draw(&win->get_rendering_geometry());
    });

    rg.run(nullptr, RenderingGlobal::ClosePolicy::WhenPrimaryClosed);

    console.stop();
    return 0;
}