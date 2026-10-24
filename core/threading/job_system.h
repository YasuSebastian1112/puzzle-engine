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
    static JobSystem& get() {
        static JobSystem instance;
        return instance;
    }

    // Inicializa el pool con N hilos (por defecto núcleos lógicos - 1)
    void initialize(size_t p_thread_count = 0) {
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

    // Encola una función y retorna un std::future con el resultado
    template<typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args) 
        -> std::future<typename std::invoke_result<F, Args...>::type> {
        
        using return_type = typename std::invoke_result<F, Args...>::type;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(m_queue_mutex);
            if (m_stop) {
                return std::future<return_type>();
            }
            m_tasks.emplace([task]() { (*task)(); });
        }
        m_cv.notify_one();
        return res;
    }

    // Divide un rango [0, count) en trozos homogéneos y los procesa en paralelo
    void parallel_for(size_t count, const std::function<void(size_t start, size_t end)>& loop_func) {
        if (count == 0) return;

        size_t thread_count = m_workers.size();
        if (thread_count == 0 || count < thread_count * 2) {
            loop_func(0, count);
            return;
        }

        size_t chunk_size = count / thread_count;
        std::vector<std::future<void>> futures;
        futures.reserve(thread_count);

        for (size_t i = 0; i < thread_count; ++i) {
            size_t start = i * chunk_size;
            size_t end = (i == thread_count - 1) ? count : start + chunk_size;

            futures.push_back(enqueue([start, end, &loop_func]() {
                loop_func(start, end);
            }));
        }

        for (auto& f : futures) {
            f.wait();
        }
    }

    void shutdown() {
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

    size_t get_worker_count() const { return m_workers.size(); }

private:
    JobSystem() : m_stop(false), m_initialized(false) {}
    ~JobSystem() { shutdown(); }

    JobSystem(const JobSystem&) = delete;
    JobSystem& operator=(const JobSystem&) = delete;

    void worker_loop() {
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
            task();
        }
    }

    std::vector<std::thread> m_workers;
    std::queue<std::function<void()>> m_tasks;
    std::mutex m_queue_mutex;
    std::condition_variable m_cv;
    std::atomic<bool> m_stop;
    bool m_initialized;
};