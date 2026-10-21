// ========================================================================= //
// Sierra Engine - 2D/3D Hybrid Game Engine
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// ========================================================================= //
#include "window.h"

bool Window::initialize(const String& p_title, const Size2i& p_size, const Point2i& p_position, Type p_type) {
    title = p_title;
    size = p_size;
    position = p_position;
    type = p_type;

    if (!rendering_device.create_window(title, size, position)) {
        return false;
    }
    
    set_vsync(vsync);
    return true;
}

void Window::poll_events() {
    rendering_device.poll_events();
}

void Window::swap_buffers() {
    rendering_device.swap_buffers();
}

bool Window::should_close() const {
    return rendering_device.window_should_close();
}

void Window::clear(const Color& p_color) {
    rendering_device.set_clear_color(p_color);
    rendering_device.clear_buffers(true, true, false);
}

void Window::set_title(const String& p_title) {
    title = p_title;
    rendering_device.set_window_title(title);
}


void Window::set_size(const Size2i& p_size) {
    size = p_size;
    rendering_device.set_window_size(size);
    rendering_device.set_viewport(Point2i(0, 0), size);
}

void Window::set_position(const Point2i& p_pos) {
    position = p_pos;
    rendering_device.set_window_position(position);
}

void Window::set_vsync(bool p_enabled) {
    vsync = p_enabled;
    rendering_device.set_vsync(vsync);
}

Window::Window() {
    title = "Puzzle Engine Window";
    size = Size2i(1280, 720);
    position = Point2i(100, 100);
    type = Type::Custom;
}

Window::~Window() {
    rendering_device.destroy_window();
}