#include <iostream>

// Plataforma y Ventana
#include "globals/rendering_global/window.h"

// Reflexión y Registro de Clases
#include "classes/register_class_types.h"

// Nodos 2D y Recursos
#include "classes/node.h"
#include "classes/2d/node_2d.h"
#include "classes/2d/camera_2d.h"
#include "classes/2d/sprite_2d.h"
#include "classes/resources/texture_2d.h"

// Interfaz (UI), Entrada y Texto Tipográfico
#include "classes/interface/button.h"
#include "classes/interface/label.h"
#include "globals/rendering_global/text/rendering_text.h"
#include "globals/input/input.h"

// Consola Interactiva
#include "script/puzzle_cli/engine_console.h"

int main() {
    // 1. Inicializar ClassDB y tipos centrales del motor
    register_core_classes();

    // 2. Inicializar ventana y subsistema gráfico (conecta Input automáticamente)
    Window window;
    if (!window.initialize("Puzzle Engine - Interface & Input", Size2i(1280, 720), Point2i(100, 100), Window::Type::Editor)) {
        return -1;
    }

    // 3. Montar el árbol de escena (Scene Tree)
    Node root("root");

    // Árbol de Mundo 2D (afectado por la cámara)
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

    // Árbol de Interfaz de Usuario (UI fija en coordenadas de pantalla)
    Interface ui_root("UI");
    root.add_child(&ui_root);

    Button my_button("TestButton");
    my_button.set_position(Vector2(30.0f, 30.0f));
    my_button.set_size(Vector2(160.0f, 40.0f));
    my_button.set_text("Toggle Flip Mario");

    // Callback al hacer clic en el botón
    my_button.set_on_pressed([&mario]() {
        bool current_flip = mario.is_flipped_h();
        mario.set_flip_h(!current_flip);
        std::cout << "\033[38;2;74;222;40m[UI] Click detectado en el boton! Mario flip_h = " 
                  << (!current_flip ? "true" : "false") << "\033[0m\n";
    });

    ui_root.add_child(&my_button);

    // --- Carga de JetBrains Mono y Creación del Label superior derecho ---
    Ref<RenderingText> jetbrains_font = new RenderingText();
    // Ajusta la ruta a donde guardaste el archivo .ttf
    if (!jetbrains_font->load_font("JetBrainsMono-Regular.ttf", 26)) {
        std::cerr << "\033[31m[Main] Error al cargar la fuente JetBrains Mono.\033[0m\n";
    }

    Label label_title("TitleLabel");
    label_title.set_font(jetbrains_font);
    label_title.set_text("¡PUZZLEEEE UwU!");
    label_title.set_modulate(Color(0.95f, 0.4f, 0.6f, 1.0f)); // Tono rosa/magenta pastel

    // Calcular posición anclada a la esquina superior derecha con margen de 25px
    Vector2 text_dim = label_title.get_minimum_size();
    float margin = 25.0f;
    float label_x = static_cast<float>(window.get_size().width) - text_dim.x - margin;
    float label_y = margin;

    label_title.set_position(Vector2(label_x, label_y));
    label_title.set_size(text_dim);

    ui_root.add_child(&label_title);

    // Inicializar nodos en cascada
    root.propagate_ready();

    // 4. Iniciar Puzzle CLI conectado a la escena
    EngineConsole console;
    console.set_scene_root(&root);
    console.start();

    // 5. Bucle principal
    window.run([&](double delta) {
        // Actualizar estados de entrada al inicio de cada fotograma
        Input::update();

        // Despachar comandos de la terminal en el hilo de render
        console.process_commands();

        RenderingGeometry& geom = window.get_rendering_geometry();
        Size2i vp_size = window.get_size();

        // Actualizar la posición del Label si la ventana cambia de tamaño
        Vector2 cur_dim = label_title.get_minimum_size();
        label_title.set_position(Vector2(static_cast<float>(vp_size.width) - cur_dim.x - margin, margin));

        // --- Navegación de Cámara con Input nativo ---
        float move_speed = 400.0f * static_cast<float>(delta);
        Vector2 input_dir(0.0f, 0.0f);

        if (Input::is_key_pressed(Key::D)) input_dir.x += 1.0f;
        if (Input::is_key_pressed(Key::A)) input_dir.x -= 1.0f;
        if (Input::is_key_pressed(Key::S)) input_dir.y += 1.0f;
        if (Input::is_key_pressed(Key::W)) input_dir.y -= 1.0f;

        if (input_dir.x != 0.0f || input_dir.y != 0.0f) {
            camera.translate(input_dir.normalized() * move_speed);
        }

        // --- Controles de Zoom ---
        float zoom_speed = 1.0f * static_cast<float>(delta);
        if (Input::is_key_pressed(Key::E)) {
            camera.set_zoom(camera.get_zoom() + Vector2(zoom_speed, zoom_speed));
        }
        if (Input::is_key_pressed(Key::Q)) {
            camera.set_zoom(camera.get_zoom() - Vector2(zoom_speed, zoom_speed));
        }

        // Actualizar lógica de cámara
        camera.update_camera(delta, vp_size);

        // Propagar proceso lógico a través del árbol (ejecuta Button::_process y Label::_process)
        root.propagate_process(static_cast<float>(delta));

        // --- Renderizado 1: Mundo 2D proyectado con cámara ---
        Vector2 world_pos = mario.get_position();
        Vector2 base_scale = mario.get_scale();
        Vector2 active_zoom = camera.get_zoom();

        Vector2 screen_pos = camera.world_to_screen(world_pos, vp_size);
        mario.set_position(screen_pos);
        mario.set_scale(Vector2(base_scale.x * active_zoom.x, base_scale.y * active_zoom.y));

        world.propagate_draw(&geom);

        mario.set_position(world_pos);
        mario.set_scale(base_scale);

        // --- Renderizado 2: Capa de Interfaz de Usuario ---
        ui_root.propagate_draw(&geom);
    });

    console.stop();
    return 0;
}