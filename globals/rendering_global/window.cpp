// =============================================================================
// PUZZLE ENGINE — 2D/3D GAME ENGINE
// -----------------------------------------------------------------------------
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// =============================================================================

#include "window.h"
#include <GLFW/glfw3.h>

#if defined(_WIN32)
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <windows.h>
#include "platform/system_info.h"
#include "platform/windows/os_windows.h"
#include "core/time/engine_time.h"
#include "licenses_generated/logo.h"
#include "globals/input/input.h"

static const wchar_t* PROP_ORIG_WNDPROC = L"PuzzleEngine_OrigWndProc";

static LRESULT CALLBACK custom_borderless_wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    WNDPROC orig_proc = reinterpret_cast<WNDPROC>(GetPropW(hwnd, PROP_ORIG_WNDPROC));

    switch (msg) {
        case WM_NCCALCSIZE: {
            if (wparam == TRUE) {
                return 0;
            }
            break;
        }

        // Se envía una vez que el usuario suelta el borde tras redimensionar o mover
        case WM_EXITSIZEMOVE: {
            SystemInfo::trim_memory();
            break;
        }

        case WM_SYSCOMMAND: {
            if (orig_proc) {
                return CallWindowProc(orig_proc, hwnd, msg, wparam, lparam);
            }
            break;
        }
    }

    if (orig_proc) {
        return CallWindowProc(orig_proc, hwnd, msg, wparam, lparam);
    }
    return DefWindowProc(hwnd, msg, wparam, lparam);
}
#endif

void Window::enable_native_windows_snap(void* p_native_window_handle) {
#if defined(_WIN32)
    HWND hwnd = static_cast<HWND>(p_native_window_handle);
    OSWindows::enable_borderless_snap(hwnd);
#endif
}

void Window::disable_native_windows_snap(void* p_native_window_handle) {
    // Ya se encarga OSWindows::remove_window_hooks en RenderingDevice::destroy_window()
}

Window::Window() : m_geometry(&rendering_device) {}

Window::~Window() {
    // Desvincular cualquier callback de renderizado en el dispositivo gráfico para evitar punteros huérfanos
    rendering_device.set_render_callback(nullptr);

    GLFWwindow* win = static_cast<GLFWwindow*>(rendering_device.get_native_handle());
    if (win) {
#if defined(_WIN32)
        HWND hwnd = glfwGetWin32Window(win);
        disable_native_windows_snap(hwnd);
#endif
        if (m_cursor_arrow)   glfwDestroyCursor(m_cursor_arrow);
        if (m_cursor_hresize) glfwDestroyCursor(m_cursor_hresize);
        if (m_cursor_vresize) glfwDestroyCursor(m_cursor_vresize);
        if (m_cursor_nesw)    glfwDestroyCursor(m_cursor_nesw);
        if (m_cursor_nwse)    glfwDestroyCursor(m_cursor_nwse);
    }

    // Liberar la textura del icono antes de destruir el contexto OpenGL
    if (m_icon_loaded) {
        m_title_bar_icon.release();
        m_icon_loaded = false;
    }

    rendering_device.destroy_window();
}

