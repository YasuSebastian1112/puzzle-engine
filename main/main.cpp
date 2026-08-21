#include "globals/rendering_global/rendering_global.h"
#include "core/core.h"
#include "classes/2d/sprite_2d.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

// Shaders básicos integrados
const char* vertex_src = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 u_projection;
uniform mat4 u_transform;

void main() {
    gl_Position = u_projection * u_transform * vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
}
)";

const char* fragment_src = R"(
#version 330 core
out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D u_texture;

void main() {
    FragColor = texture(u_texture, TexCoord);
}
)";

GLuint create_shader_program() {
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertex_src, nullptr);
    glCompileShader(vs);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragment_src, nullptr);
    glCompileShader(fs);

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}

int main() {
    core_init();

    auto& renderer = RenderingGlobal::get();
    if (!renderer.init()) {
        return -1;
    }

    const int screen_w = 1280;
    const int screen_h = 720;

    auto main_window = renderer.create_window(screen_w, screen_h, "Puzzle Engine", Window::Type::Editor);
    if (!main_window) {
        return -1;
    }

    main_window->make_current();

    // Activar transparencia (Alpha Blending) para PNGs
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Compilar el shader de dibujo
    GLuint shader = create_shader_program();
    glUseProgram(shader);

    GLint loc_proj = glGetUniformLocation(shader, "u_projection");
    GLint loc_trans = glGetUniformLocation(shader, "u_transform");

    // Proyección ortográfica 2D (0,0 en la esquina superior izquierda)
    glm::mat4 projection = glm::ortho(0.0f, (float)screen_w, (float)screen_h, 0.0f, -1.0f, 1.0f);
    glUniformMatrix4fv(loc_proj, 1, GL_FALSE, glm::value_ptr(projection));

    // Instanciar y configurar el Sprite
    Sprite2D player_sprite("PlayerSprite");
    if (!player_sprite.load_texture_from_file("HELLBUNNY_OG.png")) {
        std::cerr << "No se pudo encontrar HELLBUNNY OG.png\n";
    }

    // Centrar en pantalla y escalar a su resolución real
    player_sprite.position = glm::vec2(screen_w / 2.0f, screen_h / 2.0f);
    player_sprite.scale = glm::vec2(player_sprite.get_width(), player_sprite.get_height());

    // Loop principal
    while (!main_window->should_close()) {
        renderer.poll_events();

        main_window->make_current();
        renderer.clear_screen(0.12f, 0.12f, 0.14f, 1.0f);

        // Renderizar Sprite
        glUseProgram(shader);
        glm::mat4 transform = player_sprite.get_transform();
        glUniformMatrix4fv(loc_trans, 1, GL_FALSE, glm::value_ptr(transform));

        player_sprite._draw();

        main_window->swap_buffers();
    }

    glDeleteProgram(shader);
    renderer.shutdown();
    return 0;
}