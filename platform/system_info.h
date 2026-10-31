// ========================================================================= //
// PUZZLE ENGINE - 2D/3D GAME ENGINE
// ------------------------------------------------------------------------- //
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// ========================================================================= //

#pragma once

#include "core/variant.h"
#include <cstdint>
#include <cstddef>

class SystemInfo {
public:
    struct MemoryStats {
        size_t process_physical_used_bytes; // Working Set real del proceso (lo que muestra el Task Manager)
        size_t process_virtual_used_bytes;  // Commit size / Espacio virtual asignado
        size_t system_total_ram_bytes;      // RAM física total del equipo
        size_t system_free_ram_bytes;       // RAM física disponible en el sistema
    };

    struct PerformanceMetrics {
        float fps;                          // Fotogramas por segundo instantáneos/promediados
        float frame_time_ms;                // Duración en milisegundos del último fotograma
        float cpu_process_usage_pct;        // Porcentaje de uso de CPU atribuible exclusivamente al proceso (0.0% a 100.0%)
        float memory_used_mb;               // Working Set en Megabytes
    };

private:
    static inline float m_current_fps = 0.0f;
    static inline float m_current_frametime = 0.0f;
    static inline float m_current_cpu_pct = 0.0f;

    // Acumuladores para el cálculo de FPS
    static inline double m_fps_timer = 0.0;
    static inline uint32_t m_frame_counter = 0;

    // Registros de tiempo para cálculo diferencial de CPU
    static inline uint64_t m_last_system_time = 0;
    static inline uint64_t m_last_process_time = 0;
    static inline uint32_t m_num_processors = 0;

public:
    // Inicialización de subsistema y capturadores internos
    static void initialize();

    // Actualización recurrente de métricas (invocar una vez al inicio o final de cada cuadro en el bucle principal)
    static void update(double p_delta);

    // Consulta de hardware y plataforma
    static String get_os_name();
    static String get_cpu_model();
    static uint32_t get_cpu_logical_core_count();
    static String get_gpu_vendor();
    static String get_gpu_renderer();
    static String get_gpu_driver_version();

    // Consulta de memoria y rendimiento
    static MemoryStats get_memory_stats();
    static float get_process_memory_mb();
    static float get_cpu_usage_percentage();
    static float get_fps() { return m_current_fps; }
    static float get_frame_time_ms() { return m_current_frametime; }
    static PerformanceMetrics get_performance_metrics();

    // Utilidades de registro en consola / depuración
    static void print_hardware_summary();
    static void print_performance_snapshot();
};