bool Window::initialize(const String& p_title, const Size2i& p_size, const Point2i& p_pos, Type p_type) {
    title = p_title;
    type = p_type;
    position = p_pos;

    // Garantizar que la ventana física reserve el espacio de la barra si es de tipo Editor
    int title_padding = (type == Type::Editor) ? TITLE_BAR_HEIGHT : 0;
    Size2i physical_window_size(p_size.width, p_size.height + title_padding);

    size = physical_window_size;
    m_saved_windowed_pos = p_pos;
    m_saved_windowed_size = physical_window_size;

    if (!rendering_device.create_window(p_title, physical_window_size, p_pos)) {
        return false;
    }

    GLFWwindow* win = static_cast<GLFWwindow*>(rendering_device.get_native_handle());
    if (win) {
        m_cursor_arrow = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
        m_cursor_hresize = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
        m_cursor_vresize = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
        m_cursor_nesw = glfwCreateStandardCursor(GLFW_RESIZE_NESW_CURSOR);
        m_cursor_nwse = glfwCreateStandardCursor(GLFW_RESIZE_NWSE_CURSOR);

        // Único propietario del UserPointer de GLFW: evita colisiones de tipos entre Window y RenderingDevice
        glfwSetWindowUserPointer(win, this);

        // Sincroniza el framebuffer y los viewports cuando el SO cambia el tamaño de la ventana
        glfwSetFramebufferSizeCallback(win, [](GLFWwindow* p_win, int width, int height) {
            Window* self = static_cast<Window*>(glfwGetWindowUserPointer(p_win));
            if (!self) return;

            if (width > 0 && height > 0) {
                self->size = Size2i(width, height);
                self->rendering_device.set_screen_size(Size2i(width, height));
                self->rendering_device.set_viewport(Point2i(0, 0), Size2i(width, height));
                self->update_viewports(width, height);
            }
        });

        // Repintado durante redimensionado o eventos modales
        glfwSetWindowRefreshCallback(win, [](GLFWwindow* p_win) {
            Window* self = static_cast<Window*>(glfwGetWindowUserPointer(p_win));
            if (!self) return;

            self->rendering_device.trigger_frame_update();
        });

        // Sincroniza el estado al restaurar la ventana desde la barra de tareas tras haber sido minimizada
        glfwSetWindowIconifyCallback(win, [](GLFWwindow* p_win, int p_iconified) {
            Window* self = static_cast<Window*>(glfwGetWindowUserPointer(p_win));
            if (!self) return;

            if (p_iconified) {
                self->current_mode = Mode::Minimized;
            } else {
                int is_max = glfwGetWindowAttrib(p_win, GLFW_MAXIMIZED);
                self->current_mode = is_max ? Mode::Maximized : Mode::Windowed;
            }
        });

        // Sincroniza el estado cuando la ventana se maximiza o restaura mediante el sistema operativo (Win + Flechas, etc.)
        glfwSetWindowMaximizeCallback(win, [](GLFWwindow* p_win, int p_maximized) {
            Window* self = static_cast<Window*>(glfwGetWindowUserPointer(p_win));
            if (!self) return;

            self->current_mode = p_maximized ? Mode::Maximized : Mode::Windowed;
        });

        // --- SISTEMA DE ENTRADA (INPUT) INTEGRADO ---

        // Corrección de coordenadas del cursor: compensa el desfase de la barra de título personalizada
        glfwSetCursorPosCallback(win, [](GLFWwindow* p_win, double xpos, double ypos) {
            Window* self = static_cast<Window*>(glfwGetWindowUserPointer(p_win));
            if (!self) return;

            // Resta el alto de la barra superior para alinear el origen (Y=0) con el lienzo de trabajo
            int title_offset = (self->type == Type::Editor) ? TITLE_BAR_HEIGHT : 0;
            double local_y = ypos - static_cast<double>(title_offset);

            Input::on_mouse_moved_event(xpos, local_y);
        });

        glfwSetMouseButtonCallback(win, [](GLFWwindow*, int button, int action, int mods) {
            Input::on_mouse_button_event(button, action);
        });

        glfwSetScrollCallback(win, [](GLFWwindow*, double xoffset, double yoffset) {
            Input::on_mouse_scrolled_event(xoffset, yoffset);
        });

        glfwSetKeyCallback(win, [](GLFWwindow*, int key, int scancode, int action, int mods) {
            Input::on_key_event(key, action);
        });

#if defined(_WIN32)
        HWND hwnd = glfwGetWin32Window(win);
        enable_native_windows_snap(hwnd);
#endif
    }

    if (type == Type::Editor) {
        set_icon_from_memory(EmbeddedAssets::PUZZLE_LOGO_PNG, EmbeddedAssets::PUZZLE_LOGO_PNG_SIZE);
    }

    // Configuración inicial de los límites geométricos de cada Viewport
    update_viewports(physical_window_size.width, physical_window_size.height);

    return true;
}

void Window::set_title(const String& p_title) {
    title = p_title;
    GLFWwindow* win = static_cast<GLFWwindow*>(rendering_device.get_native_handle());
    if (win) {
        glfwSetWindowTitle(win, p_title.c_str());
    }
}

bool Window::set_icon(const String& p_filepath) {
    if (m_icon_loaded) {
        m_title_bar_icon.release();
        m_icon_loaded = false;
    }

    if (m_title_bar_icon.load_from_file(p_filepath.c_str(), false)) {
        m_icon_loaded = true;
        return true;
    }
    return false;
}

