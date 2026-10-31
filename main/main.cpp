// ========================================================================= //
// PUZZLE ENGINE - 2D/3D GAME ENGINE
// ------------------------------------------------------------------------- //
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// ========================================================================= //

#include "globals/rendering_global/rendering_global.h"
#include "globals/rendering_global/device/rendering_geometry.h"
#include "globals/input/input.h"
#include "platform/system_info.h"
#include "core/threading/job_system.h"
#include <cstdio>

int main() {
    SystemInfo::initialize();

    JobSystem::get().initialize();
    RenderingGlobal& renderer = RenderingGlobal::get();

    const int initial_w = 900;
    const int initial_h = 600;

    auto primary_window = renderer.create_window(
        Size2i(initial_w, initial_h),
        "Puzzle Engine - System Diagnostics & Hardware Monitor",
        Point2i(80, 80),
        Window::Type::Game
    );

    if (!primary_window) {
        return -1;
    }

    primary_window->get_rendering_device().init_geometry_pipeline();
    RenderingGeometry geometry(&primary_window->get_rendering_device());

    SystemInfo::print_hardware_summary();

    // Consultas fijas fuera del bucle para no generar cadenas dinámicas en el heap
    const String os_name     = SystemInfo::get_os_name();
    const String cpu_name    = SystemInfo::get_cpu_model();
    const String gpu_vendor  = SystemInfo::get_gpu_vendor();
    const String gpu_device  = SystemInfo::get_gpu_renderer();
    const String opengl_ver  = SystemInfo::get_gpu_driver_version();
    const uint32_t cpu_cores = SystemInfo::get_cpu_logical_core_count();

    primary_window->run([&](double p_delta) {
        SystemInfo::update(p_delta);
        const auto perf = SystemInfo::get_performance_metrics();
        const auto mem  = SystemInfo::get_memory_stats();

        Size2i cur_size = primary_window->get_size();

        primary_window->clear(Color(0.04f, 0.04f, 0.06f, 1.0f));
        geometry.begin();

        // ---------------------------------------------------------------------
        // 1. PANEL SUPERIOR: ESPECIFICACIONES DE HARDWARE
        // ---------------------------------------------------------------------
        Rect2 hw_panel(20.0f, 44.0f, static_cast<float>(cur_size.width) - 40.0f, 175.0f);
        geometry.draw_rect(hw_panel, Color(0.08f, 0.08f, 0.12f, 0.95f), true);
        geometry.draw_rect(hw_panel, Color(0.18f, 0.22f, 0.32f, 1.0f), false, 1.0f);

        geometry.draw_text(Vector2(35.0f, 54.0f), "HARDWARE ENVIRONMENT", Color(0.25f, 0.85f, 1.0f), 1.6f);

        char buf[256];
        std::snprintf(buf, sizeof(buf), "OS       : %s", os_name.c_str());
        geometry.draw_text(Vector2(35.0f, 80.0f), buf, Color(0.85f, 0.85f, 0.90f), 1.3f);

        std::snprintf(buf, sizeof(buf), "CPU      : %s (%u LOGICAL THREADS)", cpu_name.c_str(), cpu_cores);
        geometry.draw_text(Vector2(35.0f, 102.0f), buf, Color(0.85f, 0.85f, 0.90f), 1.3f);

        std::snprintf(buf, sizeof(buf), "GPU VEND : %s", gpu_vendor.c_str());
        geometry.draw_text(Vector2(35.0f, 124.0f), buf, Color(0.85f, 0.85f, 0.90f), 1.3f);

        std::snprintf(buf, sizeof(buf), "GPU DEV  : %s", gpu_device.c_str());
        geometry.draw_text(Vector2(35.0f, 146.0f), buf, Color(0.85f, 0.85f, 0.90f), 1.3f);

        std::snprintf(buf, sizeof(buf), "OPENGL   : %s", opengl_ver.c_str());
        geometry.draw_text(Vector2(35.0f, 168.0f), buf, Color(0.85f, 0.85f, 0.90f), 1.3f);

        float total_ram_gb = static_cast<float>(mem.system_total_ram_bytes) / 1073741824.0f;
        float free_ram_gb  = static_cast<float>(mem.system_free_ram_bytes)  / 1073741824.0f;
        std::snprintf(buf, sizeof(buf), "SYS RAM  : %.2f GB FREE / %.2f GB TOTAL", free_ram_gb, total_ram_gb);
        geometry.draw_text(Vector2(35.0f, 190.0f), buf, Color(0.85f, 0.85f, 0.90f), 1.3f);

        // ---------------------------------------------------------------------
        // 2. TARJETAS DE MONITOREO DE RENDIMIENTO EN VIVO (HUD)
        // ---------------------------------------------------------------------
        float card_y = 230.0f;
        float card_gap = 14.0f;
        float card_width = (static_cast<float>(cur_size.width) - 40.0f - (card_gap * 3.0f)) / 4.0f;
        float card_height = 110.0f;

        // Tarjeta 1: Framerate (FPS)
        Rect2 card_fps(20.0f, card_y, card_width, card_height);
        geometry.draw_rect(card_fps, Color(0.08f, 0.10f, 0.14f, 0.95f), true);
        geometry.draw_rect(card_fps, Color(0.20f, 0.80f, 0.40f, 0.8f), false, 1.0f);
        geometry.draw_text(Vector2(card_fps.position.x + 12.0f, card_y + 12.0f), "FRAMERATE", Color(0.6f, 0.7f, 0.8f), 1.2f);
        std::snprintf(buf, sizeof(buf), "%.0f", perf.fps);
        geometry.draw_text(Vector2(card_fps.position.x + 12.0f, card_y + 36.0f), buf, Color(0.25f, 1.0f, 0.45f), 3.2f);
        geometry.draw_text(Vector2(card_fps.position.x + 12.0f, card_y + 82.0f), "FRAMES / SEC", Color(0.4f, 0.5f, 0.6f), 1.0f);

        // Tarjeta 2: Tiempo de Cuadro (Latencia en ms)
        Rect2 card_ms(card_fps.position.x + card_width + card_gap, card_y, card_width, card_height);
        geometry.draw_rect(card_ms, Color(0.08f, 0.10f, 0.14f, 0.95f), true);
        geometry.draw_rect(card_ms, Color(0.20f, 0.65f, 1.00f, 0.8f), false, 1.0f);
        geometry.draw_text(Vector2(card_ms.position.x + 12.0f, card_y + 12.0f), "FRAME TIME", Color(0.6f, 0.7f, 0.8f), 1.2f);
        std::snprintf(buf, sizeof(buf), "%.2f", perf.frame_time_ms);
        geometry.draw_text(Vector2(card_ms.position.x + 12.0f, card_y + 36.0f), buf, Color(0.20f, 0.75f, 1.0f), 3.2f);
        geometry.draw_text(Vector2(card_ms.position.x + 12.0f, card_y + 82.0f), "MILLISECONDS", Color(0.4f, 0.5f, 0.6f), 1.0f);

        // Tarjeta 3: RAM del Motor (Working Set Privado)
        Rect2 card_ram(card_ms.position.x + card_width + card_gap, card_y, card_width, card_height);
        geometry.draw_rect(card_ram, Color(0.08f, 0.10f, 0.14f, 0.95f), true);
        geometry.draw_rect(card_ram, Color(0.95f, 0.75f, 0.20f, 0.8f), false, 1.0f);
        geometry.draw_text(Vector2(card_ram.position.x + 12.0f, card_y + 12.0f), "ENGINE RAM", Color(0.6f, 0.7f, 0.8f), 1.2f);
        std::snprintf(buf, sizeof(buf), "%.1f", perf.memory_used_mb);
        geometry.draw_text(Vector2(card_ram.position.x + 12.0f, card_y + 36.0f), buf, Color(1.0f, 0.85f, 0.25f), 2.6f);

        // Tarjeta 4: Carga de CPU del proceso
        Rect2 card_cpu(card_ram.position.x + card_width + card_gap, card_y, card_width, card_height);
        geometry.draw_rect(card_cpu, Color(0.08f, 0.10f, 0.14f, 0.95f), true);
        Color cpu_col = (perf.cpu_process_usage_pct > 15.0f) ? Color(1.0f, 0.3f, 0.3f) : Color(0.9f, 0.35f, 1.0f);
        geometry.draw_rect(card_cpu, cpu_col, false, 1.0f);
        geometry.draw_text(Vector2(card_cpu.position.x + 12.0f, card_y + 12.0f), "CPU USAGE", Color(0.6f, 0.7f, 0.8f), 1.2f);
        std::snprintf(buf, sizeof(buf), "%.1f%%", perf.cpu_process_usage_pct);
        geometry.draw_text(Vector2(card_cpu.position.x + 12.0f, card_y + 36.0f), buf, cpu_col, 3.2f);
        geometry.draw_text(Vector2(card_cpu.position.x + 12.0f, card_y + 82.0f), "NORMALIZED CORES", Color(0.4f, 0.5f, 0.6f), 1.0f);

        // ---------------------------------------------------------------------
        // 3. BARRA DE TELEMETRÍA EN TIEMPO REAL
        // ---------------------------------------------------------------------
        float status_y = card_y + card_height + 16.0f;
        Rect2 status_panel(20.0f, status_y, static_cast<float>(cur_size.width) - 40.0f, 160.0f);
        geometry.draw_rect(status_panel, Color(0.06f, 0.06f, 0.09f, 0.95f), true);
        geometry.draw_rect(status_panel, Color(0.14f, 0.16f, 0.22f, 1.0f), false, 1.0f);

        geometry.draw_text(Vector2(35.0f, status_y + 14.0f), "RUNTIME TELEMETRY", Color(0.5f, 0.6f, 0.75f), 1.4f);

        size_t v_count = geometry.get_vertices().size();
        size_t i_count = geometry.get_indices().size();
        size_t workers = JobSystem::get().get_worker_count();

        std::snprintf(buf, sizeof(buf), "> BATCH BUFFER : %zu VERTICES | %zu INDICES IN QUEUE", v_count, i_count);
        geometry.draw_text(Vector2(35.0f, status_y + 42.0f), buf, Color(0.40f, 0.90f, 0.60f), 1.3f);

        std::snprintf(buf, sizeof(buf), "> THREAD POOL  : %zu WORKERS ACTIVE IN BSS BARRIER", workers);
        geometry.draw_text(Vector2(35.0f, status_y + 66.0f), buf, Color(0.40f, 0.90f, 0.60f), 1.3f);

        float virt_mb = static_cast<float>(mem.process_virtual_used_bytes) / 1048576.0f;
        std::snprintf(buf, sizeof(buf), "> MEMORY SPACE : WORKING SET = %.1f MB | COMMIT SIZE = %.1f MB", 
            perf.memory_used_mb, virt_mb);
        geometry.draw_text(Vector2(35.0f, status_y + 90.0f), buf, Color(0.40f, 0.90f, 0.60f), 1.3f);

        std::snprintf(buf, sizeof(buf), "> WINDOW STATS : %dx%d RESOLUTION | VSYNC LOCKED (1:1)", cur_size.width, cur_size.height);
        geometry.draw_text(Vector2(35.0f, status_y + 114.0f), buf, Color(0.40f, 0.90f, 0.60f), 1.3f);

        // ---------------------------------------------------------------------
        // 4. BARRA DE TÍTULO
        // ---------------------------------------------------------------------
        primary_window->draw_title_bar(geometry);

        geometry.flush();
    });

    JobSystem::get().shutdown();
    renderer.shutdown();
    return 0;
}