// ========================================================================= //
// Puzzle Engine - 2D/3D Hybrid Game Engine
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// ========================================================================= //

#include "job_system.h"

JobSystem& JobSystem::get() {
    static JobSystem instance;
    return instance;
}

JobSystem::JobSystem() 
    : m_stop(false), m_initialized(false) {}

JobSystem::~JobSystem() {
    shutdown();
}

void JobSystem::initialize(size_t p_thread_count) {
    if (m_initialized) return;

    if (p_thread_count == 0) {
        size_t hw = std::thread::hardware_concurrency();
        p_thread_count = (hw > 1) ? (hw - 1) : 1;
    }

    m_stop = false;
    m_workers.reserve(p_thread_count);

    for (size_t i = 0; i < p_thread_count; ++i) {
        m_workers.emplace_back([this]() {
            worker_loop();
        });
    }

    m_initialized = true;
}

void JobSystem::enqueue_raw(std::function<void()> task) {
    {
        std::unique_lock<std::mutex> lock(m_queue_mutex);
        if (m_stop) return;
        m_tasks.push(std::move(task));
    }
    m_cv.notify_one();
}

void JobSystem::parallel_for(size_t count, const std::function<void(size_t start, size_t end)>& loop_func) {
    if (count == 0) return;

    size_t thread_count = m_workers.size();
    if (thread_count == 0 || count < thread_count * 2) {
        loop_func(0, count);
        return;
    }

    // Usamos shared_ptr para el estado de la barrera para evitar colapsos de memoria en stack
    struct BarrierState {
        std::atomic<size_t> remaining;
        std::mutex mutex;
        std::condition_variable cv;
        explicit BarrierState(size_t total) : remaining(total) {}
    };

    auto barrier = std::make_shared<BarrierState>(thread_count);
    size_t chunk_size = count / thread_count;

    for (size_t i = 0; i < thread_count; ++i) {
        size_t start = i * chunk_size;
        size_t end = (i == thread_count - 1) ? count : start + chunk_size;

        enqueue_raw([start, end, &loop_func, barrier]() {
            loop_func(start, end);
            if (barrier->remaining.fetch_sub(1, std::memory_order_acq_rel) == 1) {
                std::unique_lock<std::mutex> lock(barrier->mutex);
                barrier->cv.notify_one();
            }
        });
    }

    std::unique_lock<std::mutex> lock(barrier->mutex);
    barrier->cv.wait(lock, [&barrier]() {
        return barrier->remaining.load(std::memory_order_acquire) == 0;
    });
}

void JobSystem::shutdown() {
    if (!m_initialized) return;

    {
        std::unique_lock<std::mutex> lock(m_queue_mutex);
        m_stop = true;
    }
    m_cv.notify_all();

    for (std::thread& worker : m_workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }

    m_workers.clear();
    m_initialized = false;
}

size_t JobSystem::get_worker_count() const {
    return m_workers.size();
}

void JobSystem::worker_loop() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(m_queue_mutex);
            m_cv.wait(lock, [this]() {
                return m_stop || !m_tasks.empty();
            });

            if (m_stop && m_tasks.empty()) {
                return;
            }

            task = std::move(m_tasks.front());
            m_tasks.pop();
        }
        if (task) {
            task();
        }
    }
}