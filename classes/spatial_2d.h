// ========================================================================= //
// Puzzle Engine - 2D/3D Hybrid Game Engine
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// ===============================================

#pragma once
#include "node.h"
#include <glm/glm.hpp>

class Spatial2D : public Node {
    CLASS_TYPE(Spatial2D, Node)

public:

    // Propiedades comunes de render 2D
    bool is_visible() const {return visible;}
    void set_visible(bool p_visible) {visible = p_visible;}

    int get_z_index() const {return z_index;}
    void set_z_index(int p_z) {z_index = p_z;}

    glm::vec4 get_modulate() const {return modulate;}
    void set_modulate(const glm::vec4& p_color) {modulate = p_color;}

    // Interfaz de renderizado
    virtual void _draw() {}
    virtual glm::mat4 get_transform() const = 0; // Obliga a Node2D y Control a definir su matriz


    Spatial2D(const std::string& node_name = "Spatial2D") 
        : Node(node_name), visible(true), z_index(0), modulate(1.0f, 1.0f, 1.0f, 1.0f) {}
    virtual ~Spatial2D() override = default;

protected:

    bool visible;
    int z_index;
    glm::vec4 modulate; // RGBA

};