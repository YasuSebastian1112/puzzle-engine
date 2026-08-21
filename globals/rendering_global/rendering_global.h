#pragma once
#include "window.h"
#include <vector>
#include <memory>

class RenderingGlobal {
public:
    static RenderingGlobal& get();

    bool init();
    void poll_events();
    void shutdown();

    std::shared_ptr<Window> create_window(int width, int height, const std::string& title, Window::Type type = Window::Type::Game);
    std::shared_ptr<Window> get_primary_window() const;
    const std::vector<std::shared_ptr<Window>>& get_windows() const { return windows; }

    void clear_screen(float r, float g, float b, float a);

private:
    RenderingGlobal() = default;
    ~RenderingGlobal() = default;

    bool glad_initialized = false;
    std::vector<std::shared_ptr<Window>> windows;
};
