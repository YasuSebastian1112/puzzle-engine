// ========================================================================= //
// Sierra Engine - 2D/3D Hybrid Game Engine
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// ========================================================================= //


#include "rendering_device.h"
#include "globals/input/input.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>


#if defined(_WIN32)
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <windows.h>

static WNDPROC g_original_wndproc = nullptr;

static LRESULT CALLBACK CustomSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    GLFWwindow* win = static_cast<GLFWwindow*>(glfwGetCurrentContext());
    RenderingDevice* device = win ? static_cast<RenderingDevice*>(glfwGetWindowUserPointer(win)) : nullptr;

    switch (uMsg) {
        case WM_ENTERSIZEMOVE:
            // El usuario tocó el borde o la barra: activamos el timer
            SetTimer(hWnd, 1, 16, nullptr);
            break;

        case WM_TIMER:
            if (wParam == 1 && device) {
                device->trigger_frame_update();
            }
            return 0;

        case WM_EXITSIZEMOVE:
            // El usuario soltó el marco: apagamos el timer de inmediato
            KillTimer(hWnd, 1);
            break;
    }

    return CallWindowProc(g_original_wndproc, hWnd, uMsg, wParam, lParam);
}
#endif


// Creacion de ventanas

bool RenderingDevice::create_window(const String& p_title, const Size2i& p_size, const Point2i& p_pos) {
    if (!glfwInit()) {
        std::cerr << "[RenderingDevice] Error al inicializar GLFW" << std::endl;
        return false;
    }

    current_screen_size = p_size;
    windowed_size = p_size;
    windowed_pos = p_pos;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(p_size.width, p_size.height, p_title.c_str(), nullptr, nullptr);
    if (!window) {
        std::cerr << "[RenderingDevice] Error al crear GLFWwindow" << std::endl;
        glfwTerminate();
        return false;
    }

    native_window = static_cast<WindowHandle>(window);
    glfwSetWindowPos(window, p_pos.x, p_pos.y);
    glfwMakeContextCurrent(window);

    // Guardamos la instancia de RenderingDevice en la ventana GLFW para el callback
    glfwSetWindowUserPointer(window, this);

    // Callback para cuando el usuario maximiza, minimiza o cambia el tamaño
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* win, int width, int height) {
        RenderingDevice* device = static_cast<RenderingDevice*>(glfwGetWindowUserPointer(win));
        if (device && width > 0 && height > 0) {
            device->current_screen_size = Size2i(width, height);
            device->set_viewport(Point2i(0, 0), Size2i(width, height));
            device->trigger_frame_update();
        }
    });

    //  Callback de refresco continuo durante el redimensionamiento modal de Windows
    glfwSetWindowRefreshCallback(window, [](GLFWwindow* win) {
        RenderingDevice* device = static_cast<RenderingDevice*>(glfwGetWindowUserPointer(win));
        if (device) {
            device->trigger_frame_update();
        }
    });

    #if defined(_WIN32)
    HWND hwnd = glfwGetWin32Window(window);
    if (hwnd) {
        g_original_wndproc = (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)CustomSubclassProc);
    }
    #endif

    // Callbacks del Sistema de Entrada
    glfwSetKeyCallback(window, [](GLFWwindow*, int key, int scancode, int action, int mods) {
        Input::on_key_event(key, action);
    });

    glfwSetMouseButtonCallback(window, [](GLFWwindow*, int button, int action, int mods) {
        Input::on_mouse_button_event(button, action);
    });

    glfwSetCursorPosCallback(window, [](GLFWwindow*, double xpos, double ypos) {
        Input::on_mouse_moved_event(xpos, ypos);
    });

    glfwSetScrollCallback(window, [](GLFWwindow*, double xoffset, double yoffset) {
        Input::on_mouse_scrolled_event(xoffset, yoffset);
    });

    if (!gladLoadGL()) {
        std::cerr << "[RenderingDevice] Error al inicializar GLAD" << std::endl;
        return false;
    }
    
    init_geometry_pipeline();

    set_viewport(Point2i(0, 0), p_size);
    set_blend_enabled(true);

    return true;
}

