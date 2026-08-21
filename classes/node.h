// ========================================================================= //
// Puzzle Engine - 2D/3D Hybrid Game Engine
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// ========================================================================= //

#pragma once
#include "object.h"


class Node : public Object {
    CLASS_TYPE(Node, Object);

public:

    // Manejan el ciclo de vida al igual que godot
    virtual void _enter_tree() {}
    virtual void _ready() {}
    virtual void _process(float delta) {}
    virtual void _exit_tree() {}

    // Gestion del arbol
    void add_child(Node* child);
    void remove_child(Node* child);
    Node* get_parent() const {return parent;};
    const std::vector<Node*>& get_children() const { return children; }
    size_t get_child_count() const { return children.size(); }
    Node* get_child(size_t index) const;

    // Gestion del nodo
    const std::string& get_name() const {return name;}
    void set_name(const std::string& new_name) {name = new_name;}
    std::string get_path() const;

    // Ejecucion Interna
    void propagate_ready();
    void propagate_process(float delta);


    Node(const std::string& node_name = "Node");
    virtual ~Node() override;

private:

    // Datos del nodo
    std::string name;
    Node* parent = nullptr;
    std::vector<Node*> children;
    bool is_ready = false;
};