// ========================================================================= //
// Puzzle Engine - 2D/3D Hybrid Game Engine
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// ========================================================================= //

#include "node.h"
#include <algorithm>

void Node::add_child(Node* child) { // Crear nodos hijos
    if (!child || child->parent == this) return;

    if (child->parent) { // Poder tener solo un padre
        child->parent->remove_child(child);
    }

    child->parent = this;
    children.push_back(child);
    child->_enter_tree();

    if (is_ready && !child->is_ready) { // Inicializar el hijo si aun no lo ha hecho
        child->propagate_ready();
    }
}

void Node::remove_child(Node* child) {
    auto it = std::find(children.begin(), children.end(), child); // Localizar el puntero del hijo

    if (it != children.end()) {
        child->_exit_tree();
        child->parent = nullptr;
        children.erase(it);
    }
}

Node* Node::get_child(size_t index) const { // Localizar y retornar el hijo
    if (index < children.size()) { 
        return children[index];
    }

    return nullptr;
}

std::string Node::get_path() const { // Obtener la ruta del hijo
    if (!parent) return "/" + name;
    return parent->get_path() + "/" + name;
}

void Node::propagate_ready() { // Iniciazarse e iniciar a sus hijos
    for (Node* child : children) { 
        child->propagate_ready();
    }
    if (!is_ready) {
        _ready();
        is_ready = true;
    }
}

void Node::propagate_process(float delta) { // Llamar cada delta time
    _process(delta);
    for (Node* child : children) {
        child->propagate_process(delta);
    }
}

Node::Node(const std::string& node_name) : name(node_name), parent(nullptr), is_ready(false) {
}

Node::~Node() {
    for (Node* child : children) {
        delete child;
    }
    children.clear();
}