Size2i RenderingDevice::get_window_size() const {
    if (native_window) {
        int w, h;
        glfwGetWindowSize(static_cast<GLFWwindow*>(native_window), &w, &h);
        return Size2i(w, h);
    }
    return current_screen_size;
}

Size2i RenderingDevice::get_framebuffer_size() const {
    if (native_window) {
        int w, h;
        glfwGetFramebufferSize(static_cast<GLFWwindow*>(native_window), &w, &h);
        return Size2i(w, h);
    }
    return current_screen_size;
}

Point2i RenderingDevice::get_window_position() const {
    if (native_window) {
        int x, y;
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
        glfwSetWindowMonitor(win, monitor, 0, 0, vid->width, vid->height, vid->refreshRate);
    } else if (p_mode == WindowMode::Maximized) {
        if (glfwGetWindowMonitor(win) != nullptr) {
            glfwSetWindowMonitor(win, nullptr, windowed_pos.x, windowed_pos.y, windowed_size.width, windowed_size.height, 0);
        }
        glfwMaximizeWindow(win);
    } else { // Windowed
        if (glfwGetWindowMonitor(win) != nullptr) {
            glfwSetWindowMonitor(win, nullptr, windowed_pos.x, windowed_pos.y, windowed_size.width, windowed_size.height, 0);
        } else {
            glfwRestoreWindow(win);
        }
    }
}

void RenderingDevice::destroy_window() {
    if (native_window) {
        glfwDestroyWindow(static_cast<GLFWwindow*>(native_window));
        native_window = nullptr;
        glfwTerminate();
    }
}

void RenderingDevice::trigger_frame_update() {
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

// Control de ventana desde el backend gráfico

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

// Traducción de transformaciones y viewport

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
    if (p_enabled) glEnable(GL_SCISSOR_TEST);
    else glDisable(GL_SCISSOR_TEST);
}

void RenderingDevice::set_blend_enabled(bool p_enabled) {
    if (p_enabled) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    } else {
        glDisable(GL_BLEND);
    }
}

// Formas

// En rendering_device.cpp

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

void RenderingDevice::init_geometry_pipeline() {
    // 1. Compilar shaders
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

    // 2. Crear VAO, VBO y EBO dinámicos
    glGenVertexArrays(1, &geom_vao);
    glGenBuffers(1, &geom_vbo);
    glGenBuffers(1, &geom_ebo);

    glBindVertexArray(geom_vao);
    glBindBuffer(GL_ARRAY_BUFFER, geom_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geom_ebo);

    // Layout: Position (Vector2 = 2 floats)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)0);

    // Layout: Color (Color = 4 floats)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 2));

    // Layout: UV (Vector2 = 2 floats)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 6));

    glBindVertexArray(0);
}

void RenderingDevice::render_geometry(const void* p_vertices, size_t p_vertex_bytes, const uint32_t* p_indices, size_t p_index_count) {
    if (!p_vertices || p_index_count == 0) return;

    glUseProgram(geom_shader);

    // Matriz ortográfica calculada en tiempo real según el tamaño del framebuffer
    float left = 0.0f;
    float right = static_cast<float>(current_screen_size.width);
    float bottom = static_cast<float>(current_screen_size.height);
    float top = 0.0f;

    float ortho_mat[16] = {
        2.0f / (right - left), 0.0f, 0.0f, 0.0f,
        0.0f, 2.0f / (top - bottom), 0.0f, 0.0f,
        0.0f, 0.0f, -1.0f, 0.0f,
        -(right + left) / (right - left), -(top + bottom) / (top - bottom), 0.0f, 1.0f
    };

    GLint proj_loc = glGetUniformLocation(geom_shader, "u_projection");
    glUniformMatrix4fv(proj_loc, 1, GL_FALSE, ortho_mat);

    glBindVertexArray(geom_vao);

    glBindBuffer(GL_ARRAY_BUFFER, geom_vbo);
    glBufferData(GL_ARRAY_BUFFER, p_vertex_bytes, p_vertices, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geom_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, p_index_count * sizeof(uint32_t), p_indices, GL_DYNAMIC_DRAW);

    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(p_index_count), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
}

RenderingDevice::~RenderingDevice() {
    destroy_window();
}