// ========================================================================= //
// Sierra Engine - Platform Abstraction (Windows)
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// ========================================================================= //

#pragma once

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <mmsystem.h>
#include <functional>

class OSWindows {
public:
    using FramePumpCallback = std::function<void()>;

    static void trim_memory();
    static void install_window_hooks(HWND p_hwnd, FramePumpCallback p_pump_callback);
    static void remove_window_hooks(HWND p_hwnd);
    static void initialize_system_timer();
    static void restore_system_timer();

private:
    static LRESULT CALLBACK subclass_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
};

#endif // _WIN32