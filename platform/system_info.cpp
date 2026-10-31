// ========================================================================= //
// PUZZLE ENGINE - 2D/3D GAME ENGINE
// ------------------------------------------------------------------------- //
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// ========================================================================= //

#include "system_info.h"
#include <iostream>
#include <cstdio>
#include <cstring>
#include <glad/glad.h>

#if defined(_WIN32)
#include <windows.h>
#include <psapi.h>
#include <intrin.h>
#pragma comment(lib, "psapi.lib")
#else
#include <unistd.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#endif

// Convierte FILETIME nativo de Win32 a un entero sin signo de 64 bits en unidades de 100 ns
#if defined(_WIN32)
static inline uint64_t filetime_to_uint64(const FILETIME& ft) {
    return (static_cast<uint64_t>(ft.dwHighDateTime) << 32) | static_cast<uint64_t>(ft.dwLowDateTime);
}
#endif

void SystemInfo::initialize() {
#if defined(_WIN32)
    SYSTEM_INFO sys_info;
    GetSystemInfo(&sys_info);
    m_num_processors = sys_info.dwNumberOfProcessors;

    FILETIME ftime, fsys, fuser;
    GetSystemTimeAsFileTime(&ftime);
    m_last_system_time = filetime_to_uint64(ftime);

    GetProcessTimes(GetCurrentProcess(), &ftime, &ftime, &fsys, &fuser);
    m_last_process_time = filetime_to_uint64(fsys) + filetime_to_uint64(fuser);
#else
    m_num_processors = sysconf(_SC_NPROCESSORS_ONLN);
#endif
    m_fps_timer = 0.0;
    m_frame_counter = 0;
}

void SystemInfo::update(double p_delta) {
    // 1. Tasa de fotogramas y tiempo de cuadro
    m_current_frametime = static_cast<float>(p_delta * 1000.0);
    m_frame_counter++;
    m_fps_timer += p_delta;

    if (m_fps_timer >= 0.5) { // Promedio cada 500 ms para estabilidad de lectura
        m_current_fps = static_cast<float>(m_frame_counter / m_fps_timer);
        m_frame_counter = 0;
        m_fps_timer = 0.0;
    }

    // 2. Muestreo de uso de CPU del proceso
#if defined(_WIN32)
    FILETIME now_sys_ft, creation_ft, exit_ft, kernel_ft, user_ft;
    GetSystemTimeAsFileTime(&now_sys_ft);

    if (GetProcessTimes(GetCurrentProcess(), &creation_ft, &exit_ft, &kernel_ft, &user_ft)) {
        uint64_t now_sys = filetime_to_uint64(now_sys_ft);
        uint64_t now_proc = filetime_to_uint64(kernel_ft) + filetime_to_uint64(user_ft);

        uint64_t sys_diff = now_sys - m_last_system_time;
        uint64_t proc_diff = now_proc - m_last_process_time;

        if (sys_diff > 0 && m_num_processors > 0) {
            float pct = (static_cast<float>(proc_diff) / static_cast<float>(sys_diff)) * 100.0f;
            // Normalizar entre hilos lógicos
            m_current_cpu_pct = pct / static_cast<float>(m_num_processors);
        }

        m_last_system_time = now_sys;
        m_last_process_time = now_proc;
    }
#endif
}

String SystemInfo::get_os_name() {
#if defined(_WIN32)
    return "Windows (Win32 / DWM x64)";
#elif defined(__linux__)
    return "GNU/Linux";
#elif defined(__APPLE__)
    return "macOS";
#else
    return "Unknown OS";
#endif
}

String SystemInfo::get_cpu_model() {
#if defined(_WIN32)
    int cpu_info[4] = { -1 };
    char cpu_brand[0x40] = { 0 };

    __cpuid(cpu_info, 0x80000000);
    unsigned int nExIds = cpu_info[0];

    if (nExIds >= 0x80000004) {
        __cpuid(reinterpret_cast<int*>(cpu_brand), 0x80000002);
        __cpuid(reinterpret_cast<int*>(cpu_brand + 16), 0x80000003);
        __cpuid(reinterpret_cast<int*>(cpu_brand + 32), 0x80000004);
        return String(cpu_brand);
    }
    return "Generic x86_64 Processor";
#else
    return "Generic CPU";
#endif
}

uint32_t SystemInfo::get_cpu_logical_core_count() {
#if defined(_WIN32)
    SYSTEM_INFO sys_info;
    GetSystemInfo(&sys_info);
    return static_cast<uint32_t>(sys_info.dwNumberOfProcessors);
#else
    return static_cast<uint32_t>(sysconf(_SC_NPROCESSORS_ONLN));
#endif
}

String SystemInfo::get_gpu_vendor() {
    const GLubyte* vendor = glGetString(GL_VENDOR);
    return vendor ? String(reinterpret_cast<const char*>(vendor)) : "Unknown Vendor";
}

String SystemInfo::get_gpu_renderer() {
    const GLubyte* renderer = glGetString(GL_RENDERER);
    return renderer ? String(reinterpret_cast<const char*>(renderer)) : "Unknown Device";
}

