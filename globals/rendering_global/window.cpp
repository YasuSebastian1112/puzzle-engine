#include "window.h"
#include <GLFW/glfw3.h>

#if defined(_WIN32)
#include "platform/windows/os_windows.h"
#endif

Window::Window() {}

Window::~Window() {
    GLFWwindow* win = static_cast<GLFWwindow*>(rendering_device.get_native_handle());
    if (win) {
        if (m_cursor_arrow)   glfwDestroyCursor(m_cursor_arrow);
        if (m_cursor_hresize) glfwDestroyCursor(m_cursor_hresize);
        if (m_cursor_vresize) glfwDestroyCursor(m_cursor_vresize);
        if (m_cursor_nesw)    glfwDestroyCursor(m_cursor_nesw);
        if (m_cursor_nwse)    glfwDestroyCursor(m_cursor_nwse);
    }
    rendering_device.destroy_window();
}

bool Window::initialize(const String& p_title, const Size2i& p_size, const Point2i& p_pos, Type p_type) {
    title = p_title;
    type = p_type;
    size = p_size;
    position = p_pos;
    m_saved_windowed_pos = p_pos;
    m_saved_windowed_size = p_size;

    if (!rendering_device.create_window(p_title, p_size, p_pos)) {
        return false;
    }

    GLFWwindow* win = static_cast<GLFWwindow*>(rendering_device.get_native_handle());
    if (win) {
        m_cursor_arrow   = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
        m_cursor_hresize = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
        m_cursor_vresize = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
        m_cursor_nesw    = glfwCreateStandardCursor(GLFW_RESIZE_NESW_CURSOR);
        m_cursor_nwse    = glfwCreateStandardCursor(GLFW_RESIZE_NWSE_CURSOR);
    }

    #if defined(_WIN32)
    OSWindows::trim_memory();
    #endif
    

    return true;
}

void Window::set_title(const String& p_title) {
    title = p_title;
    GLFWwindow* win = static_cast<GLFWwindow*>(rendering_device.get_native_handle());
    if (win) {
        glfwSetWindowTitle(win, p_title.c_str());
    }
}

void Window::close() {
    GLFWwindow* win = static_cast<GLFWwindow*>(rendering_device.get_native_handle());
    if (win) {
        glfwSetWindowShouldClose(win, GLFW_TRUE);
    }
}

bool Window::should_close() const {
    return rendering_device.window_should_close();
}

void Window::poll_events() {
    rendering_device.poll_events();
}

void Window::swap_buffers() {
    rendering_device.swap_buffers();
}

void Window::clear(const Color& p_color) {
    rendering_device.set_clear_color(p_color);
    rendering_device.clear_buffers(true, true, true);
}

void Window::set_position(const Point2i& p_pos) {
    position = p_pos;
    rendering_device.set_window_position(p_pos);
}

Point2i Window::get_position() const {
    return rendering_device.get_window_position();
}

void Window::set_size(const Size2i& p_size) {
    size = p_size;
    rendering_device.set_window_size(p_size);
}

Size2i Window::get_size() const {
    return rendering_device.get_window_size();
}

void Window::minimize() {
    GLFWwindow* win = static_cast<GLFWwindow*>(rendering_device.get_native_handle());
    if (win) {
        glfwIconifyWindow(win);
    }
}

void Window::maximize() {
    GLFWwindow* win = static_cast<GLFWwindow*>(rendering_device.get_native_handle());
    if (!win || m_is_maximized) return;

    glfwGetWindowPos(win, &m_saved_windowed_pos.x, &m_saved_windowed_pos.y);
    glfwGetWindowSize(win, &m_saved_windowed_size.width, &m_saved_windowed_size.height);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    if (monitor) {
        int mx, my, mw, mh;
        glfwGetMonitorWorkarea(monitor, &mx, &my, &mw, &mh);
        glfwSetWindowPos(win, mx, my);
        glfwSetWindowSize(win, mw, mh);
    }

    m_is_maximized = true;
    rendering_device.trigger_frame_update();
}

