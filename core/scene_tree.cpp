// ============================================================================ //
// Puzzle Engine - 2D/3D Hybrid Game Engine
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// ============================================================================ //

#include "core/scene_tree.h"
#include "classes/object.h"
#include "classes/node.h"
#include <algorithm>

SceneTree::~SceneTree() {
    shutdown();
}

void SceneTree::set_root(Node* p_root) {
    if (root == p_root) return;

    if (root) {
        delete root;
    }

    root = p_root;

    if (root) {
        root->propagate_ready();
    }
}

void SceneTree::change_scene_to_node(Node* p_new_scene) {
    if (!p_new_scene) return;

    // Marcamos la raíz actual para borrado diferido si existe
    if (root) {
        queue_delete(root);
        root = nullptr;
    }

    set_root(p_new_scene);
}

void SceneTree::iteration(float p_delta) {
    // 1. Procesar la lógica de los nodos si no está en pausa
    if (root && !paused) {
        root->propagate_process(p_delta);
    }

    // 2. Al final de la iteración, vaciar de forma segura la cola de queue_free
    flush_deletion_queue();
}

void SceneTree::queue_delete(Object* p_object) {
    if (!p_object) return;

    // Evitar duplicados en la cola si se llama queue_free() varias veces al mismo objeto
    auto it = std::find(deletion_queue.begin(), deletion_queue.end(), p_object);
    if (it == deletion_queue.end()) {
        deletion_queue.push_back(p_object);
    }
}

void SceneTree::flush_deletion_queue() {
    if (deletion_queue.empty()) return;

    // Copiamos la lista y limpiamos el vector original de inmediato
    // por si algún destructor llama indirectamente a queue_free de otro objeto
    std::vector<Object*> to_delete = std::move(deletion_queue);
    deletion_queue.clear();

    for (Object* obj : to_delete) {
        if (!obj) continue;

        // Si el objeto es un Node, desvincularlo primero de su padre
        Node* node = Object::cast_to<Node>(obj);
        if (node) {
            if (node == root) {
                root = nullptr;
            } else if (node->get_parent()) {
                node->get_parent()->remove_child(node);
            }
        }

        // Destrucción física en el heap
        delete obj;
    }
}

void SceneTree::shutdown() {
    flush_deletion_queue();

    if (root) {
        delete root;
        root = nullptr;
    }
}