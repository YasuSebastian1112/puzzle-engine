// ========================================================================= //
// PUZZLE ENGINE - 2D/3D GAME ENGINE
// ------------------------------------------------------------------------- //
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// ========================================================================= //

#include "class_db.h"
#include "object.h"
#include <iostream>

void ClassDB::register_class_internal(const std::string& class_name, const std::string& parent_name) {
    ClassInfo info;
    info.name = class_name;
    info.parent_class = parent_name;
    classes[class_name] = info;
}

void ClassDB::add_property_internal(const std::string& class_name, const PropertyInfo& prop) {
    auto it = classes.find(class_name);
    if (it == classes.end()) {
        std::cerr << "[ClassDB] Error: La clase '" << class_name << "' debe registrarse antes de anadir propiedades.\n";
        return;
    }
    it->second.properties[prop.name] = prop;
}

const ClassInfo* ClassDB::get_class_info(const std::string& class_name) const {
    auto it = classes.find(class_name);
    if (it != classes.end()) {
        return &it->second;
    }
    return nullptr;
}

bool ClassDB::set_property(Object* obj, const std::string& class_name, const std::string& prop_name, const std::vector<std::string>& args) {
    std::string current_class = class_name;

    // Recorre la cadena de herencia hacia arriba hasta encontrar la propiedad
    while (!current_class.empty()) {
        auto it = classes.find(current_class);
        if (it == classes.end()) {
            break;
        }

        const ClassInfo& info = it->second;
        auto prop_it = info.properties.find(prop_name);
        if (prop_it != info.properties.end()) {
            prop_it->second.setter(obj, args);
            return true;
        }

        current_class = info.parent_class;
    }

    return false;
}

std::string ClassDB::get_property(const Object* obj, const std::string& class_name, const std::string& prop_name) const {
    std::string current_class = class_name;

    // Recorre la cadena de herencia hacia arriba hasta encontrar el getter
    while (!current_class.empty()) {
        auto it = classes.find(current_class);
        if (it == classes.end()) {
            break;
        }

        const ClassInfo& info = it->second;
        auto prop_it = info.properties.find(prop_name);
        if (prop_it != info.properties.end()) {
            return prop_it->second.getter(obj);
        }

        current_class = info.parent_class;
    }

    return "";
}