// ========================================================================= //
// PUZZLE ENGINE - 2D/3D GAME ENGINE
// ------------------------------------------------------------------------- //
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// ========================================================================= //


#pragma once

#include "texture.h"
#include "globals/rendering_global/device/rendering_texture.h"

class Texture2D : public Texture {
    CLASS_TYPE(Texture2D, Texture);

private:
    RenderingTexture m_texture;

public:
    Texture2D() = default;
    virtual ~Texture2D() {
        m_texture.release();
    }

    bool load(const std::string& p_path, bool p_filter_linear = false) {
        set_path(p_path);
        return m_texture.load_from_file(p_path, p_filter_linear);
    }

    bool load_from_memory(const uint8_t* p_buffer, size_t p_size, bool p_filter_linear = false) {
        return m_texture.load_from_memory(p_buffer, p_size, p_filter_linear);
    }

    virtual int get_width() const override { return static_cast<int>(m_texture.get_width()); }
    virtual int get_height() const override { return static_cast<int>(m_texture.get_height()); }
    virtual uint32_t get_rid() const override { return m_texture.get_id(); }
    virtual bool is_valid() const override { return m_texture.is_valid(); }

    const RenderingTexture& get_rendering_texture() const { return m_texture; }
};