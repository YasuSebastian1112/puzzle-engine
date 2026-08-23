// ========================================================================= //
// Sierra Engine - 2D/3D Hybrid Game Engine
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// ========================================================================= //

#include "rendering_device.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

// Creacion de ventanas

bool RenderingDevice::create_window(const std::string& p_title, const Size2i& p_size, const Point2i& p_pos) {
    if (!glfwInit()) {
        std::cerr << "[RenderingDevice] Error al inicializar GLFW" << std::endl;
        return false;
    }

    // Configuración OpenGL
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

    // Inicializar GLAD tras tener el contexto activo
    if (!gladLoadGL()) {
        std::cerr << "[RenderingDevice] Error al inicializar GLAD" << std::endl;
        return false;
    }

    set_viewport(Point2i(0, 0), p_size);
    set_blend_enabled(true);

    return true;
}

void RenderingDevice::destroy_window() {
    if (native_window) {
        glfwDestroyWindow(static_cast<GLFWwindow*>(native_window));
        native_window = nullptr;
        glfwTerminate();
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

void RenderingDevice::set_window_title(const std::string& p_title) {
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

// Destructor

RenderingDevice::~RenderingDevice() {
    destroy_window();
}