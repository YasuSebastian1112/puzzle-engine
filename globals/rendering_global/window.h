// ========================================================================= //
// Sierra Engine - 2D/3D Hybrid Game Engine
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// ========================================================================= //

#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>


class Window {
public:
    enum class Type {
        Editor,
        Game,
        Custom
    };

    Window(int width, int height, const std::string& title, Type type = Type::Game);
    ~Window();

    bool is_valid() const { return handle != nullptr; }
    bool should_close() const;
    void make_current();
    void swap_buffers();
    void set_title(const std::string& title);

    int get_width() const { return width; }
    int get_height() const { return height; }
    Type get_type() const { return type; }
    GLFWwindow* get_handle() const { return handle; }

private:
    GLFWwindow* handle = nullptr;
    int width = 0;
    int height = 0;
    std::string title;
    Type type;

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
};
