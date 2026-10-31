// ========================================================================= //
// PUZZLE ENGINE - 2D/3D GAME ENGINE
// ------------------------------------------------------------------------- //
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// ========================================================================= //

#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <memory>
#include <type_traits>
#include <atomic>

class JobSystem {
public:
    // Acceso Singleton global
    static JobSystem& get();

    // Inicialización y apagado del pool de hilos
    void initialize(size_t p_thread_count = 0);
    void shutdown();

    // Encolado básico sin retorno
    void enqueue_raw(std::function<void()> task);

    // Encolado con retorno de std::future
    template<typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args) 
        -> std::future<typename std::invoke_result<F, Args...>::type> {
        
        using return_type = typename std::invoke_result<F, Args...>::type;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        std::future<return_type> res = task->get_future();
        enqueue_raw([task]() { (*task)(); });
        return res;
    }

    // Paralelización particionada optimizada (el hilo invocador procesa parte del trabajo)
    void parallel_for(size_t count, const std::function<void(size_t start, size_t end)>& loop_func);

    size_t get_worker_count() const;
    bool is_initialized() const { return m_initialized; }

private:
    JobSystem();
    ~JobSystem();

    JobSystem(const JobSystem&) = delete;
    JobSystem& operator=(const JobSystem&) = delete;

    void worker_loop();

    std::vector<std::thread> m_workers;
    std::queue<std::function<void()>> m_tasks;
    std::mutex m_queue_mutex;
    std::condition_variable m_cv;
    std::atomic<bool> m_stop;
    bool m_initialized;
};