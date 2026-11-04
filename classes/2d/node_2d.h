// ========================================================================= //
// PUZZLE ENGINE - 2D/3D GAME ENGINE
// ------------------------------------------------------------------------- //
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// ========================================================================= //

#pragma once

#include "classes/spatial_2d.h"
#include "core/mathematics/math_2d.h"
#include "core/mathematics/transform_2d.h"

class Node2D : public Spatial2D {
    CLASS_TYPE(Node2D, Spatial2D);

public:
    
    // TRANSFORMACIÓN LOCAL (Encapsulada con control de dirty flag)
    
    void set_position(const Vector2& p_pos);
    Vector2 get_position() const { return position; }

    void set_rotation(float p_radians);
    float get_rotation() const { return rotation; }

    // Soporte para grados (grados sexagesimales directos para el CLI y el inspector)
    void set_rotation_degrees(float p_degrees);
    float get_rotation_degrees() const;

    void set_scale(const Vector2& p_scale);
    Vector2 get_scale() const { return scale; }

    // Métodos de conveniencia para desplazamientos relativos
    void translate(const Vector2& p_offset);
    void rotate(float p_radians);

    
    // MATRICES AFINES (Con caché)
    
    virtual Transform2D get_transform() const override;

    
    // TRANSFORMACIÓN GLOBAL Y CONVERSIÓN DE ESPACIOS
    
    Vector2 get_global_position() const;
    void set_global_position(const Vector2& p_global_pos);

    float get_global_rotation() const;
    Vector2 get_global_scale() const;

    // Conversión de coordenadas
    Vector2 to_local(const Vector2& p_global_point) const;
    Vector2 to_global(const Vector2& p_local_point) const;

    // Apuntar hacia un objetivo en el mundo (por ejemplo, el cursor o un enemigo)
    void look_at(const Vector2& p_target);

    Node2D(const String& node_name = "Node2D");
    virtual ~Node2D() override = default;

private:
    void _update_transform() const;
    // Propagación recursiva para invalidar la caché de los hijos cuando el padre rota o se traslada
    void _propagate_transform_changed();

    Vector2 position = Vector2(0.0f, 0.0f);
    float rotation = 0.0f; // En radianes
    Vector2 scale = Vector2(1.0f, 1.0f);

    // Caché mutable para evitar llamadas trigonométricas repetitivas
    mutable Transform2D local_transform;
    mutable bool transform_dirty = true;
    
};