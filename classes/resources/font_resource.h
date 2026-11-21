// ========================================================================= //
// PUZZLE ENGINE - 2D/3D GAME ENGINE
// ------------------------------------------------------------------------- //
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// ========================================================================= //

#pragma once

#include "classes/resource.h"
#include "core/puzzle_string.h"
#include "core/color.h"
#include "core/mathematics/math_2d.h"
#include "globals/rendering_global/text/rendering_text.h"

class RenderingGeometry;

class FontResource : public Resource {
    CLASS_TYPE(FontResource, Resource);

private:
    RenderingText m_rendering_text;
    uint32_t m_font_size = 16;
    bool m_loaded = false;
    bool m_from_memory = false;

public:
    FontResource() = default;
    virtual ~FontResource() override = default;

    // Carga desde archivo en disco o desde buffer embebido en memoria
    bool load_from_file(const String& p_filepath, uint32_t p_pixel_size = 16);
    bool load_from_memory(const uint8_t* p_data, size_t p_size, uint32_t p_pixel_size = 16);

    virtual void reload_from_file() override;

    // Caché global de fuente por defecto (evita duplicar atlas en Window, Label y Button)
    static Ref<FontResource> get_default_font(uint32_t p_pixel_size = 16);
    static void clear_cache();

    bool is_loaded() const { return m_loaded; }
    uint32_t get_font_size() const { return m_font_size; }
    float get_line_height() const { return m_rendering_text.get_line_height(); }
    float get_ascender() const { return m_rendering_text.get_ascender(); }
    float get_descender() const { return m_rendering_text.get_descender(); }

    Vector2 get_string_size(const String& p_text) const;
    void draw_text(
        RenderingGeometry* p_geometry,
        const Vector2& p_position,
        const String& p_text,
        const Color& p_color = Color(1.0f, 1.0f, 1.0f, 1.0f)
    ) const;

    const RenderingText& get_rendering_text() const { return m_rendering_text; }
};