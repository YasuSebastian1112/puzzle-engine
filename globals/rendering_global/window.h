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

    using Mode = RenderingDevice::WindowMode;
    
private:

    RenderingDevice rendering_device;

    Type type = Type::Custom;
    String title = "New Window";
    Size2i size = Size2i(1280, 720);
    Point2i position = Point2i(100, 100);
    bool vsync = true;


public:

    bool initialize(const String& p_title, const Size2i& p_size, const Point2i& p_position = Point2i(100, 100), Type p_type = Type::Custom);
    void poll_events();
    void swap_buffers();
    bool should_close() const;

    // Limpieza de pantalla básica
    void clear(const Color& p_color = Color(0.1f, 0.1f, 0.1f, 1.0f));

    // Getters/Setters
    void set_type(Type p_type) { type = p_type; }
    Type get_type() const { return type; }

    void set_mode(Mode p_mode) { rendering_device.set_window_mode(p_mode); }
    bool is_maximized() const { return rendering_device.is_window_maximized(); }

    void set_title(const String& p_title);
    String get_title() const { return title; }

    void set_size(const Size2i& p_size);
    Size2i get_size() const { return rendering_device.get_window_size(); }
    

    void set_position(const Point2i& p_pos);
    Point2i get_position() const { return rendering_device.get_window_position(); }

    void set_vsync(bool p_enabled);
    bool is_vsync_enabled() const { return vsync; }

    Size2i get_framebuffer_size() const { return rendering_device.get_framebuffer_size(); }

    RenderingDevice& get_rendering_device() { return rendering_device; }
    const RenderingDevice& get_rendering_device() const { return rendering_device; }

    Window();
    virtual ~Window();
};
