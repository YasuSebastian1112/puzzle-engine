#ifdef _WIN32

#include "os_windows.h"
#include <timeapi.h>
#include <psapi.h>

void OSWindows::trim_memory() {
    HANDLE process = GetCurrentProcess();
    EmptyWorkingSet(process);
    SetProcessWorkingSetSize(process, (SIZE_T)-1, (SIZE_T)-1);
}

void OSWindows::initialize_system_timer() {
    timeBeginPeriod(1);
}

void OSWindows::restore_system_timer() {
    timeEndPeriod(1);
}

// Puedes dejar install/remove vacíos o retirarlos sin alterar la API
void OSWindows::install_window_hooks(HWND, FramePumpCallback) {}
void OSWindows::remove_window_hooks(HWND) {}

#endif // _WIN32