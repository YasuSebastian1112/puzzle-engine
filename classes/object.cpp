#include "object.h"
#include "class_db.h"
#include "core/scene_tree.h"

void Object::_bind_methods() {
    // La raíz Object no expone propiedades específicas por defecto
}

bool Object::set(const std::string& p_property, const std::vector<std::string>& p_args) {
    return ClassDB::get_singleton().set_property(this, get_class_name(), p_property, p_args);
}

std::string Object::get(const std::string& p_property) const {
    return ClassDB::get_singleton().get_property(this, get_class_name(), p_property);
}

Object::Object() {
    instance_id = next_instance_id++;
}

Object::~Object() = default;

void Object::queue_free() {
    if (!queued_for_deletion) {
        queued_for_deletion = true;
        SceneTree::get().queue_delete(this);
    }
}

// -----------------------------------------------------------------------------
// SEÑALES
// -----------------------------------------------------------------------------
void Object::connect(const std::string& p_signal, const SignalCallback& p_callback) {
    if (!signal_map) {
        signal_map = std::make_unique<SignalMap>();
    }
    (*signal_map)[p_signal].push_back(p_callback);
}

void Object::emit_signal(const std::string& p_signal, const std::vector<Variant>& p_args) {
    if (!signal_map) return;

    auto it = signal_map->find(p_signal);
    if (it != signal_map->end()) {
        for (const auto& callback : it->second) {
            if (callback) {
                callback(p_args);
            }
        }
    }
}

void Object::disconnect_all(const std::string& p_signal) {
    if (!signal_map) return;

    signal_map->erase(p_signal);
    if (signal_map->empty()) {
        signal_map.reset();
    }
}

// -----------------------------------------------------------------------------
// METADATA
// -----------------------------------------------------------------------------
void Object::set_meta(const std::string& p_name, const Variant& p_value) {
    if (!metadata) {
        metadata = std::make_unique<MetaMap>();
    }
    (*metadata)[p_name] = p_value;
}

Variant Object::get_meta(const std::string& p_name, const Variant& p_default) const {
    if (!metadata) return p_default;

    auto it = metadata->find(p_name);
    if (it != metadata->end()) {
        return it->second;
    }
    return p_default;
}

bool Object::has_meta(const std::string& p_name) const {
    if (!metadata) return false;
    return metadata->find(p_name) != metadata->end();
}

void Object::remove_meta(const std::string& p_name) {
    if (!metadata) return;

    metadata->erase(p_name);
    if (metadata->empty()) {
        metadata.reset();
    }
}