// ========================================================================= //
// PUZZLE ENGINE - 2D/3D GAME ENGINE
// ------------------------------------------------------------------------- //
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// ========================================================================= //

#pragma once

#include "ref_counted.h"

class Resource : public RefCounted {
    CLASS_TYPE(Resource, RefCounted);

private:
    std::string path;
    std::string name;

public:
    Resource() = default;
    virtual ~Resource() = default;

    void set_path(const std::string& p_path) { path = p_path; }
    const std::string& get_path() const { return path; }

    void set_name(const std::string& p_name) { name = p_name; }
    const std::string& get_name() const { return name; }

    virtual void reload_from_file() {}
};