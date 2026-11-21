// ========================================================================= //
// PUZZLE ENGINE - 2D/3D GAME ENGINE
// ------------------------------------------------------------------------- //
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// ========================================================================= //

#include "classes/resources/font_resource.h"
#include "licenses_generated/default_font.h"
#include <unordered_map>

namespace {
    // Caché indexada por tamaño en píxeles: 1 solo atlas por tamaño en todo el motor
    std::unordered_map<uint32_t, Ref<FontResource>> s_default_font_cache;
}

bool FontResource::load_from_file(const String& p_filepath, uint32_t p_pixel_size) {
    set_path(p_filepath);
    m_font_size = p_pixel_size;
    m_from_memory = false;
    m_loaded = m_rendering_text.load_font(p_filepath, p_pixel_size);
    return m_loaded;
}

bool FontResource::load_from_memory(const uint8_t* p_data, size_t p_size, uint32_t p_pixel_size) {
    m_font_size = p_pixel_size;
    m_from_memory = true;
    m_loaded = m_rendering_text.load_font_from_memory(p_data, p_size, p_pixel_size);
    return m_loaded;
}

void FontResource::reload_from_file() {
    if (!m_from_memory && !get_path().empty()) {
        m_loaded = m_rendering_text.load_font(get_path(), m_font_size);
    }
}

Ref<FontResource> FontResource::get_default_font(uint32_t p_pixel_size) {
    auto it = s_default_font_cache.find(p_pixel_size);
    if (it != s_default_font_cache.end() && it->second.is_valid()) {
        return it->second;
    }

    Ref<FontResource> font = new FontResource();
    if (font->load_from_memory(EmbeddedAssets::DEFAULT_FONT_TTF, EmbeddedAssets::DEFAULT_FONT_TTF_SIZE, p_pixel_size)) {
        s_default_font_cache[p_pixel_size] = font;
        return font;
    }

    return Ref<FontResource>();
}

void FontResource::clear_cache() {
    s_default_font_cache.clear();
}

Vector2 FontResource::get_string_size(const String& p_text) const {
    if (!m_loaded) return Vector2(0.0f, 0.0f);
    return m_rendering_text.get_string_size(p_text);
}

void FontResource::draw_text(
    RenderingGeometry* p_geometry,
    const Vector2& p_position,
    const String& p_text,
    const Color& p_color
) const {
    if (!m_loaded) return;
    m_rendering_text.draw_text(p_geometry, p_position, p_text, p_color);
}