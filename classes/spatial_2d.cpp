#include "classes/spatial_2d.h"
#include "globals/rendering_global/device/rendering_geometry.h"

Spatial2D::Spatial2D(const String& node_name)
    : Node(node_name), visible(true), z_index(0), modulate(1.0f, 1.0f, 1.0f, 1.0f) {
}

void Spatial2D::set_visible(bool p_visible) {
    visible = p_visible;
}

bool Spatial2D::is_visible_in_tree() const {
    if (!visible) return false;
    const Spatial2D* parent_spatial = Object::cast_to<Spatial2D>(get_parent());
    if (parent_spatial) {
        return parent_spatial->is_visible_in_tree();
    }
    return true;
}

Color Spatial2D::get_global_modulate() const {
    const Spatial2D* parent_spatial = Object::cast_to<Spatial2D>(get_parent());
    if (parent_spatial) {
        return parent_spatial->get_global_modulate() * modulate;
    }
    return modulate;
}

Transform2D Spatial2D::get_global_transform() const {
    Transform2D local = get_transform();

    const Spatial2D* parent_spatial = Object::cast_to<Spatial2D>(get_parent());
    if (parent_spatial) {
        // Multiplicación afín 2D directa: mucho más rápida que glm::mat4
        return parent_spatial->get_global_transform() * local;
    }

    return local;
}

void Spatial2D::propagate_draw(RenderingGeometry* p_geometry) {
    if (!is_visible_in_tree() || !p_geometry) return;

    _draw(p_geometry);

    for (size_t i = 0; i < get_child_count(); ++i) {
        Spatial2D* child_spatial = Object::cast_to<Spatial2D>(get_child(i));
        if (child_spatial) {
            child_spatial->propagate_draw(p_geometry);
        }
    }
}