String SystemInfo::get_gpu_driver_version() {
    const GLubyte* version = glGetString(GL_VERSION);
    return version ? String(reinterpret_cast<const char*>(version)) : "Unknown Version";
}

SystemInfo::MemoryStats SystemInfo::get_memory_stats() {
    MemoryStats stats = {0, 0, 0, 0};

#if defined(_WIN32)
    HANDLE process = GetCurrentProcess();

    // Obtener Commit Size / Memoria virtual asignada
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(process, reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), sizeof(pmc))) {
        stats.process_virtual_used_bytes = pmc.PrivateUsage;
    }

    // Calcular el Working Set Privado real (idéntico a la columna Memoria del Task Manager)
    // Se recorren las páginas asignadas del proceso consultando sus atributos
    size_t private_working_set = 0;
    PSAPI_WORKING_SET_INFORMATION* ws_info = nullptr;
    ULONG_PTR entries = 4096;

    while (true) {
        size_t buffer_size = sizeof(PSAPI_WORKING_SET_INFORMATION) + (entries * sizeof(PSAPI_WORKING_SET_BLOCK));
        ws_info = static_cast<PSAPI_WORKING_SET_INFORMATION*>(std::malloc(buffer_size));
        if (!ws_info) break;

        if (QueryWorkingSet(process, ws_info, buffer_size)) {
            for (ULONG_PTR i = 0; i < ws_info->NumberOfEntries; ++i) {
                // Si la página es válida y no es compartida (Shared == 0), pertenece exclusivamente al proceso
                if (ws_info->WorkingSetInfo[i].Shared == 0) {
                    private_working_set += 4096; // Tamaño de página estándar (4 KB)
                }
            }
            std::free(ws_info);
            break;
        }

        std::free(ws_info);
        if (GetLastError() == ERROR_BAD_LENGTH) {
            entries *= 2; // Incrementar el búfer si el proceso creció
        } else {
            break;
        }
    }

    // Si fallara la consulta detallada, se toma el PrivateUsage como respaldo
    stats.process_physical_used_bytes = (private_working_set > 0) ? private_working_set : stats.process_virtual_used_bytes;

    // Memoria total y libre del sistema operativo
    MEMORYSTATUSEX mem_status;
    mem_status.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&mem_status)) {
        stats.system_total_ram_bytes = mem_status.ullTotalPhys;
        stats.system_free_ram_bytes  = mem_status.ullAvailPhys;
    }
#endif

    return stats;
}

float SystemInfo::get_process_memory_mb() {
    MemoryStats stats = get_memory_stats();
    // Conversión estricta a MB (1024 * 1024 = 1048576 bytes)
    return static_cast<float>(stats.process_physical_used_bytes) / 1048576.0f;
}

float SystemInfo::get_cpu_usage_percentage() {
    return m_current_cpu_pct;
}

SystemInfo::PerformanceMetrics SystemInfo::get_performance_metrics() {
    PerformanceMetrics pm;
    pm.fps = m_current_fps;
    pm.frame_time_ms = m_current_frametime;
    pm.cpu_process_usage_pct = m_current_cpu_pct;
    pm.memory_used_mb = get_process_memory_mb();
    return pm;
}

void SystemInfo::print_hardware_summary() {
    MemoryStats mem = get_memory_stats();
    float total_ram_gb = static_cast<float>(mem.system_total_ram_bytes) / (1024.0f * 1024.0f * 1024.0f);
    float free_ram_gb = static_cast<float>(mem.system_free_ram_bytes) / (1024.0f * 1024.0f * 1024.0f);

    std::cout << "=========================================================================\n";
    std::cout << "                 PUZZLE ENGINE - HARDWARE ENVIRONMENT\n";
    std::cout << "-------------------------------------------------------------------------\n";
    std::cout << "  Sistema Operativo:  " << get_os_name().c_str() << "\n";
    std::cout << "  Procesador (CPU):   " << get_cpu_model().c_str() << " [" << get_cpu_logical_core_count() << " hilos]\n";
    std::cout << "  GPU Vendor:         " << get_gpu_vendor().c_str() << "\n";
    std::cout << "  GPU Dispositivo:    " << get_gpu_renderer().c_str() << "\n";
    std::cout << "  Driver / OpenGL:    " << get_gpu_driver_version().c_str() << "\n";
    std::printf("  Memoria del Sistema: %.2f GB libres de %.2f GB totales\n", free_ram_gb, total_ram_gb);
    std::cout << "=========================================================================\n" << std::endl;
}

void SystemInfo::print_performance_snapshot() {
    PerformanceMetrics pm = get_performance_metrics();
    std::printf("[PERF] FPS: %6.1f | Frame: %5.2f ms | RAM: %5.2f MB | CPU Proc: %4.1f%%\n",
                pm.fps, pm.frame_time_ms, pm.memory_used_mb, pm.cpu_process_usage_pct);
}

void SystemInfo::trim_memory() {
#if defined(_WIN32)
    // Pasar -1 (o (SIZE_T)-1) indica al kernel de Windows que vacíe tantas páginas del
    // working set como sea posible hacia el estado disponible sin penalizar el rendimiento.
    SetProcessWorkingSetSize(GetCurrentProcess(), static_cast<SIZE_T>(-1), static_cast<SIZE_T>(-1));
#endif
}