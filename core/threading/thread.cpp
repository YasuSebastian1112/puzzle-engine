// ========================================================================= //
// Puzzle Engine - 2D/3D Hybrid Game Engine
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// ========================================================================= //

#include "thread.h"

Thread::Thread(Mode p_mode) 
    : m_mode(p_mode), m_running(false), m_stop(false) {}

Thread::~Thread() {
    wait_to_finish();
}

void Thread::start(std::function<void()> p_callable) {
    wait_to_finish();
    m_mode = MODE_SINGLE_TASK;
    m_running = true;
    m_thread = std::thread([this, p_callable]() {
        if (p_callable) {
            p_callable();
        }
        m_running = false;
    });
}

void Thread::start_persistent() {
    if (m_running) return;
    m_mode = MODE_PERSISTENT;
    m_stop = false;
    m_running = true;
    m_thread = std::thread(&Thread::persistent_loop, this);
}

void Thread::post_task(std::function<void()> p_task) {
    if (m_mode != MODE_PERSISTENT) return;
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_task_queue.push(std::move(p_task));
    }
    m_cv.notify_one();
}

void Thread::wait_to_finish() {
    if (m_mode == MODE_PERSISTENT) {
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_stop = true;
        }
        m_cv.notify_all();
    }

    if (m_thread.joinable()) {
        m_thread.join();
    }
    m_running = false;
}

bool Thread::is_alive() const {
    return m_running.load(std::memory_order_relaxed);
}

void Thread::persistent_loop() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cv.wait(lock, [this]() {
                return m_stop || !m_task_queue.empty();
            });

            if (m_stop && m_task_queue.empty()) {
                break;
            }

            task = std::move(m_task_queue.front());
            m_task_queue.pop();
        }
        if (task) {
            task();
        }
    }
}