void Window::restore() {
    GLFWwindow* win = static_cast<GLFWwindow*>(rendering_device.get_native_handle());
    if (!win || !m_is_maximized) return;

    glfwSetWindowPos(win, m_saved_windowed_pos.x, m_saved_windowed_pos.y);
    glfwSetWindowSize(win, m_saved_windowed_size.width, m_saved_windowed_size.height);

    m_is_maximized = false;
    rendering_device.trigger_frame_update();
}

void Window::toggle_maximize() {
    if (m_is_maximized) {
        restore();
    } else {
        maximize();
    }
#if defined(_WIN32)
    OSWindows::trim_memory();
#endif
}

// ------------------------------------------------------------------------- //
// Arrastre con coordenadas absolutas de pantalla
// ------------------------------------------------------------------------- //

void Window::start_dragging() {
    GLFWwindow* win = static_cast<GLFWwindow*>(rendering_device.get_native_handle());
    if (!win || m_is_maximized) return;

    m_is_dragging = true;
    double cx, cy;
    glfwGetCursorPos(win, &cx, &cy);
    glfwGetWindowPos(win, &m_drag_win_start_x, &m_drag_win_start_y);

    m_drag_screen_start_x = m_drag_win_start_x + cx;
    m_drag_screen_start_y = m_drag_win_start_y + cy;
}

void Window::update_dragging() {
    if (!m_is_dragging) return;
    GLFWwindow* win = static_cast<GLFWwindow*>(rendering_device.get_native_handle());
    if (!win) return;

    double cx, cy;
    glfwGetCursorPos(win, &cx, &cy);

    int cur_win_x, cur_win_y;
    glfwGetWindowPos(win, &cur_win_x, &cur_win_y);

    double current_screen_x = cur_win_x + cx;
    double current_screen_y = cur_win_y + cy;

    int new_win_x = m_drag_win_start_x + static_cast<int>(current_screen_x - m_drag_screen_start_x);
    int new_win_y = m_drag_win_start_y + static_cast<int>(current_screen_y - m_drag_screen_start_y);

    glfwSetWindowPos(win, new_win_x, new_win_y);
}

void Window::stop_dragging() {
    if (m_is_dragging) {
        m_is_dragging = false;
#if defined(_WIN32)
        OSWindows::trim_memory();
#endif
    }
}

// ------------------------------------------------------------------------- //
// Redimensionamiento con coordenadas absolutas
// ------------------------------------------------------------------------- //

Window::ResizeDirection Window::hit_test_resize_borders(const Point2i& p_cursor_pos, int p_border_thickness) const {
    if (m_is_maximized) return ResizeDirection::None;

    Size2i sz = get_size();
    int dir = static_cast<int>(ResizeDirection::None);

    if (p_cursor_pos.x >= 0 && p_cursor_pos.x <= p_border_thickness) {
        dir |= static_cast<int>(ResizeDirection::Left);
    } else if (p_cursor_pos.x >= sz.width - p_border_thickness && p_cursor_pos.x <= sz.width) {
        dir |= static_cast<int>(ResizeDirection::Right);
    }

    if (p_cursor_pos.y >= 0 && p_cursor_pos.y <= p_border_thickness) {
        dir |= static_cast<int>(ResizeDirection::Top);
    } else if (p_cursor_pos.y >= sz.height - p_border_thickness && p_cursor_pos.y <= sz.height) {
        dir |= static_cast<int>(ResizeDirection::Bottom);
    }

    return static_cast<ResizeDirection>(dir);
}

void Window::update_cursor_icon(ResizeDirection p_dir) {
    GLFWwindow* win = static_cast<GLFWwindow*>(rendering_device.get_native_handle());
    if (!win) return;

    if (p_dir == ResizeDirection::Left || p_dir == ResizeDirection::Right) {
        glfwSetCursor(win, m_cursor_hresize);
    } else if (p_dir == ResizeDirection::Top || p_dir == ResizeDirection::Bottom) {
        glfwSetCursor(win, m_cursor_vresize);
    } else if (p_dir == ResizeDirection::TopLeft || p_dir == ResizeDirection::BottomRight) {
        glfwSetCursor(win, m_cursor_nwse);
    } else if (p_dir == ResizeDirection::TopRight || p_dir == ResizeDirection::BottomLeft) {
        glfwSetCursor(win, m_cursor_nesw);
    } else {
        glfwSetCursor(win, m_cursor_arrow);
    }
}

