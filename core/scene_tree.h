// ============================================================================ //
// Puzzle Engine - 2D/3D Hybrid Game Engine
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// ============================================================================ //

#pragma once

#include <vector>
#include <string>

// Forward declarations para evitar inclusiones circulares
class Object;
class Node;

class SceneTree {
public:
    static SceneTree& get() {
        static SceneTree instance;
        return instance;
    }

    // GESTIÓN DEL NODO RAÍZ Y CAMBIO DE ESCENA
    void set_root(Node* p_root);
    Node* get_root() const { return root; }
    
    // Cambia la escena activa eliminando la anterior de forma limpia
    void change_scene_to_node(Node* p_new_scene);

    // CICLO DE PROCESAMIENTO
    // Invocado una vez por frame desde el main loop
    void iteration(float p_delta);

    // Control de pausa del juego
    void set_pause(bool p_pause) { paused = p_pause; }
    bool is_paused() const { return paused; }

    // COLA DE ELIMINACIÓN DIFERIDA (queue_free)
    void queue_delete(Object* p_object);
    void flush_deletion_queue();

    // Limpieza total al cerrar el motor
    void shutdown();

private:
    SceneTree() = default;
    ~SceneTree();

    SceneTree(const SceneTree&) = delete;
    SceneTree& operator=(const SceneTree&) = delete;

    Node* root = nullptr;
    bool paused = false;

    // Buffer de objetos pendientes por destruir en el frame actual
    std::vector<Object*> deletion_queue;
};