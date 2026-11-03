// ========================================================================= //
// PUZZLE ENGINE - 2D/3D GAME ENGINE
// ------------------------------------------------------------------------- //
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// ========================================================================= //

#include "engine_console.h"
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

// Secuencias de Color TrueColor basadas en el logotipo de la 'P'
#define ANSI_RESET      "\033[0m"
#define ANSI_GRAY       "\033[38;2;110;115;135m"
#define ANSI_DARK_GRAY  "\033[38;2;70;75;90m"
#define ANSI_YELLOW     "\033[38;2;250;197;28m"
#define ANSI_ORANGE     "\033[38;2;245;130;32m"
#define ANSI_GREEN      "\033[38;2;74;222;40m"
#define ANSI_BLUE       "\033[38;2;40;105;218m"
#define ANSI_PURPLE     "\033[38;2;130;53;245m"
#define ANSI_WHITE      "\033[38;2;230;235;245m"

EngineConsole::EngineConsole() {
    dispatcher.set_console(this);

#ifdef _WIN32
    // 1. Establecer la página de códigos de entrada y salida a UTF-8 (65001)
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    // 2. Habilitar el procesamiento de secuencias de escape ANSI / Virtual Terminal
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }
#endif
}

EngineConsole::~EngineConsole() {
    stop();
}

void EngineConsole::print_logo() {
    std::cout << "\n";
    std::cout << ANSI_DARK_GRAY << "  ┌─────────────────────────────────────────────────────────────┐" << ANSI_RESET << "\n";
    std::cout << ANSI_DARK_GRAY << "  │  " 
              << ANSI_YELLOW << "██████╗ " 
              << ANSI_ORANGE << "██╗   ██╗" 
              << ANSI_GREEN  << "███████╗" 
              << ANSI_BLUE   << "███████╗" 
              << ANSI_PURPLE << "██╗     " 
              << ANSI_YELLOW << "███████╗" 
              << ANSI_DARK_GRAY << "  │" << ANSI_RESET << "\n";

    std::cout << ANSI_DARK_GRAY << "  │  " 
              << ANSI_YELLOW << "██╔══██╗" 
              << ANSI_ORANGE << "██║   ██║" 
              << ANSI_GREEN  << "╚══███╔╝" 
              << ANSI_BLUE   << "╚══███╔╝" 
              << ANSI_PURPLE << "██║     " 
              << ANSI_YELLOW << "██╔════╝" 
              << ANSI_DARK_GRAY << "  │" << ANSI_RESET << "\n";

    std::cout << ANSI_DARK_GRAY << "  │  " 
              << ANSI_YELLOW << "██████╔╝" 
              << ANSI_ORANGE << "██║   ██║" 
              << ANSI_GREEN  << "  ███╔╝ " 
              << ANSI_BLUE   << "  ███╔╝ " 
              << ANSI_PURPLE << "██║     " 
              << ANSI_YELLOW << "█████╗  " 
              << ANSI_DARK_GRAY << "  │" << ANSI_RESET << "\n";

    std::cout << ANSI_DARK_GRAY << "  │  " 
              << ANSI_YELLOW << "██╔═══╝ " 
              << ANSI_ORANGE << "██║   ██║" 
              << ANSI_GREEN  << " ███╔╝  " 
              << ANSI_BLUE   << " ███╔╝  " 
              << ANSI_PURPLE << "██║     " 
              << ANSI_YELLOW << "██╔══╝  " 
              << ANSI_DARK_GRAY << "  │" << ANSI_RESET << "\n";

    std::cout << ANSI_DARK_GRAY << "  │  " 
              << ANSI_YELLOW << "██║     " 
              << ANSI_ORANGE << "╚██████╔╝" 
              << ANSI_GREEN  << "███████╗" 
              << ANSI_BLUE   << "███████╗" 
              << ANSI_PURPLE << "███████╗" 
              << ANSI_YELLOW << "███████╗" 
              << ANSI_DARK_GRAY << "  │" << ANSI_RESET << "\n";

    std::cout << ANSI_DARK_GRAY << "  │  " 
              << ANSI_GRAY   << "╚═╝      ╚═════╝ ╚══════╝╚══════╝╚══════╝╚══════╝" 
              << ANSI_DARK_GRAY << "  │" << ANSI_RESET << "\n";
    std::cout << ANSI_DARK_GRAY << "  └─────────────────────────────────────────────────────────────┘" << ANSI_RESET << "\n\n";

    std::cout << ANSI_GRAY << "  Tips para iniciar:\n"
              << "  1. Modifica nodos en caliente con: " << ANSI_YELLOW << "$[Ruta/Nodo] [propiedad] [valor]" << ANSI_GRAY << "\n"
              << "  2. Escribe " << ANSI_GREEN << "/tree" << ANSI_GRAY << " para inspeccionar la jerarquia de la escena activa.\n"
              << "  3. Escribe " << ANSI_GREEN << "/help" << ANSI_GRAY << " para ver comandos auxiliares del CLI.\n\n" << ANSI_RESET;
}