void Window::start_resizing(ResizeDirection p_dir) {
    if (p_dir == ResizeDirection::None || m_is_maximized) return;
    GLFWwindow* win = static_cast<GLFWwindow*>(rendering_device.get_native_handle());
    if (!win) return;

    m_resize_direction = p_dir;
    double cx, cy;
    glfwGetCursorPos(win, &cx, &cy);
    glfwGetWindowPos(win, &m_resize_win_start_x, &m_resize_win_start_y);
    glfwGetWindowSize(win, &m_resize_win_start_w, &m_resize_win_start_h);

    m_resize_screen_start_x = m_resize_win_start_x + cx;
    m_resize_screen_start_y = m_resize_win_start_y + cy;
}

void Window::update_resizing() {
    if (m_resize_direction == ResizeDirection::None) return;
    GLFWwindow* win = static_cast<GLFWwindow*>(rendering_device.get_native_handle());
    if (!win) return;

    double cx, cy;
    glfwGetCursorPos(win, &cx, &cy);

    int cur_win_x, cur_win_y;
    glfwGetWindowPos(win, &cur_win_x, &cur_win_y);

    double current_screen_x = cur_win_x + cx;
    double current_screen_y = cur_win_y + cy;

    int delta_screen_x = static_cast<int>(current_screen_x - m_resize_screen_start_x);
    int delta_screen_y = static_cast<int>(current_screen_y - m_resize_screen_start_y);

    int new_x = m_resize_win_start_x;
    int new_y = m_resize_win_start_y;
    int new_w = m_resize_win_start_w;
    int new_h = m_resize_win_start_h;

    constexpr int MIN_WIDTH = 320;
    constexpr int MIN_HEIGHT = 200;

    int dir = static_cast<int>(m_resize_direction);

    if (dir & static_cast<int>(ResizeDirection::Right)) {
        new_w = m_resize_win_start_w + delta_screen_x;
        if (new_w < MIN_WIDTH) new_w = MIN_WIDTH;
    }
    if (dir & static_cast<int>(ResizeDirection::Bottom)) {
        new_h = m_resize_win_start_h + delta_screen_y;
        if (new_h < MIN_HEIGHT) new_h = MIN_HEIGHT;
    }
    if (dir & static_cast<int>(ResizeDirection::Left)) {
        int target_w = m_resize_win_start_w - delta_screen_x;
        if (target_w >= MIN_WIDTH) {
            new_w = target_w;
            new_x = m_resize_win_start_x + delta_screen_x;
        } else {
            new_w = MIN_WIDTH;
            new_x = m_resize_win_start_x + (m_resize_win_start_w - MIN_WIDTH);
        }
    }
    if (dir & static_cast<int>(ResizeDirection::Top)) {
        int target_h = m_resize_win_start_h - delta_screen_y;
        if (target_h >= MIN_HEIGHT) {
            new_h = target_h;
            new_y = m_resize_win_start_y + delta_screen_y;
        } else {
            new_h = MIN_HEIGHT;
            new_y = m_resize_win_start_y + (m_resize_win_start_h - MIN_HEIGHT);
        }
    }

    glfwSetWindowPos(win, new_x, new_y);
    glfwSetWindowSize(win, new_w, new_h);

    rendering_device.trigger_frame_update();
}

void Window::stop_resizing() {
    if (m_resize_direction != ResizeDirection::None) {
        m_resize_direction = ResizeDirection::None;
#if defined(_WIN32)
        OSWindows::trim_memory();
#endif
    }
}

// ------------------------------------------------------------------------- //
// Interacción autónoma de ventana
// ------------------------------------------------------------------------- //

