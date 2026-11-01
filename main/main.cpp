#include "globals/rendering_global/window.h"
#include "core/scene_tree.h"
#include "classes/2d/sprite_2d.h"
#include "classes/resources/texture_2d.h"

int main() {
    Window window;
    if (!window.initialize("Puzzle Engine - Line Test", Size2i(1024, 640), Point2i(100, 100), Window::Type::Editor)) {
        return -1;
    }

    Ref<Texture2D> mario_texture = new Texture2D();
    mario_texture->load("mario.png");

    Node2D* root_node = new Node2D("Root2D");
    Sprite2D* mario = new Sprite2D("Mario");
    mario->set_texture(mario_texture);
    mario->set_position(Vector2(200.0f, 200.0f));
    root_node->add_child(mario);

    SceneTree& tree = SceneTree::get();
    tree.set_root(root_node);

    window.run([&](double delta) {
        tree.iteration(static_cast<float>(delta));

        RenderingGeometry& geom = window.get_rendering_geometry();

        // 1. Dibujar el árbol de escenas existente
        Spatial2D* visual_root = Object::cast_to<Spatial2D>(tree.get_root());
        if (visual_root) {
            visual_root->propagate_draw(&geom);
        }

        // 2. Línea diagonal de prueba:
        // Origen: (0, 0) local del viewport activo del juego
        // Destino: esquina inferior derecha exacta según las dimensiones del Viewport
    Size2i win_size = window.get_size(); // Retorna 1024x640 exactos de área de juego
    Vector2 start_point(0.0f, 0.0f);
    Vector2 end_point(static_cast<float>(win_size.width), static_cast<float>(win_size.height));

    geom.draw_line(start_point, end_point, Color(0.0f, 1.0f, 0.2f, 1.0f), 2.0f);

        // Color verde brillante para distinguir el trazo claramente sobre el fondo oscuro
        geom.draw_line(start_point, end_point, Color(0.0f, 1.0f, 0.2f, 1.0f), 2.0f);
    });

    tree.shutdown();
    return 0;
}