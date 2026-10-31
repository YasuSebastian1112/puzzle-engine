// =============================================================================
// PUZZLE ENGINE — 2D/3D GAME ENGINE
// -----------------------------------------------------------------------------
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// =============================================================================

#include "core/time/engine_time.h"
#include <thread>

void Time::init() {
    start_time = std::chrono::high_resolution_clock::now();
    prev_time = start_time;
    frame_start_time = start_time;
    delta = 0.0f;
    time_scale = 1.0f;
    total_time = 0.0;
    frame_count = 0;
    current_fps = 0;
    fps_counter = 0;
    fps_timer = 0.0f;
}

void Time::update() {
    auto current_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsed = current_time - prev_time;
    prev_time = current_time;

    delta = elapsed.count();
    
    // Límite de seguridad contra espirales de la muerte / congelamientos (clamp a 10 FPS mínimo)
    if (delta > 0.1f) {
        delta = 0.1f;
    }

    total_time = std::chrono::duration<double>(current_time - start_time).count();
    frame_count++;

    // Cálculo de FPS estables por segundo
    fps_counter++;
    fps_timer += delta;
    if (fps_timer >= 1.0f) {
        current_fps = fps_counter;
        fps_counter = 0;
        fps_timer -= 1.0f;
    }

    frame_start_time = std::chrono::high_resolution_clock::now();
}

uint64_t Time::get_ticks_msec() {
    auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();
}

void Time::delay_to_target_fps() {
    if (target_fps == 0) return;

    const double target_frame_time = 1.0 / static_cast<double>(target_fps);

    auto current_now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> frame_work = current_now - frame_start_time;
    double sleep_seconds = target_frame_time - frame_work.count();

    // Si aún sobra tiempo antes de que empiece el siguiente fotograma, liberamos la CPU
    if (sleep_seconds > 0.001) {
        std::this_thread::sleep_for(std::chrono::duration<double>(sleep_seconds));
    }
}