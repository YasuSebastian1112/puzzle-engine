// ========================================================================= //
// PUZZLE ENGINE - 2D/3D GAME ENGINE
// ------------------------------------------------------------------------- //
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// ========================================================================= //

#pragma once

#include "object.h"
#include <vector>

class SceneTree;

class Node : public Object {
    CLASS_TYPE(Node, Object);

public:
    // Ciclo de vida
    SceneTree* get_tree() const;
    
    virtual void _enter_tree() {}
    virtual void _ready() {}
    virtual void _process(float delta) {}
    virtual void _exit_tree() {}

    // Gestión del árbol
    void add_child(Node* child);
    void remove_child(Node* child);
    Node* get_parent() const { return parent; }
    const std::vector<Node*>& get_children() const { return children; }
    size_t get_child_count() const { return children.size(); }
    Node* get_child(size_t index) const;

    // Navegación de rutas
    Node* find_child(const String& child_name) const;
    Node* get_node(const String& path) const;

    // Gestión del nodo
    const String& get_name() const { return name; }
    void set_name(const String& new_name) { name = new_name; }
    String get_path() const;

    // Ejecución interna
    void propagate_ready();
    void propagate_process(float delta);

    Node(const String& node_name = "Node");
    virtual ~Node() override;

private:
    String name;
    Node* parent = nullptr;
    std::vector<Node*> children;
    bool is_ready = false;

};