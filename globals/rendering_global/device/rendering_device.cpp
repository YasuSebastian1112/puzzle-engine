// ========================================================================= //
// PUZZLE ENGINE - 2D/3D GAME ENGINE
// ------------------------------------------------------------------------- //
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// ========================================================================= //

#include "rendering_device.h"
#include "globals/input/input.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#if defined(_WIN32)
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include "platform/system_info.h"
#include "platform/windows/os_windows.h"
#endif

// Shaders básicos integrados para el pipeline de geometría 2D
static const char* vertex_shader_src = R"(
#version 330 core
layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec4 a_color;
layout (location = 2) in vec2 a_uv;

out vec4 v_color;

uniform mat4 u_projection;

void main() {
    v_color = a_color;
    gl_Position = u_projection * vec4(a_pos, 0.0, 1.0);
}
)";

static const char* fragment_shader_src = R"(
#version 330 core
in vec4 v_color;
out vec4 FragColor;

void main() {
    FragColor = v_color;
}
)";

// Recalcula la matriz ortográfica de proyección únicamente si cambian las dimensiones del viewport
void RenderingDevice::update_orthographic_projection() {
    float right = static_cast<float>(current_screen_size.width);
    float bottom = static_cast<float>(current_screen_size.height);

    m_cached_ortho_mat[0]  = 2.0f / right;
    m_cached_ortho_mat[1]  = 0.0f;
    m_cached_ortho_mat[2]  = 0.0f;
    m_cached_ortho_mat[3]  = 0.0f;

    m_cached_ortho_mat[4]  = 0.0f;
    m_cached_ortho_mat[5]  = -2.0f / bottom;
    m_cached_ortho_mat[6]  = 0.0f;
    m_cached_ortho_mat[7]  = 0.0f;

    m_cached_ortho_mat[8]  = 0.0f;
    m_cached_ortho_mat[9]  = 0.0f;
    m_cached_ortho_mat[10] = -1.0f;
    m_cached_ortho_mat[11] = 0.0f;

    m_cached_ortho_mat[12] = -1.0f;
    m_cached_ortho_mat[13] = 1.0f;
    m_cached_ortho_mat[14] = 0.0f;
    m_cached_ortho_mat[15] = 1.0f;

    m_proj_dirty = false;
}

bool RenderingDevice::create_window(const String& p_title, const Size2i& p_size, const Point2i& p_pos) {
    if (!glfwInit()) {
        std::cerr << "[RenderingDevice] Fallo crítico al inicializar GLFW." << std::endl;
        return false;
    }

    current_screen_size = p_size;
    windowed_size = p_size;
    windowed_pos = p_pos;
    m_proj_dirty = true;

    // Configuración del contexto OpenGL 3.3 Core
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(p_size.width, p_size.height, p_title.c_str(), nullptr, nullptr);
    if (!window) {
        std::cerr << "[RenderingDevice] Fallo al crear la instancia de GLFWwindow." << std::endl;
        glfwTerminate();
        return false;
    }

    native_window = static_cast<WindowHandle>(window);
    glfwSetWindowPos(window, p_pos.x, p_pos.y);
    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, this);

    // Callbacks del subsistema de ventanas
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* win, int width, int height) {
        RenderingDevice* device = static_cast<RenderingDevice*>(glfwGetWindowUserPointer(win));
        if (device && width > 0 && height > 0) {
            device->current_screen_size = Size2i(width, height);
            device->m_proj_dirty = true;
            device->set_viewport(Point2i(0, 0), Size2i(width, height));
            device->trigger_frame_update();
        }
    });

    glfwSetWindowRefreshCallback(window, [](GLFWwindow* win) {
        RenderingDevice* device = static_cast<RenderingDevice*>(glfwGetWindowUserPointer(win));
        if (device) {
            device->trigger_frame_update();
        }
    });

#if defined(_WIN32)
    HWND hwnd = glfwGetWin32Window(window);
    if (hwnd) {
        OSWindows::install_window_hooks(hwnd, [this]() {
            trigger_frame_update();
        });
    }
#endif

    // Callbacks del subsistema de entrada
    glfwSetKeyCallback(window, [](GLFWwindow*, int key, int, int action, int) {
        Input::on_key_event(key, action);
    });

    glfwSetMouseButtonCallback(window, [](GLFWwindow*, int button, int action, int) {
        Input::on_mouse_button_event(button, action);
    });

    glfwSetCursorPosCallback(window, [](GLFWwindow*, double xpos, double ypos) {
        Input::on_mouse_moved_event(xpos, ypos);
    });

    glfwSetScrollCallback(window, [](GLFWwindow*, double xoffset, double yoffset) {
        Input::on_mouse_scrolled_event(xoffset, yoffset);
    });

    if (!gladLoadGL()) {
        std::cerr << "[RenderingDevice] Fallo al inicializar los punteros de funciones con GLAD." << std::endl;
        return false;
    }

    init_geometry_pipeline();
    set_viewport(Point2i(0, 0), p_size);
    set_blend_enabled(true);    

    // Purgar inmediatamente las páginas transitorias de arranque (DLLs, JIT shaders, etc.)
