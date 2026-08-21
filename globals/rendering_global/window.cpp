// ========================================================================= //
// Sierra Engine - 2D/3D Hybrid Game Engine
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// ========================================================================= //
#include "window.h"
#include <iostream>


void Window::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

Window::Window(int w, int h, const std::string& t, Window::Type type)
    : width(w), height(h), title(t), type(type) {

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    handle = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!handle) {
        std::cerr << "[Sierra Engine] Error al crear la ventana: " << title << "\n";
        return;
    }

    glfwSetWindowUserPointer(handle, this);
    glfwSetFramebufferSizeCallback(handle, framebuffer_size_callback);
}

Window::~Window() {
    if (handle) {
        glfwDestroyWindow(handle);
        handle = nullptr;
    }
}

bool Window::should_close() const {
    return handle ? glfwWindowShouldClose(handle) : true;
}

void Window::make_current() {
    if (handle) {
        glfwMakeContextCurrent(handle);
    }
}

void Window::swap_buffers() {
    if (handle) {
        glfwSwapBuffers(handle);
    }
}

void Window::set_title(const std::string& new_title) {
    title = new_title;
    if (handle) {
        glfwSetWindowTitle(handle, title.c_str());
    }
}
