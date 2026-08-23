// ========================================================================= //
// Sierra Engine - 2D/3D Hybrid Game Engine
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// ========================================================================= //

#pragma once

#include "device/rendering_device.h"
#include "classes/object.h"

class Window : public Object {

    CLASS_TYPE(Window, Object);

public:
    
    enum class Type { // Enumero los tipos para indicar a que pertenece la ventana
        Editor,
        Game,
        Custom
    };

private:

    RenderingDevice rendering_device;

    Type type = Type::Custom;
    std::string title = "New Window";
    Size2i size = Size2i(1280, 720);
    Point2i position = Point2i(100, 100);
    bool vsync = true;

public:

    bool initialize(const std::string& p_title, const Size2i& p_size, const Point2i& p_position = Point2i(100, 100), Type p_type = Type::Custom);
    void poll_events();
    void swap_buffers();
    bool should_close() const;

    // Limpieza de pantalla básica
    void clear(const Color& p_color = Color(0.1f, 0.1f, 0.1f, 1.0f));

    // Getters/Setters
    void set_type(Type p_type) { type = p_type; }
    Type get_type() const { return type; }

    void set_title(const std::string& p_title);
    std::string get_title() const { return title; }

    void set_size(const Size2i& p_size);
    Size2i get_size() const { return size; }

    void set_position(const Point2i& p_pos);
    Point2i get_position() const { return position; }

    void set_vsync(bool p_enabled);
    bool is_vsync_enabled() const { return vsync; }

    RenderingDevice& get_rendering_device() { return rendering_device; }

    Window();
    virtual ~Window();
};
