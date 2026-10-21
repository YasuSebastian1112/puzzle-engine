// ========================================================================= //
// Puzzle Engine - 2D/3D Hybrid Game Engine
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// ========================================================================= //

#include "node.h"
#include "core/scene_tree.h"
#include <algorithm>


// -----------------------------------------------------------------------------
// GESTIÓN DEL ÁRBOL
// -----------------------------------------------------------------------------
void Node::add_child(Node* child) {
    if (!child || child == this) return;

    // Si ya tenía otro padre, quitarlo de allí primero
    if (child->parent) {
        child->parent->remove_child(child);
    }

    child->parent = this;
    children.push_back(child);

    // Notificar entrada al árbol
    child->_enter_tree();

    // Si el árbol ya está en ejecución, propagar ready al nuevo nodo
    if (is_ready && !child->is_ready) {
        child->propagate_ready();
    }
}

void Node::remove_child(Node* child) {
    if (!child) return;

    auto it = std::find(children.begin(), children.end(), child);
    if (it != children.end()) {
        child->_exit_tree();
        child->parent = nullptr;
        children.erase(it);
    }
}

Node* Node::get_child(size_t index) const {
    if (index < children.size()) {
        return children[index];
    }
    return nullptr;
}

String Node::get_path() const {
    if (!parent) {
        return "/" + name;
    }
    return parent->get_path() + "/" + name;
}

// -----------------------------------------------------------------------------
// PROPAGACIÓN DEL CICLO DE VIDA
// -----------------------------------------------------------------------------
void Node::propagate_ready() {
    // Primero se inicializan los hijos (de abajo hacia arriba)
    for (Node* child : children) {
        if (!child->is_ready) {
            child->propagate_ready();
        }
    }

    // Luego el padre se marca listo
    if (!is_ready) {
        is_ready = true;
        _ready();
    }
}

void Node::propagate_process(float delta) {
    // Procesar este nodo
    _process(delta);

    // Propagar hacia abajo a cada hijo
    for (size_t i = 0; i < children.size(); ++i) {
        if (children[i]) {
            children[i]->propagate_process(delta);
        }
    }
}

Node::Node(const String& node_name) 
    : name(node_name), parent(nullptr), is_ready(false) {
}

Node::~Node() {
    // 1. Notificar salida del árbol
    _exit_tree();

    // 2. Desvincularse del padre si todavía existe
    if (parent) {
        parent->remove_child(this);
    }

    // 3. Destrucción en cascada de todos los hijos en el heap
    for (Node* child : children) {
        child->parent = nullptr; // Evita que el hijo intente llamar a remove_child en este destructor
        delete child;
    }
    children.clear();
}

SceneTree* Node::get_tree() const {
    return &SceneTree::get();
}
