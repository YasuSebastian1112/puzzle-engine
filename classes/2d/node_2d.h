#pragma once
#include "classes/spatial_2d.h"
#include <glm/gtc/matrix_transform.hpp>

class Node2D : public Spatial2D {
    CLASS_TYPE(Node2D, Spatial2D)

public:
    glm::vec2 position = {0.0f, 0.0f};
    float rotation = 0.0f; // Radianes
    glm::vec2 scale = {1.0f, 1.0f};

    virtual glm::mat4 get_transform() const override {
        glm::mat4 local = glm::translate(glm::mat4(1.0f), glm::vec3(position, 0.0f));
        local = glm::rotate(local, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
        local = glm::scale(local, glm::vec3(scale, 1.0f));

        if (get_parent() && get_parent()->is_class("Spatial2D")) {
            return static_cast<Spatial2D*>(get_parent())->get_transform() * local;
        }
        return local;
    }

    Node2D(const std::string& node_name = "Node2D") 
        : Spatial2D(node_name) {}
};