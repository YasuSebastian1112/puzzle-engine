// ========================================================================= //
// PUZZLE ENGINE - 2D/3D GAME ENGINE
// ------------------------------------------------------------------------- //
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// ========================================================================= //

#include "sprite_2d.h"
#include "globals/rendering_global/device/rendering_geometry.h"
#include "core/class_db.h"

Sprite2D::Sprite2D(const String& p_name) : Node2D(p_name) {}

void Sprite2D::set_texture(const Ref<Texture2D>& p_texture) {
    texture = p_texture;
    queue_redraw();
}

void Sprite2D::set_centered(bool p_centered) {
    centered = p_centered;
    queue_redraw();
}

void Sprite2D::set_offset(const Vector2& p_offset) {
    offset = p_offset;
    queue_redraw();
}

void Sprite2D::set_flip_h(bool p_flip) {
    flip_h = p_flip;
    queue_redraw();
}

void Sprite2D::set_flip_v(bool p_flip) {
    flip_v = p_flip;
    queue_redraw();
}

void Sprite2D::_draw(RenderingGeometry* p_geometry) {
    if (!p_geometry || !texture.is_valid()) return;

    const RenderingTexture& r_tex = texture->get_rendering_texture();

    Size2i raw_size = texture->get_size();
    if (raw_size.width <= 0 || raw_size.height <= 0) return;

    // Origen local según centrado y offset
    Vector2 tex_size(static_cast<float>(raw_size.width), static_cast<float>(raw_size.height));
    Vector2 local_pos = offset;
    if (centered) {
        local_pos -= tex_size * 0.5f;
    }

    // Proyección mediante la matriz afín global de Node2D
    Transform2D xform = get_global_transform();

    // Definición de las 4 esquinas del quad en espacio local (incorporando offset y centrado)
    Vector2 p0 = local_pos;                                      // Top-Left local
    Vector2 p1 = local_pos + Vector2(tex_size.x, 0.0f);          // Top-Right local
    Vector2 p2 = local_pos + tex_size;                           // Bottom-Right local
    Vector2 p3 = local_pos + Vector2(0.0f, tex_size.y);          // Bottom-Left local

    // Transformación de cada vértice al espacio de pantalla aplicando posición, rotación y escala
    Vector2 v0 = xform.xform(p0);
    Vector2 v1 = xform.xform(p1);
    Vector2 v2 = xform.xform(p2);
    Vector2 v3 = xform.xform(p3);

    // Coordenadas UV base con soporte de volteo horizontal y vertical (flip_h y flip_v)
    Vector2 uv_min(flip_h ? 1.0f : 0.0f, flip_v ? 1.0f : 0.0f);
    Vector2 uv_max(flip_h ? 0.0f : 1.0f, flip_v ? 0.0f : 1.0f);

    Vector2 uv0(uv_min.x, uv_min.y);
    Vector2 uv1(uv_max.x, uv_min.y);
    Vector2 uv2(uv_max.x, uv_max.y);
    Vector2 uv3(uv_min.x, uv_max.y);

    // Emisión al batcher existente con modulación de color y vértices transformados
    p_geometry->draw_quad_textured(
        v0, v1, v2, v3,
        uv0, uv1, uv2, uv3,
        r_tex,
        get_global_modulate()
    );
}

// -----------------------------------------------------------------------------
// VINCULACIÓN AL SISTEMA DE REFLEXIÓN (ClassDB)
// -----------------------------------------------------------------------------
void Sprite2D::_bind_methods() {
    ClassDB::add_property("centered", &Sprite2D::set_centered, &Sprite2D::is_centered);
    ClassDB::add_property("offset",   &Sprite2D::set_offset,   &Sprite2D::get_offset);
    ClassDB::add_property("flip_h",   &Sprite2D::set_flip_h,   &Sprite2D::is_flipped_h);
    ClassDB::add_property("flip_v",   &Sprite2D::set_flip_v,   &Sprite2D::is_flipped_v);
}