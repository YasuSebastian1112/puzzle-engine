// ========================================================================= //
// PUZZLE ENGINE - 2D/3D GAME ENGINE
// ------------------------------------------------------------------------- //
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// ========================================================================= //

#ifdef _WIN32

#include "os_windows.h"
#include <timeapi.h>
#include <psapi.h>

void OSWindows::trim_memory() {
    HANDLE process = GetCurrentProcess();
    EmptyWorkingSet(process);
    SetProcessWorkingSetSize(process, static_cast<SIZE_T>(-1), static_cast<SIZE_T>(-1));
}

void OSWindows::initialize_system_timer() {
    timeBeginPeriod(1);
}

void OSWindows::restore_system_timer() {
    timeEndPeriod(1);
}

LRESULT CALLBACK OSWindows::custom_wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
        // En cuanto la ventana se vuelve visible y DWM asigna sus buffers de composición
        case WM_SHOWWINDOW: {
            LRESULT res = CallWindowProc(s_original_wndproc, hwnd, msg, wparam, lparam);
            if (wparam == TRUE && !s_initial_trim_done) {
                trim_memory();
                s_initial_trim_done = true;
            }
            return res;
        }

        // Se dispara al terminar de arrastrar, mover o redimensionar los bordes
        case WM_EXITSIZEMOVE: {
            LRESULT res = CallWindowProc(s_original_wndproc, hwnd, msg, wparam, lparam);
            trim_memory();
            return res;
        }

        // Se dispara al maximizar o restaurar con atajos de teclado (Win + Flechas) o botones
        case WM_SYSCOMMAND: {
            LRESULT res = CallWindowProc(s_original_wndproc, hwnd, msg, wparam, lparam);
            WPARAM cmd = wparam & 0xFFF0;
            if (cmd == SC_MAXIMIZE || cmd == SC_RESTORE) {
                trim_memory();
            }
            return res;
        }

        // Redibujado continuo y fluido durante el redimensionamiento modal de Windows
        case WM_SIZING:
        case WM_MOVING: {
            if (s_frame_callback) {
                s_frame_callback();
            }
            break;
        }
    }

    return CallWindowProc(s_original_wndproc, hwnd, msg, wparam, lparam);
}

void OSWindows::install_window_hooks(HWND p_hwnd, FramePumpCallback p_callback) {
    if (!p_hwnd) return;

    s_frame_callback = p_callback;
    s_initial_trim_done = false;

    // Asignar el icono compilado en el ejecutable a la ventana y barra de tareas
    HINSTANCE hInstance = GetModuleHandle(nullptr);
    HICON hIconBig = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(1), IMAGE_ICON, 
                                      GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), 
                                      LR_DEFAULTCOLOR);
    HICON hIconSmall = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(1), IMAGE_ICON, 
                                        GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 
                                        LR_DEFAULTCOLOR);

    if (hIconBig) {
        SendMessage(p_hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIconBig);
    }
    if (hIconSmall) {
        SendMessage(p_hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall);
    }

    // Subclasear la ventana de GLFW con nuestro procedimiento de control
    s_original_wndproc = reinterpret_cast<WNDPROC>(
        SetWindowLongPtr(p_hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(custom_wndproc))
    );
}

void OSWindows::remove_window_hooks(HWND p_hwnd) {
    if (p_hwnd && s_original_wndproc) {
        SetWindowLongPtr(p_hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(s_original_wndproc));
        s_original_wndproc = nullptr;
        s_frame_callback = nullptr;
    }
}

#endif // _WIN32