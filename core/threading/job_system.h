// ========================================================================= //
// Puzzle Engine - 2D/3D Hybrid Game Engine
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
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
    static JobSystem& get();

    void initialize(size_t p_thread_count = 0);
    void shutdown();

    void enqueue_raw(std::function<void()> task);

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

    void parallel_for(size_t count, const std::function<void(size_t start, size_t end)>& loop_func);

    size_t get_worker_count() const;

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