void Window::process_window_interaction() {
    GLFWwindow* win = static_cast<GLFWwindow*>(rendering_device.get_native_handle());
    if (!win) return;

    Size2i win_sz = get_size();
    constexpr float TITLE_BAR_HEIGHT = 28.0f;
    constexpr float BTN_WIDTH = 34.0f;

    double mx, my;
    glfwGetCursorPos(win, &mx, &my);
    Point2i cursor_pos(static_cast<int>(mx), static_cast<int>(my));

    int mouse_left = glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT);

    Rect2 rect_close(static_cast<float>(win_sz.width) - BTN_WIDTH, 0.0f, BTN_WIDTH, TITLE_BAR_HEIGHT);
    Rect2 rect_max(static_cast<float>(win_sz.width) - (BTN_WIDTH * 2.0f), 0.0f, BTN_WIDTH, TITLE_BAR_HEIGHT);
    Rect2 rect_min(static_cast<float>(win_sz.width) - (BTN_WIDTH * 3.0f), 0.0f, BTN_WIDTH, TITLE_BAR_HEIGHT);

    bool hover_close = (mx >= rect_close.position.x && mx <= rect_close.position.x + rect_close.size.width && my >= 0 && my <= TITLE_BAR_HEIGHT);
    bool hover_max   = (mx >= rect_max.position.x && mx <= rect_max.position.x + rect_max.size.width && my >= 0 && my <= TITLE_BAR_HEIGHT);
    bool hover_min   = (mx >= rect_min.position.x && mx <= rect_min.position.x + rect_min.size.width && my >= 0 && my <= TITLE_BAR_HEIGHT);

    // Actualización del cursor
    if (m_resize_direction == ResizeDirection::None && !m_is_dragging) {
        if (my > TITLE_BAR_HEIGHT) {
            auto hit_dir = hit_test_resize_borders(cursor_pos, 8);
            update_cursor_icon(hit_dir);
        } else {
            update_cursor_icon(ResizeDirection::None);
        }
    }

    // Procesamiento de clics
    if (mouse_left == GLFW_PRESS) {
        if (!m_is_dragging && m_resize_direction == ResizeDirection::None) {
            if (hover_close) {
                close();
            } else if (hover_max) {
                toggle_maximize();
            } else if (hover_min) {
                minimize();
            } else if (my <= TITLE_BAR_HEIGHT) {
                start_dragging();
            } else {
                auto dir = hit_test_resize_borders(cursor_pos, 8);
                if (dir != ResizeDirection::None) {
                    start_resizing(dir);
                }
            }
        }
    } else {
        stop_dragging();
        stop_resizing();
    }
}

