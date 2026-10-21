// ========================================================================= //
// Puzzle Engine - 2D/3D Hybrid Game Engine
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// ========================================================================= //

#include <cstdint>
#include <chrono>

class Time {
public:
    static void init();
    static void update();

    // Tiempo transcurrido entre el frame anterior y el actual (en segundos)
    static float get_delta() { return delta; }
    
    // Delta con time_scale aplicado (útil para slow-motion, pausa, etc.)
    static float get_process_delta() { return delta * time_scale; }

    // Tiempo total desde que arrancó el motor (en segundos y milisegundos)
    static double get_ticks_sec() { return total_time; }
    static uint64_t get_ticks_msec();

    // Escala de tiempo (1.0 = normal, 0.5 = mitad de velocidad, 0.0 = pausa)
    static void set_time_scale(float p_scale) { time_scale = p_scale; }
    static float get_time_scale() { return time_scale; }

    // Estadísticas de rendimiento
    static uint32_t get_fps() { return current_fps; }
    static uint64_t get_frame_count() { return frame_count; }

private:
    static inline std::chrono::high_resolution_clock::time_point start_time;
    static inline std::chrono::high_resolution_clock::time_point prev_time;
    
    static inline float delta = 0.0f;
    static inline float time_scale = 1.0f;
    static inline double total_time = 0.0;
    
    static inline uint64_t frame_count = 0;
    static inline uint32_t current_fps = 0;
    static inline uint32_t fps_counter = 0;
    static inline float fps_timer = 0.0f;
};