// ========================================================================= //
// PUZZLE ENGINE - 2D/3D GAME ENGINE
// ------------------------------------------------------------------------- //
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// ========================================================================= //

#include "command_dispatcher.h"
#include "classes/node.h"
#include "engine_console.h"
#include <iostream>

void CommandDispatcher::register_command(const std::string& name, CommandCallback callback) {
    commands[name] = callback;
}

std::vector<std::string> CommandDispatcher::tokenize(const std::string& line) const {
    std::vector<std::string> tokens;
    std::string current;
    bool in_quotes = false;

    for (char c : line) {
        if (c == '"') {
            in_quotes = !in_quotes;
        } else if (c == ' ' && !in_quotes) {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }

    if (!current.empty()) {
        tokens.push_back(current);
    }

    return tokens;
}

void CommandDispatcher::print_tree_recursive(const Node* node, const std::string& prefix, bool is_last) const {
    if (!node) return;

    std::cout << "\033[38;2;120;125;140m" << prefix << (is_last ? "└── " : "├── ");
    std::cout << "\033[38;2;74;222;40m" << node->get_name().c_str() 
              << " \033[38;2;90;95;115m[" << node->get_class_name() << "]\033[0m\n";

    const auto& children = node->get_children();
    for (size_t i = 0; i < children.size(); ++i) {
        bool last = (i == children.size() - 1);
        print_tree_recursive(children[i], prefix + (is_last ? "    " : "│   "), last);
    }
}

void CommandDispatcher::handle_node_command(const std::vector<std::string>& tokens) {
    if (!scene_root) {
        std::cout << "\033[38;2;245;70;70m[puzzle_cli] Error: No hay nodo raiz asignado en la escena.\033[0m\n";
        return;
    }

    std::string path = tokens[0].substr(1);
    Node* target = scene_root->get_node(path.c_str());

    if (!target) {
        std::cout << "\033[38;2;245;70;70m[puzzle_cli] Error: Nodo no encontrado en '" << path << "'.\033[0m\n";
        return;
    }

    if (tokens.size() == 1) {
        std::cout << "\033[38;2;74;222;40m[puzzle_cli] Nodo encontrado: " << target->get_name().c_str()
                  << " \033[38;2;90;95;115m[" << target->get_class_name() << "]\033[0m\n";
        return;
    }

    std::string prop = tokens[1];
    std::vector<std::string> prop_args(tokens.begin() + 2, tokens.end());

    if (prop_args.empty()) {
        std::string val = target->get(prop);
        if (!val.empty()) {
            std::cout << "\033[38;2;250;197;28m" << path << "." << prop << "\033[0m = \033[38;2;74;222;40m" << val << "\033[0m\n";
        } else {
            std::cout << "\033[38;2;245;70;70m[puzzle_cli] Propiedad '" << prop << "' no existe o no tiene lectura.\033[0m\n";
        }
        return;
    }

    if (target->set(prop, prop_args)) {
        std::cout << "\033[38;2;74;222;40m[puzzle_cli] OK: " << path << "." << prop << " actualizado.\033[0m\n";
    } else {
        std::cout << "\033[38;2;245;70;70m[puzzle_cli] Error al asignar propiedad '" << prop << "'.\033[0m\n";
    }
}

void CommandDispatcher::execute(const std::string& line) {
    if (line.empty()) return;

    std::vector<std::string> tokens = tokenize(line);
    if (tokens.empty()) return;

    // Manejo de comandos para nodos del árbol con '$'
    if (tokens[0][0] == '$') {
        handle_node_command(tokens);
        return;
    }

    // Comandos CLI con prefijo '/'
    if (tokens[0] == "/help") {
        std::cout << "\033[38;2;250;197;28mComandos del Sistema CLI:\033[0m\n"
                  << "  \033[38;2;74;222;40m/help\033[0m              - Muestra esta lista de comandos auxiliares.\n"
                  << "  \033[38;2;74;222;40m/version\033[0m           - Muestra la version actual de PuzzleEngine.\n"
                  << "  \033[38;2;74;222;40m/clear\033[0m             - Limpia la pantalla preservando el historial en memoria.\n"
                  << "  \033[38;2;74;222;40m/clearall\033[0m          - Limpia la pantalla y vacia todo el historial.\n"
                  << "  \033[38;2;74;222;40m/tree\033[0m              - Imprime el arbol jerarquico de nodos activos.\n"
                  << "  \033[38;2;74;222;40m/history\033[0m           - Muestra todos los comandos ejecutados.\n\n"
                  << "\033[38;2;250;197;28mInspeccion y Asignacion de Nodos ($):\033[0m\n"
                  << "  \033[38;2;40;105;218m$[Ruta/Al/Nodo]\033[0m                       - Valida si el nodo existe.\n"
                  << "  \033[38;2;40;105;218m$[Ruta/Al/Nodo] [propiedad]\033[0m           - Obtiene el valor (getter).\n"
                  << "  \033[38;2;40;105;218m$[Ruta/Al/Nodo] [propiedad] [valores...]\033[0m - Modifica la propiedad (setter).\n";
        return;
    }

    if (tokens[0] == "/version") {
        std::cout << "\033[38;2;245;130;32mPuzzleEngine v0.1.0-dev \033[38;2;90;95;115m(C++20 / OpenGL / WitchCraft ready)\033[0m\n";
        return;
    }

    if (tokens[0] == "/tree") {
        if (!scene_root) {
            std::cout << "\033[38;2;245;70;70m[puzzle_cli] No hay arbol de escena cargado.\033[0m\n";
            return;
        }
        std::cout << "\033[38;2;250;197;28mScene Tree:\033[0m\n";
        print_tree_recursive(scene_root, "", true);
        return;
    }

    if (tokens[0] == "/history") {
        if (console) {
            console->print_history();
        }
        return;
    }

    // Comandos directos registrados a medida
    auto it = commands.find(tokens[0]);
    if (it != commands.end()) {
        std::string cmd = tokens[0];
        tokens.erase(tokens.begin());
        it->second(tokens);
        return;
    }

    std::cout << "\033[38;2;245;70;70mComando desconocido: '" << tokens[0] << "'. Escribe /help para ver la lista.\033[0m\n";
}