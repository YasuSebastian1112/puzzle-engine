// ========================================================================= //
// PUZZLE ENGINE - 2D/3D GAME ENGINE
// ------------------------------------------------------------------------- //
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
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

    size_t worker_count = m_workers.size();
    // Ejecución secuencial si no hay workers o la carga de trabajo es trivial
    if (worker_count == 0 || count < (worker_count + 1) * 2) {
        loop_func(0, count);
        return;
    }

    // Dividir entre workers + hilo principal para no desperdiciar ciclos del llamador
    size_t total_slices = worker_count + 1;
    size_t chunk_size = count / total_slices;

    // Barrera en stack local sin asignaciones dinámicas en heap
    struct LocalBarrier {
        std::atomic<size_t> remaining;
        std::mutex mutex;
        std::condition_variable cv;
        explicit LocalBarrier(size_t total) : remaining(total) {}
    } barrier(worker_count);

    // Encolar las porciones destinadas a los hilos trabajadores
    for (size_t i = 0; i < worker_count; ++i) {
        size_t start = i * chunk_size;
        size_t end = start + chunk_size;

        enqueue_raw([start, end, &loop_func, &barrier]() {
            loop_func(start, end);
            if (barrier.remaining.fetch_sub(1, std::memory_order_acq_rel) == 1) {
                std::unique_lock<std::mutex> lock(barrier.mutex);
                barrier.cv.notify_one();
            }
        });
    }

    // El hilo principal computa inmediatamente la última porción sin bloquearse
    size_t main_start = worker_count * chunk_size;
    size_t main_end = count;
    loop_func(main_start, main_end);

    // Espera activa a que los workers terminen su fracción de carga
    if (barrier.remaining.load(std::memory_order_acquire) > 0) {
        std::unique_lock<std::mutex> lock(barrier.mutex);
        barrier.cv.wait(lock, [&barrier]() {
            return barrier.remaining.load(std::memory_order_acquire) == 0;
        });
    }
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