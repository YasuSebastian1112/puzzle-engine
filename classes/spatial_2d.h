// ============================================================================ //
// Puzzle Engine - 2D/3D Hybrid Game Engine
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// ============================================================================ //

#pragma once

#include "classes/node.h"
#include "core/color.h"

// Forward declaration del batcher para no sobrecargar de includes este header
class RenderingGeometry;

class Spatial2D : public Node {
    CLASS_TYPE(Spatial2D, Node);

public:
    // -------------------------------------------------------------------------
    // PROPIEDADES COMUNES DE RENDER 2D
    // -------------------------------------------------------------------------
    bool is_visible() const { return visible; }
    void set_visible(bool p_visible);
    bool is_visible_in_tree() const;

    int get_z_index() const { return z_index; }
    void set_z_index(int p_z) { z_index = p_z; }

    Color get_modulate() const { return modulate; }
    void set_modulate(const Color& p_color) { modulate = p_color; }
    Color get_global_modulate() const;

    // -------------------------------------------------------------------------
    // INTERFAZ DE TRANSFORMADA (Ahora puramente 2D afín)
    // -------------------------------------------------------------------------
    virtual Transform2D get_transform() const = 0;
    virtual Transform2D get_global_transform() const;

    // Ciclo de dibujo
    virtual void _draw(RenderingGeometry* p_geometry) {}
    void propagate_draw(RenderingGeometry* p_geometry);
    void queue_redraw() { redraw_requested = true; }
    bool is_redraw_requested() const { return redraw_requested; }

    Spatial2D(const String& node_name = "Spatial2D");
    virtual ~Spatial2D() override = default;

protected:
    bool visible = true;
    int z_index = 0;
    Color modulate = Color(1.0f, 1.0f, 1.0f, 1.0f);
    bool redraw_requested = true;
};