#if defined(_WIN32)
    // Forzar a Windows a vaciar la cola de mensajes del hilo actual
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Con todos los mensajes de creación y mapeo procesados, purgar la memoria
    SystemInfo::trim_memory();
#endif

    return true;
}

Size2i RenderingDevice::get_window_size() const {
    if (native_window) {
        int w = 0, h = 0;
        glfwGetWindowSize(static_cast<GLFWwindow*>(native_window), &w, &h);
        return Size2i(w, h);
    }
    return current_screen_size;
}

Size2i RenderingDevice::get_framebuffer_size() const {
    if (native_window) {
        int w = 0, h = 0;
        glfwGetFramebufferSize(static_cast<GLFWwindow*>(native_window), &w, &h);
        return Size2i(w, h);
    }
    return current_screen_size;
}

Point2i RenderingDevice::get_window_position() const {
    if (native_window) {
        int x = 0, y = 0;
        glfwGetWindowPos(static_cast<GLFWwindow*>(native_window), &x, &y);
        return Point2i(x, y);
    }
    return windowed_pos;
}

bool RenderingDevice::is_window_maximized() const {
    if (native_window) {
        return glfwGetWindowAttrib(static_cast<GLFWwindow*>(native_window), GLFW_MAXIMIZED) == GLFW_TRUE;
    }
    return false;
}

void RenderingDevice::set_window_mode(WindowMode p_mode) {
    if (!native_window) return;
    GLFWwindow* win = static_cast<GLFWwindow*>(native_window);

    if (p_mode == WindowMode::Fullscreen) {
        glfwGetWindowPos(win, &windowed_pos.x, &windowed_pos.y);
        glfwGetWindowSize(win, &windowed_size.width, &windowed_size.height);

        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* vid = glfwGetVideoMode(monitor);
        if (monitor && vid) {
            glfwSetWindowMonitor(win, monitor, 0, 0, vid->width, vid->height, vid->refreshRate);
        }
    } else if (p_mode == WindowMode::Maximized) {
        if (glfwGetWindowMonitor(win) != nullptr) {
            glfwSetWindowMonitor(win, nullptr, windowed_pos.x, windowed_pos.y, windowed_size.width, windowed_size.height, 0);
        }
        glfwMaximizeWindow(win);
    } else {
        if (glfwGetWindowMonitor(win) != nullptr) {
            glfwSetWindowMonitor(win, nullptr, windowed_pos.x, windowed_pos.y, windowed_size.width, windowed_size.height, 0);
        } else {
            glfwRestoreWindow(win);
        }
    }
}

void RenderingDevice::destroy_window() {
    if (native_window) {
        GLFWwindow* win = static_cast<GLFWwindow*>(native_window);

#if defined(_WIN32)
        HWND hwnd = glfwGetWin32Window(win);
        OSWindows::remove_window_hooks(hwnd);
#endif

        if (geom_vao)    { glDeleteVertexArrays(1, &geom_vao); geom_vao = 0; }
        if (geom_vbo)    { glDeleteBuffers(1, &geom_vbo); geom_vbo = 0; }
        if (geom_ebo)    { glDeleteBuffers(1, &geom_ebo); geom_ebo = 0; }
        if (geom_shader) { glDeleteProgram(geom_shader); geom_shader = 0; }

        glfwDestroyWindow(win);
        native_window = nullptr;
        glfwTerminate();
    }
}

void RenderingDevice::trigger_frame_update() {
    if (native_window) {
        int fb_w = 0, fb_h = 0;
        glfwGetFramebufferSize(static_cast<GLFWwindow*>(native_window), &fb_w, &fb_h);
        if (fb_w > 0 && fb_h > 0 && (fb_w != current_screen_size.width || fb_h != current_screen_size.height)) {
            current_screen_size = Size2i(fb_w, fb_h);
            m_proj_dirty = true;
            set_viewport(Point2i(0, 0), current_screen_size);
        }
    }

    if (render_callback) {
        render_callback();
        swap_buffers();
    }
}

void RenderingDevice::poll_events() {
    glfwPollEvents();
}

void RenderingDevice::swap_buffers() {
    if (native_window) {
        glfwSwapBuffers(static_cast<GLFWwindow*>(native_window));
    }
}

bool RenderingDevice::window_should_close() const {
    return native_window ? glfwWindowShouldClose(static_cast<GLFWwindow*>(native_window)) : true;
}

void RenderingDevice::set_window_size(const Size2i& p_size) {
    if (native_window) {
        glfwSetWindowSize(static_cast<GLFWwindow*>(native_window), p_size.width, p_size.height);
    }
}

void RenderingDevice::set_window_position(const Point2i& p_pos) {
    if (native_window) {
        glfwSetWindowPos(static_cast<GLFWwindow*>(native_window), p_pos.x, p_pos.y);
    }
}

void RenderingDevice::set_window_title(const String& p_title) {
    if (native_window) {
        glfwSetWindowTitle(static_cast<GLFWwindow*>(native_window), p_title.c_str());
    }
}

