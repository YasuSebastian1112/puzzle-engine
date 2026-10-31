// ========================================================================= //
// PUZZLE ENGINE - 2D/3D GAME ENGINE
// ------------------------------------------------------------------------- //
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// ========================================================================= //

#pragma once

#include <thread>
#include <functional>
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>

class Thread {
public:
    enum Mode {
        MODE_SINGLE_TASK,
        MODE_PERSISTENT
    };

    Thread(Mode p_mode = MODE_SINGLE_TASK);
    ~Thread();

    Thread(const Thread&) = delete;
    Thread& operator=(const Thread&) = delete;

    void start(std::function<void()> p_callable);
    void start_persistent();
    void post_task(std::function<void()> p_task);
    void wait_to_finish();

    bool is_alive() const;

private:
    void persistent_loop();

    Mode m_mode;
    std::thread m_thread;
    std::atomic<bool> m_running;
    std::atomic<bool> m_stop;

    std::queue<std::function<void()>> m_task_queue;
    std::mutex m_mutex;
    std::condition_variable m_cv;
};