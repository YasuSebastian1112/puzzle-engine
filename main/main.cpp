

// Plataforma y Ventana
#include "globals/rendering_global/window.h"

// Reflexión y Registro de Clases
#include "classes/register_class_types.h"

// Nodos y Recursos
#include "classes/node.h"
#include "classes/2d/node_2d.h"
#include "classes/2d/camera_2d.h"
#include "classes/2d/sprite_2d.h"
#include "classes/resources/texture_2d.h"

// Consola Interactiva
#include "script/puzzle_cli/engine_console.h"

int main() {
    // 1. Inicializar ClassDB y tipos centrales del motor
    register_core_classes();

    // 2. Inicializar ventana y contexto gráfico
    Window window;
    if (!window.initialize("Puzzle Engine - Rueditas del CLI UwU", Size2i(1280, 720), Point2i(100, 100), Window::Type::Editor)) {
        return -1;
    }

    // 3. Montar el árbol de escena (Scene Tree)
    Node root("root");

    Node2D world("Node2D");
    root.add_child(&world);

    Camera2D camera;
    camera.set_name("Camera2D");
    camera.set_position_smoothing_enabled(true);
    camera.set_position_smoothing_speed(6.0f);
    world.add_child(&camera);

    Ref<Texture2D> mario_texture = new Texture2D();
    mario_texture->load("mario.png");

    Sprite2D mario("Sprite2D");
    mario.set_texture(mario_texture);
    mario.set_position(Vector2(0.0f, 0.0f));
    world.add_child(&mario);

    // Inicializar nodos en cascada
    root.propagate_ready();

    // 4. Iniciar Puzzle CLI conectado a la escena
    EngineConsole console;
    console.set_scene_root(&root);
    console.start();

    GLFWwindow* native_win = static_cast<GLFWwindow*>(window.get_rendering_device().get_native_handle());

    // 5. Bucle principal
window.run([&](double delta) {
        // Despachar comandos de la consola en el hilo de render
        console.process_commands();

        RenderingGeometry& geom = window.get_rendering_geometry();
        Size2i vp_size = window.get_size();

        // Aplicar WASD solo si hay teclas presionadas
        float move_speed = 400.0f * static_cast<float>(delta);
        Vector2 input_dir(0.0f, 0.0f);

        if (glfwGetKey(native_win, GLFW_KEY_D) == GLFW_PRESS) input_dir.x += 1.0f;
        if (glfwGetKey(native_win, GLFW_KEY_A) == GLFW_PRESS) input_dir.x -= 1.0f;
        if (glfwGetKey(native_win, GLFW_KEY_S) == GLFW_PRESS) input_dir.y += 1.0f;
        if (glfwGetKey(native_win, GLFW_KEY_W) == GLFW_PRESS) input_dir.y -= 1.0f;

        if (input_dir.x != 0.0f || input_dir.y != 0.0f) {
            camera.translate(input_dir.normalized() * move_speed);
        }

        // Zoom solo si presionas Q o E
        float zoom_speed = 1.0f * static_cast<float>(delta);
        if (glfwGetKey(native_win, GLFW_KEY_E) == GLFW_PRESS) {
            camera.set_zoom(camera.get_zoom() + Vector2(zoom_speed, zoom_speed));
        }
        if (glfwGetKey(native_win, GLFW_KEY_Q) == GLFW_PRESS) {
            camera.set_zoom(camera.get_zoom() - Vector2(zoom_speed, zoom_speed));
        }

        camera.update_camera(delta, vp_size);
        root.propagate_process(static_cast<float>(delta));

        // Proyectar el sprite con respecto a la cámara
        Vector2 world_pos = mario.get_position();
        Vector2 base_scale = mario.get_scale();
        Vector2 active_zoom = camera.get_zoom();

        Vector2 screen_pos = camera.world_to_screen(world_pos, vp_size);
        mario.set_position(screen_pos);
        mario.set_scale(Vector2(base_scale.x * active_zoom.x, base_scale.y * active_zoom.y));

        // Dibujar el árbol proyectado
        world.propagate_draw(&geom);

        // Restaurar posición y escala originales en el espacio de mundo
        mario.set_position(world_pos);
        mario.set_scale(base_scale);
    });

    console.stop();
    return 0;
}