void Window::draw_title_bar(RenderingGeometry& p_geometry) {
    GLFWwindow* win = static_cast<GLFWwindow*>(rendering_device.get_native_handle());
    if (!win) return;

    Size2i win_sz = get_size();

    constexpr float TITLE_BAR_HEIGHT = 28.0f;
    constexpr float BTN_WIDTH = 34.0f;
    constexpr float BTN_HEIGHT = TITLE_BAR_HEIGHT;

    double mx, my;
    glfwGetCursorPos(win, &mx, &my);

    Rect2 rect_close(static_cast<float>(win_sz.width) - BTN_WIDTH, 0.0f, BTN_WIDTH, BTN_HEIGHT);
    Rect2 rect_max(static_cast<float>(win_sz.width) - (BTN_WIDTH * 2.0f), 0.0f, BTN_WIDTH, BTN_HEIGHT);
    Rect2 rect_min(static_cast<float>(win_sz.width) - (BTN_WIDTH * 3.0f), 0.0f, BTN_WIDTH, BTN_HEIGHT);

    bool hover_close = (mx >= rect_close.position.x && mx <= rect_close.position.x + rect_close.size.width && my >= 0 && my <= BTN_HEIGHT);
    bool hover_max   = (mx >= rect_max.position.x && mx <= rect_max.position.x + rect_max.size.width && my >= 0 && my <= BTN_HEIGHT);
    bool hover_min   = (mx >= rect_min.position.x && mx <= rect_min.position.x + rect_min.size.width && my >= 0 && my <= BTN_HEIGHT);

    // Fondo
    p_geometry.draw_rect(Rect2(0, 0, static_cast<float>(win_sz.width), TITLE_BAR_HEIGHT), Color(0.10f, 0.10f, 0.13f, 1.0f), true);

    // Título de la ventana
    float text_x = 12.0f;
    float text_y = (TITLE_BAR_HEIGHT - 8.0f) * 0.5f;
    p_geometry.draw_text(Vector2(text_x, text_y), title, Color(0.85f, 0.85f, 0.90f, 1.0f), 1.0f);

    // Botones con efecto hover
    Color bg_close = hover_close ? Color(0.88f, 0.18f, 0.18f, 1.0f) : Color(0.10f, 0.10f, 0.13f, 0.0f);
    Color bg_max   = hover_max   ? Color(0.25f, 0.25f, 0.32f, 1.0f) : Color(0.10f, 0.10f, 0.13f, 0.0f);
    Color bg_min   = hover_min   ? Color(0.25f, 0.25f, 0.32f, 1.0f) : Color(0.10f, 0.10f, 0.13f, 0.0f);

    p_geometry.draw_rect(rect_close, bg_close, true);
    p_geometry.draw_rect(rect_max, bg_max, true);
    p_geometry.draw_rect(rect_min, bg_min, true);

    // Iconos vectoriales
    Color icon_color(0.85f, 0.85f, 0.88f, 1.0f);

    // Minimizar (-)
    float min_cx = rect_min.position.x + (BTN_WIDTH * 0.5f);
    float min_cy = TITLE_BAR_HEIGHT * 0.5f;
    p_geometry.draw_line(Vector2(min_cx - 4.5f, min_cy), Vector2(min_cx + 4.5f, min_cy), icon_color, 1.2f);

    // Maximizar / Restaurar
    float max_cx = rect_max.position.x + (BTN_WIDTH * 0.5f);
    float max_cy = TITLE_BAR_HEIGHT * 0.5f;
    if (!m_is_maximized) {
        p_geometry.draw_rect(Rect2(max_cx - 4.5f, max_cy - 4.5f, 9.0f, 9.0f), icon_color, false);
    } else {
        p_geometry.draw_rect(Rect2(max_cx - 2.5f, max_cy - 5.5f, 7.0f, 7.0f), icon_color, false);
        p_geometry.draw_rect(Rect2(max_cx - 5.5f, max_cy - 2.5f, 7.0f, 7.0f), icon_color, false);
    }

    // Cerrar (X)
    float close_cx = rect_close.position.x + (BTN_WIDTH * 0.5f);
    float close_cy = TITLE_BAR_HEIGHT * 0.5f;
    p_geometry.draw_line(Vector2(close_cx - 4.0f, close_cy - 4.0f), Vector2(close_cx + 4.0f, close_cy + 4.0f), icon_color, 1.2f);
    p_geometry.draw_line(Vector2(close_cx + 4.0f, close_cy - 4.0f), Vector2(close_cx - 4.0f, close_cy + 4.0f), icon_color, 1.2f);
}

// ------------------------------------------------------------------------- //
// Bucle persistente
// ------------------------------------------------------------------------- //

void Window::run(FrameCallback p_frame_callback) {
    rendering_device.set_render_callback([this, &p_frame_callback]() {
        if (p_frame_callback) {
            p_frame_callback(0.016);
        }
    });

    #if defined(_WIN32)
    OSWindows::trim_memory();
    #endif

    double last_time = glfwGetTime();

    while (!should_close()) {
        poll_events();
        process_window_interaction();

        if (m_is_dragging) {
            update_dragging();
        } else if (m_resize_direction != ResizeDirection::None) {
            update_resizing();
        }

        double current_time = glfwGetTime();
        double delta = current_time - last_time;
        if (delta > 0.05) delta = 0.016;
        last_time = current_time;

        if (p_frame_callback) {
            p_frame_callback(delta);
        }

        swap_buffers();
    }

    rendering_device.set_render_callback(nullptr);
}