void RenderingDevice::set_vsync(bool p_enabled) {
    glfwSwapInterval(p_enabled ? 1 : 0);
}

void RenderingDevice::set_viewport(const Point2i& p_position, const Size2i& p_size) {
    glViewport(
        static_cast<GLint>(p_position.x),
        static_cast<GLint>(p_position.y),
        static_cast<GLsizei>(p_size.width),
        static_cast<GLsizei>(p_size.height)
    );
}

void RenderingDevice::set_clear_color(const Color& p_color) {
    glClearColor(p_color.r, p_color.g, p_color.b, p_color.a);
}

void RenderingDevice::clear_buffers(bool p_color, bool p_depth, bool p_stencil) {
    GLbitfield mask = 0;
    if (p_color)   mask |= GL_COLOR_BUFFER_BIT;
    if (p_depth)   mask |= GL_DEPTH_BUFFER_BIT;
    if (p_stencil) mask |= GL_STENCIL_BUFFER_BIT;
    glClear(mask);
}

void RenderingDevice::set_scissor(const Point2i& p_position, const Size2i& p_size) {
    glScissor(
        static_cast<GLint>(p_position.x),
        static_cast<GLint>(p_position.y),
        static_cast<GLsizei>(p_size.width),
        static_cast<GLsizei>(p_size.height)
    );
}

void RenderingDevice::set_scissor_enabled(bool p_enabled) {
    if (p_enabled) {
        glEnable(GL_SCISSOR_TEST);
    } else {
        glDisable(GL_SCISSOR_TEST);
    }
}

void RenderingDevice::set_blend_enabled(bool p_enabled) {
    if (p_enabled) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    } else {
        glDisable(GL_BLEND);
    }
}

void RenderingDevice::init_geometry_pipeline() {
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertex_shader_src, nullptr);
    glCompileShader(vs);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragment_shader_src, nullptr);
    glCompileShader(fs);

    geom_shader = glCreateProgram();
    glAttachShader(geom_shader, vs);
    glAttachShader(geom_shader, fs);
    glLinkProgram(geom_shader);

    glDeleteShader(vs);
    glDeleteShader(fs);

    geom_proj_loc = glGetUniformLocation(geom_shader, "u_projection");

    glGenVertexArrays(1, &geom_vao);
    glGenBuffers(1, &geom_vbo);
    glGenBuffers(1, &geom_ebo);

    glBindVertexArray(geom_vao);

    // Reserva inicial estática para evitar reasignaciones en el bucle principal
    glBindBuffer(GL_ARRAY_BUFFER, geom_vbo);
    glBufferData(GL_ARRAY_BUFFER, current_max_vertex_bytes, nullptr, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geom_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, current_max_index_bytes, nullptr, GL_DYNAMIC_DRAW);

    // Atributo 0: Posición (vec2, 2 floats)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, reinterpret_cast<void*>(0));

    // Atributo 1: Color (vec4, 4 floats)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 8, reinterpret_cast<void*>(sizeof(float) * 2));

    // Atributo 2: Coordenadas UV (vec2, 2 floats)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, reinterpret_cast<void*>(sizeof(float) * 6));

    glBindVertexArray(0);
}

void RenderingDevice::render_geometry(const void* p_vertices, size_t p_vertex_bytes, const uint32_t* p_indices, size_t p_index_count) {
    if (!p_vertices || p_index_count == 0) return;

    size_t index_bytes = p_index_count * sizeof(uint32_t);

    glUseProgram(geom_shader);

    // Actualizar la matriz de proyección si el tamaño de pantalla cambió
    if (m_proj_dirty) {
        update_orthographic_projection();
    }

    if (geom_proj_loc != -1) {
        glUniformMatrix4fv(geom_proj_loc, 1, GL_FALSE, m_cached_ortho_mat);
    }

    glBindVertexArray(geom_vao);

    // Manejo de vértices con Buffer Orphan para evitar bloqueos del driver
    glBindBuffer(GL_ARRAY_BUFFER, geom_vbo);
    if (p_vertex_bytes > current_max_vertex_bytes) {
        current_max_vertex_bytes = p_vertex_bytes * 2;
        glBufferData(GL_ARRAY_BUFFER, current_max_vertex_bytes, nullptr, GL_DYNAMIC_DRAW);
    } else {
        glBufferData(GL_ARRAY_BUFFER, current_max_vertex_bytes, nullptr, GL_DYNAMIC_DRAW);
    }
    glBufferSubData(GL_ARRAY_BUFFER, 0, p_vertex_bytes, p_vertices);

    // Manejo de índices con Buffer Orphan
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geom_ebo);
    if (index_bytes > current_max_index_bytes) {
        current_max_index_bytes = index_bytes * 2;
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, current_max_index_bytes, nullptr, GL_DYNAMIC_DRAW);
    } else {
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, current_max_index_bytes, nullptr, GL_DYNAMIC_DRAW);
    }
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, index_bytes, p_indices);

    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(p_index_count), GL_UNSIGNED_INT, nullptr);

    glBindVertexArray(0);
}

RenderingDevice::~RenderingDevice() {
    destroy_window();
}