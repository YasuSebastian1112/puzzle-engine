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
    Vector2 world_pos = xform.xform(local_pos);
    Vector2 world_scale = get_global_scale();

    Rect2 dest_rect(
        world_pos.x,
        world_pos.y,
        tex_size.x * world_scale.x,
        tex_size.y * world_scale.y
    );

    // Emisión al batcher existente con modulación de color
    p_geometry->draw_texture(dest_rect, r_tex, get_global_modulate());
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