void EngineConsole::print_prompt_box() {
    std::cout << ANSI_DARK_GRAY << "  ┌─────────────────────────────────────────────────────────────┐\n"
              << "  │ " << ANSI_GREEN << "> " << ANSI_WHITE;
    std::cout.flush();
}

void EngineConsole::render_ui() {
    // Código ANSI para limpiar pantalla y ubicar cursor en (0,0)
    std::cout << "\033[2J\033[H";
    print_logo();

    // Mostrar el historial de comandos recientes
    if (!command_history.empty()) {
        std::cout << ANSI_GRAY << "  Historial reciente:\n";
        size_t start = (command_history.size() > 5) ? (command_history.size() - 5) : 0;
        for (size_t i = start; i < command_history.size(); ++i) {
            std::cout << ANSI_DARK_GRAY << "    " << (i + 1) << ". " << ANSI_YELLOW << command_history[i] << ANSI_RESET << "\n";
        }
        std::cout << "\n";
    }

    print_prompt_box();
}

void EngineConsole::print_history() const {
    std::cout << ANSI_YELLOW << "Historial completo de comandos:\n" << ANSI_RESET;
    for (size_t i = 0; i < command_history.size(); ++i) {
        std::cout << ANSI_GRAY << "  [" << (i + 1) << "] " << ANSI_WHITE << command_history[i] << ANSI_RESET << "\n";
    }
}

void EngineConsole::clear_history() {
    command_history.clear();
}

void EngineConsole::start() {
    if (is_running) return;

    is_running = true;
    render_ui();

    input_thread = std::thread(&EngineConsole::read_input_loop, this);
    input_thread.detach();
}

void EngineConsole::stop() {
    is_running = false;
}

CommandDispatcher& EngineConsole::get_dispatcher() {
    return dispatcher;
}

void EngineConsole::read_input_loop() {
    std::string line;
    while (is_running) {
        if (std::getline(std::cin, line)) {
            if (!line.empty()) {
                std::lock_guard<std::mutex> lock(queue_mutex);
                pending_commands.push(line);
            } else {
                print_prompt_box();
            }
        }
    }
}

void EngineConsole::process_commands() {
    std::queue<std::string> local_queue;
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        std::swap(local_queue, pending_commands);
    }

    while (!local_queue.empty()) {
        std::string cmd = local_queue.front();
        local_queue.pop();

        // Cerrar caja visual del prompt actual
        std::cout << ANSI_DARK_GRAY << "│\n"
                  << "  └─────────────────────────────────────────────────────────────┘\n" << ANSI_RESET;

        if (cmd == "/clear") {
            command_history.push_back(cmd);
            render_ui();
            continue;
        }

        if (cmd == "/clearall") {
            clear_history();
            render_ui();
            continue;
        }

        command_history.push_back(cmd);

        // Ejecutar comando a través del dispatcher
        std::cout << ANSI_GRAY << "[cli] >> " << ANSI_WHITE << cmd << ANSI_RESET << "\n";
        dispatcher.execute(cmd);
        std::cout << "\n";

        // Presentar la siguiente caja de entrada
        print_prompt_box();
    }
}