bool Window::set_icon_from_memory(const uint8_t* p_buffer, size_t p_size) {
    if (m_icon_loaded) {
        m_title_bar_icon.release();
        m_icon_loaded = false;
    }

    if (m_title_bar_icon.load_from_memory(p_buffer, p_size, false)) {
        m_icon_loaded = true;
        return true;
    }
    return false;
}

void Window::clear_icon() {
    if (m_icon_loaded) {
        m_title_bar_icon.release();
        m_icon_loaded = false;
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
    rendering_device.set_scissor_enabled(false);
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
    // Si la ventana requiere barra de título, agregamos su compensación vertical al SO
    int title_padding = (type == Type::Editor) ? TITLE_BAR_HEIGHT : 0;
    Size2i physical_size(p_size.width, p_size.height + title_padding);
    size = physical_size;
    rendering_device.set_window_size(physical_size);
    update_viewports(physical_size.width, physical_size.height);
}

Size2i Window::get_size() const {
    // Retorna estrictamente el tamaño del lienzo de renderizado disponible para el juego
    Size2i physical_size = rendering_device.get_window_size();
    int title_padding = (type == Type::Editor) ? TITLE_BAR_HEIGHT : 0;
    int game_h = physical_size.height - title_padding;
    return Size2i(physical_size.width, game_h > 0 ? game_h : 0);
}

Size2i Window::get_os_window_size() const {
    return rendering_device.get_window_size();
}

// -------------------------------------------------------------------------
// GESTIÓN DE MODOS DE VENTANA (ESTILO GODOT)
// -------------------------------------------------------------------------

void Window::set_mode(Mode p_mode) {
    GLFWwindow* win = static_cast<GLFWwindow*>(rendering_device.get_native_handle());
    if (!win) return;

    if (current_mode == p_mode) return;

    // Cancelar estados activos de arrastre o redimensionado antes de cambiar de modo
    m_is_dragging = false;
    m_resize_direction = ResizeDirection::None;

    // Resguardar posición de ventana previa al cambio a modos de pantalla completa o maximizado
    if (current_mode == Mode::Windowed) {
        glfwGetWindowPos(win, &m_saved_windowed_pos.x, &m_saved_windowed_pos.y);
        glfwGetWindowSize(win, &m_saved_windowed_size.width, &m_saved_windowed_size.height);
    }

    current_mode = p_mode;

    switch (p_mode) {
        case Mode::Windowed: {
            glfwSetWindowMonitor(win, nullptr, m_saved_windowed_pos.x, m_saved_windowed_pos.y,
                                 m_saved_windowed_size.width, m_saved_windowed_size.height, 0);
            update_viewports(m_saved_windowed_size.width, m_saved_windowed_size.height);
            break;
        }
        case Mode::Minimized: {
#if defined(_WIN32)
            HWND hwnd = glfwGetWin32Window(win);
            if (hwnd) {
                ShowWindow(hwnd, SW_MINIMIZE);
            } else {
                glfwIconifyWindow(win);
            }
#else
            glfwIconifyWindow(win);
#endif
            break;
        }
        case Mode::Maximized: {
            GLFWmonitor* monitor = glfwGetPrimaryMonitor();
            if (monitor) {
                int mx, my, mw, mh;
                glfwGetMonitorWorkarea(monitor, &mx, &my, &mw, &mh);
                glfwSetWindowMonitor(win, nullptr, mx, my, mw, mh, 0);
                update_viewports(mw, mh);
            }
            break;
        }
        case Mode::Fullscreen: {
            // Pantalla completa sin bordes (Borderless)
            GLFWmonitor* monitor = glfwGetPrimaryMonitor();
            if (monitor) {
                const GLFWvidmode* vm = glfwGetVideoMode(monitor);
                if (vm) {
                    glfwSetWindowMonitor(win, nullptr, 0, 0, vm->width, vm->height, 0);
                    update_viewports(vm->width, vm->height);
                }
            }
            break;
        }
        case Mode::ExclusiveFullscreen: {
            // Pantalla completa exclusiva directa al monitor
            GLFWmonitor* monitor = glfwGetPrimaryMonitor();
            if (monitor) {
                const GLFWvidmode* vm = glfwGetVideoMode(monitor);
                if (vm) {
                    glfwSetWindowMonitor(win, monitor, 0, 0, vm->width, vm->height, vm->refreshRate);
                    update_viewports(vm->width, vm->height);
                }
            }
            break;
        }
    }

#if defined(_WIN32)
    OSWindows::trim_memory();
#endif
}

void Window::minimize() {
    set_mode(Mode::Minimized);
}

void Window::maximize() {
    set_mode(Mode::Maximized);
}

void Window::restore() {
    set_mode(Mode::Windowed);
}

void Window::toggle_maximize() {
    if (current_mode == Mode::Maximized) {
        restore();
    } else {
        maximize();
    }
}

// -------------------------------------------------------------------------
// ARRASTRE DE VENTANA
// -------------------------------------------------------------------------

void Window::start_dragging() {
    GLFWwindow* win = static_cast<GLFWwindow*>(rendering_device.get_native_handle());
    if (!win || current_mode != Mode::Windowed) return;

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
    if (!win || current_mode != Mode::Windowed) {
        m_is_dragging = false;
        return;
    }

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

// -------------------------------------------------------------------------
// REDIMENSIONAMIENTO DE BORDES
// -------------------------------------------------------------------------

Window::ResizeDirection Window::hit_test_resize_borders(const Point2i& p_cursor_pos, int p_border_thickness) const {
    if (current_mode != Mode::Windowed) return ResizeDirection::None;

    Size2i sz = get_os_window_size();
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
    if (p_dir == ResizeDirection::None || current_mode != Mode::Windowed) return;
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
    if (!win || current_mode != Mode::Windowed) {
        m_resize_direction = ResizeDirection::None;
        return;
    }

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
    update_viewports(new_w, new_h);
}

void Window::stop_resizing() {
    if (m_resize_direction != ResizeDirection::None) {
        m_resize_direction = ResizeDirection::None;
#if defined(_WIN32)
        OSWindows::trim_memory();
#endif
    }
}

// -------------------------------------------------------------------------
// INTERACCIÓN Y HIT-TESTING DE CONTROL
// -------------------------------------------------------------------------

void Window::process_window_interaction() {
    GLFWwindow* win = static_cast<GLFWwindow*>(rendering_device.get_native_handle());
    if (!win) return;

    Size2i win_sz = get_os_window_size();
    if (win_sz.width <= 0 || win_sz.height <= 0) return;

    constexpr float BAR_HEIGHT = static_cast<float>(TITLE_BAR_HEIGHT);
    constexpr float BTN_WIDTH = 34.0f;

    double mx, my;
    glfwGetCursorPos(win, &mx, &my);
    Point2i cursor_pos(static_cast<int>(mx), static_cast<int>(my));

    static int prev_mouse_left = GLFW_RELEASE;
    int mouse_left = glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT);
    bool just_pressed = (mouse_left == GLFW_PRESS && prev_mouse_left == GLFW_RELEASE);
    prev_mouse_left = mouse_left;

    Rect2 rect_close(static_cast<float>(win_sz.width) - BTN_WIDTH, 0.0f, BTN_WIDTH, BAR_HEIGHT);
    Rect2 rect_max(static_cast<float>(win_sz.width) - (BTN_WIDTH * 2.0f), 0.0f, BTN_WIDTH, BAR_HEIGHT);
    Rect2 rect_min(static_cast<float>(win_sz.width) - (BTN_WIDTH * 3.0f), 0.0f, BTN_WIDTH, BAR_HEIGHT);

    bool hover_close = (mx >= rect_close.position.x && mx <= rect_close.position.x + rect_close.size.width && my >= 0 && my <= BAR_HEIGHT);
    bool hover_max   = (mx >= rect_max.position.x && mx <= rect_max.position.x + rect_max.size.width && my >= 0 && my <= BAR_HEIGHT);
    bool hover_min   = (mx >= rect_min.position.x && mx <= rect_min.position.x + rect_min.size.width && my >= 0 && my <= BAR_HEIGHT);

    // Actualización de forma del cursor según cercanía a bordes
    if (m_resize_direction == ResizeDirection::None && !m_is_dragging) {
        if (my > BAR_HEIGHT || type != Type::Editor) {
            auto hit_dir = hit_test_resize_borders(cursor_pos, 8);
            update_cursor_icon(hit_dir);
        } else {
            update_cursor_icon(ResizeDirection::None);
        }
    }

    // Procesamiento de clics y doble clic en la barra personalizada
    if (just_pressed) {
        if (!m_is_dragging && m_resize_direction == ResizeDirection::None) {
            if (type == Type::Editor && my <= BAR_HEIGHT) {
                if (hover_close) {
                    close();
                } else if (hover_max) {
                    toggle_maximize();
                } else if (hover_min) {
                    minimize();
                } else {
                    double current_time = glfwGetTime();
                    double click_interval = current_time - m_last_titlebar_click_time;

                    // Doble clic sobre la barra de título para alternar maximizado
                    if (click_interval <= 0.28) {
                        toggle_maximize();
                        m_last_titlebar_click_time = 0.0;
                    } else {
                        m_last_titlebar_click_time = current_time;
                        start_dragging();
                    }
                }
            } else {
                auto dir = hit_test_resize_borders(cursor_pos, 8);
                if (dir != ResizeDirection::None) {
                    start_resizing(dir);
                }
            }
        }
    } else if (mouse_left == GLFW_RELEASE) {
        stop_dragging();
        stop_resizing();
    }
}

void Window::draw_title_bar(RenderingGeometry& p_geometry) {
    GLFWwindow* win = static_cast<GLFWwindow*>(rendering_device.get_native_handle());
    if (!win) return;

    Size2i win_sz = get_os_window_size();
    if (win_sz.width < 150 || win_sz.height < TITLE_BAR_HEIGHT) {
        return;
    }

    constexpr float BAR_HEIGHT = static_cast<float>(TITLE_BAR_HEIGHT);
    constexpr float BTN_WIDTH = 34.0f;
    constexpr float BTN_HEIGHT = BAR_HEIGHT;

    double mx, my;
    glfwGetCursorPos(win, &mx, &my);

    Rect2 rect_close(static_cast<float>(win_sz.width) - BTN_WIDTH, 0.0f, BTN_WIDTH, BTN_HEIGHT);
    Rect2 rect_max(static_cast<float>(win_sz.width) - (BTN_WIDTH * 2.0f), 0.0f, BTN_WIDTH, BTN_HEIGHT);
    Rect2 rect_min(static_cast<float>(win_sz.width) - (BTN_WIDTH * 3.0f), 0.0f, BTN_WIDTH, BTN_HEIGHT);

    bool hover_close = (mx >= rect_close.position.x && mx <= rect_close.position.x + rect_close.size.width && my >= 0 && my <= BTN_HEIGHT);
    bool hover_max   = (mx >= rect_max.position.x && mx <= rect_max.position.x + rect_max.size.width && my >= 0 && my <= BTN_HEIGHT);
    bool hover_min   = (mx >= rect_min.position.x && mx <= rect_min.position.x + rect_min.size.width && my >= 0 && my <= BTN_HEIGHT);

    // Fondo de la barra
    p_geometry.draw_rect(Rect2(0, 0, static_cast<float>(win_sz.width), BAR_HEIGHT), Color(0.10f, 0.10f, 0.13f, 1.0f), true);

    // Icono del motor en la esquina superior izquierda
    float icon_offset_x = 8.0f;
    if (m_icon_loaded) {
        constexpr float ICON_SIZE = 18.0f;
        float icon_y = (BAR_HEIGHT - ICON_SIZE) * 0.5f;
        p_geometry.draw_texture(Rect2(icon_offset_x, icon_y, ICON_SIZE, ICON_SIZE), m_title_bar_icon);
        icon_offset_x += ICON_SIZE + 8.0f;
    }

    // Texto del título desplazado horizontalmente
    float text_x = icon_offset_x;
    float text_y = (BAR_HEIGHT - 8.0f) * 0.5f;
    p_geometry.draw_text(Vector2(text_x, text_y), title, Color(0.85f, 0.85f, 0.90f, 1.0f), 1.0f);

    // Botones con respuesta interactiva
    Color bg_close = hover_close ? Color(0.88f, 0.18f, 0.18f, 1.0f) : Color(0.10f, 0.10f, 0.13f, 0.0f);
    Color bg_max   = hover_max   ? Color(0.25f, 0.25f, 0.32f, 1.0f) : Color(0.10f, 0.10f, 0.13f, 0.0f);
    Color bg_min   = hover_min   ? Color(0.25f, 0.25f, 0.32f, 1.0f) : Color(0.10f, 0.10f, 0.13f, 0.0f);

    p_geometry.draw_rect(rect_close, bg_close, true);
    p_geometry.draw_rect(rect_max, bg_max, true);
    p_geometry.draw_rect(rect_min, bg_min, true);

    Color icon_color(0.85f, 0.85f, 0.88f, 1.0f);

    // Icono Minimizar (-)
    float min_cx = rect_min.position.x + (BTN_WIDTH * 0.5f);
    float min_cy = BAR_HEIGHT * 0.5f;
    p_geometry.draw_line(Vector2(min_cx - 4.5f, min_cy), Vector2(min_cx + 4.5f, min_cy), icon_color, 1.2f);

    // Icono Maximizar / Restaurar
    float max_cx = rect_max.position.x + (BTN_WIDTH * 0.5f);
    float max_cy = BAR_HEIGHT * 0.5f;
    if (current_mode != Mode::Maximized) {
        p_geometry.draw_rect(Rect2(max_cx - 4.5f, max_cy - 4.5f, 9.0f, 9.0f), icon_color, false);
    } else {
        p_geometry.draw_rect(Rect2(max_cx - 2.5f, max_cy - 5.5f, 7.0f, 7.0f), icon_color, false);
        p_geometry.draw_rect(Rect2(max_cx - 5.5f, max_cy - 2.5f, 7.0f, 7.0f), icon_color, false);
    }

    // Icono Cerrar (X)
    float close_cx = rect_close.position.x + (BTN_WIDTH * 0.5f);
    float close_cy = BAR_HEIGHT * 0.5f;
    p_geometry.draw_line(Vector2(close_cx - 4.0f, close_cy - 4.0f), Vector2(close_cx + 4.0f, close_cy + 4.0f), icon_color, 1.2f);
    p_geometry.draw_line(Vector2(close_cx + 4.0f, close_cy - 4.0f), Vector2(close_cx - 4.0f, close_cy + 4.0f), icon_color, 1.2f);
}

// -------------------------------------------------------------------------
// GESTIÓN DE VIEWPORTS Y DESPACHO DE RENDERIZADO
// -------------------------------------------------------------------------

void Window::update_viewports(int p_fb_w, int p_fb_h) {
    if (p_fb_w <= 0 || p_fb_h <= 0) return;

    float title_h = (type == Type::Editor) ? static_cast<float>(TITLE_BAR_HEIGHT) : 0.0f;
    float client_h = static_cast<float>(p_fb_h) - title_h;
    if (client_h < 1.0f) client_h = 1.0f;
    float width = static_cast<float>(p_fb_w);

    // Barra de título: franja superior
    m_title_bar_viewport.set_screen_rect(Rect2(
        Vector2(0.0f, 0.0f),
        Vector2(width, title_h)
    ));

    // Espacio de juego/escena: comienza exactamente debajo de la barra
    m_game_viewport.set_screen_rect(Rect2(
        Vector2(0.0f, title_h),
        Vector2(width, client_h)
    ));
}

void Window::render_frame_passes(const FrameCallback& p_frame_callback, int p_fb_w, int p_fb_h) {
    int min_required_h = (type == Type::Editor) ? TITLE_BAR_HEIGHT : 0;

    // Evitar llamadas anidadas o con dimensiones colapsadas
    if (m_is_rendering || p_fb_w <= 0 || p_fb_h <= min_required_h) {
        return;
    }

    m_is_rendering = true;

    clear(Color(0.08f, 0.08f, 0.10f, 1.0f));

    // Pase 1: Escena / Juego dentro de su propio viewport aislado
    m_game_viewport.bind(&rendering_device, p_fb_h);
    m_geometry.begin();
    if (p_frame_callback) {
        p_frame_callback(Time::get_process_delta());
    }
    m_geometry.flush();
    m_game_viewport.unbind();

    // Pase 2: Barra de título (modo Editor)
    if (type == Type::Editor) {
        m_title_bar_viewport.bind(&rendering_device, p_fb_h);
        m_geometry.begin();
        draw_title_bar(m_geometry);
        m_geometry.flush();
        m_title_bar_viewport.unbind();
    }

    m_is_rendering = false;
}

void Window::run(FrameCallback p_frame_callback) {
#if defined(_WIN32)
    // Fija la granularidad del planificador de Windows a 1 ms
    OSWindows::initialize_system_timer();
#endif

    // Inicializar el reloj del motor
    Time::init();

    GLFWwindow* win = static_cast<GLFWwindow*>(rendering_device.get_native_handle());
    int min_required_h = (type == Type::Editor) ? TITLE_BAR_HEIGHT : 0;

    // Sincronización de dimensiones del framebuffer físico de la ventana
    auto sync_framebuffer_and_viewports = [this, win](int& w, int& h) {
        if (win) {
            glfwGetFramebufferSize(win, &w, &h);
        } else {
            w = size.width;
            h = size.height;
        }
        if (w > 0 && h > 0) {
            size = Size2i(w, h);
            rendering_device.set_screen_size(Size2i(w, h));
            update_viewports(w, h);
        }
    };

    // Callback de renderizado continuo para eventos modales del SO (arrastre / resize)
    rendering_device.set_render_callback([this, sync_framebuffer_and_viewports, &p_frame_callback, min_required_h]() {
        int fb_w = 0, fb_h = 0;
        sync_framebuffer_and_viewports(fb_w, fb_h);

        // Prevenir división por cero o viewports degenerados al restaurar
        if (fb_w <= 0 || fb_h <= min_required_h || current_mode == Mode::Minimized) {
            return;
        }

        render_frame_passes(p_frame_callback, fb_w, fb_h);
    });

    // Contador de cuadros iniciales para diferir el vaciado del Working Set
    uint32_t startup_warmup_frames = 0;

    // Bucle principal de ejecución del motor
    while (!should_close()) {
        Time::update();

        poll_events();

        if (should_close()) {
            break;
        }

        int fb_w = 0, fb_h = 0;
        sync_framebuffer_and_viewports(fb_w, fb_h);

        // Detección nativa de ventana iconificada (evita colapsar en el driver durante swapchain al restaurar)
        bool is_minimized_native = (current_mode == Mode::Minimized);
#if defined(_WIN32)
        HWND hwnd = win ? glfwGetWin32Window(win) : nullptr;
        if (hwnd && IsIconic(hwnd) != 0) {
            is_minimized_native = true;
        }
#else
        if (win && glfwGetWindowAttrib(win, GLFW_ICONIFIED) != 0) {
            is_minimized_native = true;
        }
#endif

        // Si la ventana no está iconificada y tiene área de dibujado, procesar interacción y render
        if (!is_minimized_native && fb_w > 0 && fb_h > min_required_h) {
            process_window_interaction();

            if (m_is_dragging) {
                update_dragging();
            } else if (m_resize_direction != ResizeDirection::None) {
                update_resizing();
            }

            // Verificar si durante process_window_interaction() se cerró o minimizó la ventana
            bool minimized_after_input = (current_mode == Mode::Minimized);
#if defined(_WIN32)
            if (hwnd && IsIconic(hwnd) != 0) {
                minimized_after_input = true;
            }
#endif
            if (!should_close() && !minimized_after_input) {
                // Re-sincronizar dimensiones en caso de haber maximizado/restaurado o redimensionado en este mismo frame
                sync_framebuffer_and_viewports(fb_w, fb_h);

                if (fb_w > 0 && fb_h > min_required_h) {
                    render_frame_passes(p_frame_callback, fb_w, fb_h);
                    swap_buffers();
                }
            }
        }

#if defined(_WIN32)
        // Purgar páginas transitorias de arranque (DLLs, JIT de shaders, buffers de DWM)
        if (startup_warmup_frames <= 3) {
            startup_warmup_frames++;
            if (startup_warmup_frames == 3) {
                OSWindows::trim_memory();
            }
        }
#endif

        Time::delay_to_target_fps();
    }

    // Desregistrar el callback para evitar referencias colgantes al cerrar
    rendering_device.set_render_callback(nullptr);

#if defined(_WIN32)
    // Restaura la resolución estándar del temporizador del kernel
    OSWindows::restore_system_timer();
#endif
}