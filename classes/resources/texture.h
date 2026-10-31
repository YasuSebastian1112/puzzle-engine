// ========================================================================= //
// PUZZLE ENGINE - 2D/3D GAME ENGINE
// ------------------------------------------------------------------------- //
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// ========================================================================= //

#pragma once

#include "classes/resource.h"
#include "core/mathematics/math_2d.h"

class Texture : public Resource {
    CLASS_TYPE(Texture, Resource);

public:
    Texture() = default;
    virtual ~Texture() = default;

    virtual int get_width() const = 0;
    virtual int get_height() const = 0;
    virtual Size2i get_size() const { return Size2i(get_width(), get_height()); }
    virtual uint32_t get_rid() const = 0; // ID del handle OpenGL (GL_TEXTURE_2D)
    virtual bool is_valid() const = 0;
};