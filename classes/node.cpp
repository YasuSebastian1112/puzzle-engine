// ========================================================================= //
// PUZZLE ENGINE - 2D/3D GAME ENGINE
// ------------------------------------------------------------------------- //
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// ========================================================================= //

#include "node.h"
#include "core/class_db.h"
#include "core/scene_tree.h"
#include <algorithm>

// -----------------------------------------------------------------------------
// GESTIÓN DEL ÁRBOL
// -----------------------------------------------------------------------------
void Node::add_child(Node* child) {
    if (!child || child == this) return;

    if (child->parent) {
        child->parent->remove_child(child);
    }

    child->parent = this;
    children.push_back(child);

    child->_enter_tree();

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

Node* Node::find_child(const String& child_name) const {
    for (Node* child : children) {
        if (child && child->get_name() == child_name) {
            return child;
        }
    }
    return nullptr;
}

Node* Node::get_node(const String& path) const {
    if (path.empty()) return nullptr;

    std::string s_path = path.c_str();
    const Node* current = this;

    // Manejo de ruta absoluta si inicia con '/'
    if (s_path[0] == '/') {
        while (current->parent) {
            current = current->parent;
        }
    }

    std::string segment;
    for (size_t i = 0; i <= s_path.length(); ++i) {
        if (i == s_path.length() || s_path[i] == '/') {
            if (!segment.empty() && segment != ".") {
                if (segment == "..") {
                    current = current->get_parent();
                    if (!current) return nullptr;
                } else {
                    current = current->find_child(segment.c_str());
                    if (!current) return nullptr;
                }
                segment.clear();
            }
        } else {
            segment += s_path[i];
        }
    }

    return const_cast<Node*>(current);
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
    for (Node* child : children) {
        if (!child->is_ready) {
            child->propagate_ready();
        }
    }

    if (!is_ready) {
        is_ready = true;
        _ready();
    }
}

void Node::propagate_process(float delta) {
    _process(delta);

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
    _exit_tree();

    if (parent) {
        parent->remove_child(this);
    }

    for (Node* child : children) {
        child->parent = nullptr;
        delete child;
    }
    children.clear();
}

SceneTree* Node::get_tree() const {
    return &SceneTree::get();
}


void Node::_bind_methods() {
    ClassDB::add_property("name", &Node::set_name, &Node::get_name);   
}