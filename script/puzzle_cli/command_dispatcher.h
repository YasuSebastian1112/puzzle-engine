// ========================================================================= //
// PUZZLE ENGINE - 2D/3D GAME ENGINE
// ------------------------------------------------------------------------- //
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// ========================================================================= //

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

class Node;
class EngineConsole;

class CommandDispatcher {
public:
    using CommandCallback = std::function<void(const std::vector<std::string>&)>;

    CommandDispatcher() = default;
    ~CommandDispatcher() = default;

    void set_scene_root(Node* p_root) { scene_root = p_root; }
    Node* get_scene_root() const { return scene_root; }
    void set_console(EngineConsole* p_console) { console = p_console; }

    void register_command(const std::string& name, CommandCallback callback);
    void execute(const std::string& line);

private:
    Node* scene_root = nullptr;
    EngineConsole* console = nullptr;
    std::unordered_map<std::string, CommandCallback> commands;

    std::vector<std::string> tokenize(const std::string& line) const;
    void handle_node_command(const std::vector<std::string>& tokens);
    void print_tree_recursive(const Node* node, const std::string& prefix, bool is_last) const;
};