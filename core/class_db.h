// ========================================================================= //
// PUZZLE ENGINE - 2D/3D GAME ENGINE
// ------------------------------------------------------------------------- //
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// ========================================================================= //

#pragma once

#include <string>
#include <unordered_map>
#include <functional>
#include <vector>
#include <type_traits>
#include "property_converter.h"

class Object;

struct PropertyInfo {
    std::string name;
    std::function<void(Object*, const std::vector<std::string>&)> setter;
    std::function<std::string(const Object*)> getter;
};

struct ClassInfo {
    std::string name;
    std::string parent_class;
    std::unordered_map<std::string, PropertyInfo> properties;
};

class ClassDB {
public:
    static ClassDB& get_singleton() {
        static ClassDB singleton;
        return singleton;
    }

    template <typename T>
    static void register_class() {
        std::string class_name = T::get_class_static();
        std::string parent_name = T::get_parent_class_static();

        get_singleton().register_class_internal(class_name, parent_name);
        T::_bind_methods();
    }

    // Sobrecarga 1: Setter por valor (ej: void set_rotation(float))
    template <typename T, typename PropType, typename RetType>
    static void add_property(
        const std::string& prop_name,
        void (T::*setter)(PropType),
        RetType (T::*getter)() const
    ) {
        using CleanType = std::decay_t<PropType>;

        PropertyInfo prop;
        prop.name = prop_name;

        prop.setter = [setter](Object* obj, const std::vector<std::string>& args) {
            T* instance = static_cast<T*>(obj);
            if (instance) {
                (instance->*setter)(PropertyConverter<CleanType>::from_args(args));
            }
        };

        prop.getter = [getter](const Object* obj) -> std::string {
            const T* instance = static_cast<const T*>(obj);
            if (instance) {
                return PropertyConverter<CleanType>::to_string((instance->*getter)());
            }
            return "";
        };

        get_singleton().add_property_internal(T::get_class_static(), prop);
    }

    // Sobrecarga 2: Setter por referencia constante (ej: void set_name(const String&))
    template <typename T, typename PropType, typename RetType>
    static void add_property(
        const std::string& prop_name,
        void (T::*setter)(const PropType&),
        RetType (T::*getter)() const
    ) {
        using CleanType = std::decay_t<PropType>;

        PropertyInfo prop;
        prop.name = prop_name;

        prop.setter = [setter](Object* obj, const std::vector<std::string>& args) {
            T* instance = static_cast<T*>(obj);
            if (instance) {
                (instance->*setter)(PropertyConverter<CleanType>::from_args(args));
            }
        };

        prop.getter = [getter](const Object* obj) -> std::string {
            const T* instance = static_cast<const T*>(obj);
            if (instance) {
                return PropertyConverter<CleanType>::to_string((instance->*getter)());
            }
            return "";
        };

        get_singleton().add_property_internal(T::get_class_static(), prop);
    }

    bool set_property(Object* obj, const std::string& class_name, const std::string& prop_name, const std::vector<std::string>& args);
    std::string get_property(const Object* obj, const std::string& class_name, const std::string& prop_name) const;
    const ClassInfo* get_class_info(const std::string& class_name) const;

private:
    ClassDB() = default;
    ~ClassDB() = default;

    void register_class_internal(const std::string& class_name, const std::string& parent_name);
    void add_property_internal(const std::string& class_name, const PropertyInfo& prop);

    std::unordered_map<std::string, ClassInfo> classes;
};