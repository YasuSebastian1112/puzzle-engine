// ========================================================================= //
// Sierra Engine - 2D/3D Hybrid Game Engine
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// ========================================================================= //

#pragma once

#include "device/rendering_device.h"
#include "classes/object.h"
#include "core/mathematics/math_2d.h"
#include "globals/rendering_global/device/rendering_geometry.h"

typedef struct GLFWcursor GLFWcursor;

class Window : public Object {

    CLASS_TYPE(Window, Object);

public:
    
    enum class Type {
        Editor,
        Game,
        Custom
    };

    enum class ResizeDirection {
        None = 0,
        Left   = 1 << 0,
        Right  = 1 << 1,
        Top    = 1 << 2,
        Bottom = 1 << 3,
        TopLeft     = Top | Left,
        TopRight    = Top | Right,
        BottomLeft  = Bottom | Left,
        BottomRight = Bottom | Right
    };

    using Mode = RenderingDevice::WindowMode;
    using FrameCallback = std::function<void(double delta)>;

private:

    RenderingDevice rendering_device;

    Type type = Type::Custom;
    String title = "New Window";
    Size2i size = Size2i(1280, 720);
    Point2i position = Point2i(100, 100);
    bool vsync = true;
    bool m_is_maximized = false;
    Point2i m_saved_windowed_pos;
    Size2i m_saved_windowed_size;

    // Estado para arrastrar
    bool m_is_dragging = false;
    double m_drag_screen_start_x = 0.0;
    double m_drag_screen_start_y = 0.0;
    int m_drag_win_start_x = 0;
    int m_drag_win_start_y = 0;

    // Estado para redimensionar bordes
    ResizeDirection m_resize_direction = ResizeDirection::None;
    double m_resize_screen_start_x = 0.0;
    double m_resize_screen_start_y = 0.0;
    int m_resize_win_start_x = 0;
    int m_resize_win_start_y = 0;
    int m_resize_win_start_w = 0;
    int m_resize_win_start_h = 0;

    GLFWcursor* m_cursor_arrow = nullptr;
    GLFWcursor* m_cursor_hresize = nullptr;
    GLFWcursor* m_cursor_vresize = nullptr;
    GLFWcursor* m_cursor_nesw = nullptr;
    GLFWcursor* m_cursor_nwse = nullptr;

    // Métodos privados de manipulación interna
    void start_dragging();
    void update_dragging();
    void stop_dragging();

    ResizeDirection hit_test_resize_borders(const Point2i& p_cursor_pos, int p_border_thickness = 8) const;
    void start_resizing(ResizeDirection p_dir);
    void update_resizing();
    void stop_resizing();
    void update_cursor_icon(ResizeDirection p_dir);

public:

    Window();
    virtual ~Window();

    bool initialize(const String& p_title, const Size2i& p_size, const Point2i& p_pos, Type p_type);
    void run(FrameCallback p_frame_callback);
    void close();
    
    void set_position(const Point2i& p_pos);
    Point2i get_position() const;

    void set_size(const Size2i& p_size);
    Size2i get_size() const;

    void minimize();
    void maximize();
    void restore();
    void toggle_maximize();
    bool is_maximized() const { return m_is_maximized; }

    bool is_dragging() const { return m_is_dragging; }
    bool is_resizing() const { return m_resize_direction != ResizeDirection::None; }

    const String& get_title() const { return title; }
    void set_title(const String& p_title);

    void process_window_interaction();
    void draw_title_bar(RenderingGeometry& p_geometry);

    void clear(const Color& p_color);
    void swap_buffers();
    void poll_events();
    bool should_close() const;

    RenderingDevice& get_rendering_device() { return rendering_device; }
};