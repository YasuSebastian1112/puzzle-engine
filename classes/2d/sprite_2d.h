// Puzzle Engine - 2D/3D Hybrid Game Engine
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// ========================================================================= //

#pragma once
#include "node_2d.h"
#include <glad/glad.h>
#include <string>

class Sprite2D : public Node2D {
    CLASS_TYPE(Sprite2D, Node2D)

public:
    Sprite2D(const std::string& node_name = "Sprite2D");
    virtual ~Sprite2D() override;

    // Carga directa de archivo de imagen
    bool load_texture_from_file(const std::string& path);

    // Dibuja el sprite si hay textura cargada
    virtual void _draw() override;

    int get_width() const { return width; }
    int get_height() const { return height; }

private:
    GLuint texture_id = 0;
    GLuint vao = 0;
    GLuint vbo = 0;
    int width = 0;
    int height = 0;

    void setup_mesh();
};