// ========================================================================= //
// PUZZLE ENGINE - 2D/3D GAME ENGINE
// ------------------------------------------------------------------------- //
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// ========================================================================= //

#pragma once

#if defined(_WIN32)
#include <windows.h>
#include <functional>
#include <cstdint>

class OSWindows {
public:
    using FramePumpCallback = std::function<void()>;

    static void trim_memory();
    static void initialize_system_timer();
    static void restore_system_timer();

    static void install_window_hooks(HWND p_hwnd, FramePumpCallback p_callback);
    static void remove_window_hooks(HWND p_hwnd);
    static void enable_borderless_snap(HWND p_hwnd);

    static void initialize();
    static void cleanup();
    static void sleep_precise(uint32_t p_ms);

private:
    static LRESULT CALLBACK custom_wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

    static inline WNDPROC s_original_wndproc = nullptr;
    static inline FramePumpCallback s_frame_callback = nullptr;
    static inline bool s_initial_trim_done = false;
    static inline bool s_in_callback = false;
};
#endif