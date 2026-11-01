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

void OSWindows::initialize() {
    // Eleva la resolución del timer del sistema de ~15.6 ms a 1 ms
    timeBeginPeriod(1);
}

void OSWindows::cleanup() {
    // Restaura la resolución estándar del temporizador al salir
    timeEndPeriod(1);
}

void OSWindows::restore_system_timer() {
    timeEndPeriod(1);
}

void OSWindows::trim_memory() {
    HANDLE process = GetCurrentProcess();
    EmptyWorkingSet(process);
    SetProcessWorkingSetSize(process, static_cast<SIZE_T>(-1), static_cast<SIZE_T>(-1));
}

void OSWindows::sleep_precise(uint32_t p_ms) {
    if (p_ms > 0) {
        Sleep(static_cast<DWORD>(p_ms));
    }
}

void OSWindows::initialize_system_timer() {
    timeBeginPeriod(1);
}

LRESULT CALLBACK OSWindows::custom_wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    WNDPROC orig_proc = s_original_wndproc;
    if (!orig_proc) {
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }

    switch (msg) {
        // Elimina el marco estándar de Windows conservando las animaciones de DWM y Snap Assist
        case WM_NCCALCSIZE: {
            if (wparam == TRUE) {
                return 0;
            }
            break;
        }

        // En cuanto la ventana se vuelve visible y DWM asigna sus buffers de composición
        case WM_SHOWWINDOW: {
            LRESULT res = CallWindowProc(orig_proc, hwnd, msg, wparam, lparam);
            if (wparam == TRUE && !s_initial_trim_done) {
                trim_memory();
                s_initial_trim_done = true;
            }
            return res;
        }

        // Se dispara al terminar de arrastrar, mover o redimensionar los bordes
        case WM_EXITSIZEMOVE: {
            LRESULT res = CallWindowProc(orig_proc, hwnd, msg, wparam, lparam);
            trim_memory();
            return res;
        }

        // Se dispara al maximizar, minimizar o restaurar con atajos (Win + Flechas) o barra de tareas
        case WM_SYSCOMMAND: {
            LRESULT res = CallWindowProc(orig_proc, hwnd, msg, wparam, lparam);
            WPARAM cmd = wparam & 0xFFF0;
            if (cmd == SC_MAXIMIZE || cmd == SC_RESTORE || cmd == SC_MINIMIZE) {
                trim_memory();
            }
            return res;
        }

        // Redibujado fluido durante el redimensionamiento/movimiento modal nativo de Windows
        case WM_SIZING:
        case WM_MOVING: {
            LRESULT res = CallWindowProc(orig_proc, hwnd, msg, wparam, lparam);
            if (s_frame_callback && !s_in_callback && !IsIconic(hwnd)) {
                s_in_callback = true;
                s_frame_callback();
                s_in_callback = false;
            }
            return res;
        }
    }

    return CallWindowProc(orig_proc, hwnd, msg, wparam, lparam);
}

void OSWindows::enable_borderless_snap(HWND p_hwnd) {
    if (!p_hwnd) return;

    // Estilos requeridos para que DWM registre Snap Assist (Win + Flechas) y animaciones
    LONG_PTR style = GetWindowLongPtr(p_hwnd, GWL_STYLE);
    style |= (WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_POPUP);
    style &= ~WS_CAPTION;
    SetWindowLongPtr(p_hwnd, GWL_STYLE, style);

    // Forzar el recálculo total del marco sin alterar posición ni tamaño
    SetWindowPos(p_hwnd, nullptr, 0, 0, 0, 0,
                 SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

void OSWindows::install_window_hooks(HWND p_hwnd, FramePumpCallback p_callback) {
    if (!p_hwnd) return;

    s_frame_callback = p_callback;
    s_initial_trim_done = false;
    s_in_callback = false;

    // Asignar el icono compilado en el ejecutable a la ventana y barra de tareas
    HINSTANCE hInstance = GetModuleHandle(nullptr);
    HICON hIconBig = static_cast<HICON>(LoadImage(
        hInstance, MAKEINTRESOURCE(1), IMAGE_ICON,
        GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON),
        LR_DEFAULTCOLOR
    ));
    HICON hIconSmall = static_cast<HICON>(LoadImage(
        hInstance, MAKEINTRESOURCE(1), IMAGE_ICON,
        GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),
        LR_DEFAULTCOLOR
    ));

    if (hIconBig) {
        SendMessage(p_hwnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(hIconBig));
    }
    if (hIconSmall) {
        SendMessage(p_hwnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(hIconSmall));
    }

    // Subclasear la ventana de GLFW una única vez (evita sobrescribir s_original_wndproc con custom_wndproc)
    WNDPROC current_proc = reinterpret_cast<WNDPROC>(GetWindowLongPtr(p_hwnd, GWLP_WNDPROC));
    if (current_proc != custom_wndproc) {
        s_original_wndproc = reinterpret_cast<WNDPROC>(
            SetWindowLongPtr(p_hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(custom_wndproc))
        );
    }
}

void OSWindows::remove_window_hooks(HWND p_hwnd) {
    if (p_hwnd && s_original_wndproc) {
        SetWindowLongPtr(p_hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(s_original_wndproc));
        s_original_wndproc = nullptr;
        s_frame_callback = nullptr;
        s_in_callback = false;
    }
}

#endif // _WIN32