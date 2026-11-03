// ========================================================================= //
// PUZZLE ENGINE - 2D/3D GAME ENGINE
// ------------------------------------------------------------------------- //
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// ========================================================================= //

#pragma once

#include "command_dispatcher.h"
#include <thread>
#include <mutex>
#include <queue>
#include <vector>
#include <atomic>
#include <string>

class EngineConsole {
public:
    EngineConsole();
    ~EngineConsole();

    void start();
    void stop();

    CommandDispatcher& get_dispatcher();
    void set_scene_root(Node* p_root) { dispatcher.set_scene_root(p_root); }
    void process_commands();

    void render_ui();
    void print_history() const;
    void clear_history();

private:
    CommandDispatcher dispatcher;
    std::queue<std::string> pending_commands;
    std::vector<std::string> command_history;
    std::mutex queue_mutex;
    std::thread input_thread;
    std::atomic<bool> is_running{false};

    void read_input_loop();
    void print_logo();
    void print_prompt_box();
};