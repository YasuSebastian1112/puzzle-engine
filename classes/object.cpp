// ========================================================================= //
// Puzzle Engine - 2D/3D Hybrid Game Engine
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// ========================================================================= //

#include "object.h"
#include "core/scene_tree.h"
#include <iostream>

Object::Object() {
    instance_id = next_instance_id++;
}

Object::~Object() {
    signal_map.clear();
    metadata.clear();
}

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
    signal_map[p_signal].push_back(p_callback);
}

void Object::emit_signal(const std::string& p_signal, const std::vector<Variant>& p_args) {
    auto it = signal_map.find(p_signal);
    if (it != signal_map.end()) {
        for (const auto& callback : it->second) {
            if (callback) {
                callback(p_args);
            }
        }
    }
}

void Object::disconnect_all(const std::string& p_signal) {
    signal_map.erase(p_signal);
}

// -----------------------------------------------------------------------------
// METADATA
// -----------------------------------------------------------------------------
void Object::set_meta(const std::string& p_name, const Variant& p_value) {
    metadata[p_name] = p_value;
}

Variant Object::get_meta(const std::string& p_name, const Variant& p_default) const {
    auto it = metadata.find(p_name);
    if (it != metadata.end()) {
        return it->second;
    }
    return p_default;
}

bool Object::has_meta(const std::string& p_name) const {
    return metadata.find(p_name) != metadata.end();
}

void Object::remove_meta(const std::string& p_name) {
    metadata.erase(p_name);
}