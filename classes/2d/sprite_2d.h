// ========================================================================= //
// PUZZLE ENGINE - 2D/3D GAME ENGINE
// ------------------------------------------------------------------------- //
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// ========================================================================= //

#pragma once

#include "node_2d.h"
#include "classes/resources/texture_2d.h"

class Sprite2D : public Node2D {
    CLASS_TYPE(Sprite2D, Node2D);

public:
    // Asignación y consulta de la textura base
    void set_texture(const Ref<Texture2D>& p_texture);
    Ref<Texture2D> get_texture() const { return texture; }

    // Control de centrado del sprite respecto a su origen local
    void set_centered(bool p_centered);
    bool is_centered() const { return centered; }

    // Desplazamiento local adicional
    void set_offset(const Vector2& p_offset);
    Vector2 get_offset() const { return offset; }

    // Modificadores de inversión horizontal/vertical
    void set_flip_h(bool p_flip);
    bool is_flipped_h() const { return flip_h; }

    void set_flip_v(bool p_flip);
    bool is_flipped_v() const { return flip_v; }

    // Emisión hacia el batcher de renderizado
    virtual void _draw(RenderingGeometry* p_geometry) override;

    Sprite2D(const String& p_name = "Sprite2D");
    virtual ~Sprite2D() override = default;

private:
    Ref<Texture2D> texture;
    bool centered = true;
    Vector2 offset = Vector2(0.0f, 0.0f);
    bool flip_h = false;
    bool flip_v = false;
    
};