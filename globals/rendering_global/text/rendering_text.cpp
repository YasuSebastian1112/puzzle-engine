// ========================================================================= //
// PUZZLE ENGINE - 2D/3D GAME ENGINE
// ------------------------------------------------------------------------- //
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// ========================================================================= //

#include "globals/rendering_global/text/rendering_text.h"
#include "globals/rendering_global/device/rendering_geometry.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <glad/glad.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "external/stb/stb_truetype.h"

// Decodifica el siguiente codepoint UTF-8 y avanza el índice
static uint32_t decode_utf8(const String& p_text, size_t& p_index) {
    uint8_t b0 = static_cast<uint8_t>(p_text[p_index]);
    if (b0 < 0x80) {
        p_index += 1;
        return b0;
    }
    if ((b0 & 0xE0) == 0xC0 && p_index + 1 < p_text.length()) {
        uint8_t b1 = static_cast<uint8_t>(p_text[p_index + 1]);
        p_index += 2;
        return ((b0 & 0x1F) << 6) | (b1 & 0x3F);
    }
    if ((b0 & 0xF0) == 0xE0 && p_index + 2 < p_text.length()) {
        uint8_t b1 = static_cast<uint8_t>(p_text[p_index + 1]);
        uint8_t b2 = static_cast<uint8_t>(p_text[p_index + 2]);
        p_index += 3;
        return ((b0 & 0x0F) << 12) | ((b1 & 0x3F) << 6) | (b2 & 0x3F);
    }
    p_index += 1;
    return '?';
}

RenderingText::RenderingText() = default;

RenderingText::~RenderingText() {
    atlas_texture.release();
}

bool RenderingText::load_font(const String& p_filepath, uint32_t p_pixel_size) {
    std::ifstream file(p_filepath.c_str(), std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "\033[31m[RenderingText] No se pudo abrir la fuente: " << p_filepath << "\033[0m\n";
        return false;
    }

    std::streamsize file_size = file.tellg();
    if (file_size <= 0) return false;

    file.seekg(0, std::ios::beg);
    std::vector<uint8_t> buffer(static_cast<size_t>(file_size));
    if (!file.read(reinterpret_cast<char*>(buffer.data()), file_size)) {
        return false;
    }

    return load_font_from_memory(buffer.data(), buffer.size(), p_pixel_size);
}

bool RenderingText::load_font_from_memory(const uint8_t* p_data, size_t p_size, uint32_t p_pixel_size) {
    if (!p_data || p_size == 0) return false;
    font_size = p_pixel_size;
    return _build_texture_atlas(p_data);
}

bool RenderingText::_build_texture_atlas(const uint8_t* p_ttf_data) {
    stbtt_fontinfo font_info;
    int offset = stbtt_GetFontOffsetForIndex(p_ttf_data, 0);
    if (!stbtt_InitFont(&font_info, p_ttf_data, offset)) {
        std::cerr << "\033[31m[RenderingText] Error al inicializar fuente con stb_truetype.\033[0m\n";
        return false;
    }

    atlas_texture.release();
    m_glyphs.clear();

    // Oversampling 2x en tamaños pequeños para igualar la nitidez de FreeType
    const int OVERSAMPLE = (font_size <= 24) ? 2 : 1;
    const float inv_os   = 1.0f / static_cast<float>(OVERSAMPLE);

    float scale = stbtt_ScaleForPixelHeight(&font_info, static_cast<float>(font_size * OVERSAMPLE));

    int ascent = 0, descent = 0, line_gap = 0;
    stbtt_GetFontVMetrics(&font_info, &ascent, &descent, &line_gap);

    ascender    = std::round(static_cast<float>(ascent) * scale * inv_os);
    descender   = std::round(static_cast<float>(descent) * scale * inv_os);
    line_height = std::round(static_cast<float>(ascent - descent + line_gap) * scale * inv_os);

    // Tamaño de atlas ajustado a la resolución efectiva (256 KB / 1 MB / 4 MB)
    const int effective_size = font_size * OVERSAMPLE;
    const int ATLAS_SIZE     = (effective_size <= 18) ? 256 : (effective_size <= 36 ? 512 : 1024);
    const int ATLAS_WIDTH    = ATLAS_SIZE;
    const int ATLAS_HEIGHT   = ATLAS_SIZE;

    std::vector<uint8_t> atlas_pixels(ATLAS_WIDTH * ATLAS_HEIGHT * 4, 0);
    std::vector<uint8_t> glyph_bitmap;

    int pen_x = 1;
    int pen_y = 1;
    int max_row_h = 0;

    // Rango extendido: ASCII (32-126) y Latin-1 (128-255: acentos, ñ, ¿, ¡)
    for (uint32_t codepoint = 32; codepoint < 256; ++codepoint) {
        int advance_w = 0, lsb = 0;
        stbtt_GetCodepointHMetrics(&font_info, static_cast<int>(codepoint), &advance_w, &lsb);

        int x0 = 0, y0 = 0, x1 = 0, y1 = 0;
        stbtt_GetCodepointBitmapBox(&font_info, static_cast<int>(codepoint), scale, scale, &x0, &y0, &x1, &y1);

        int gw = x1 - x0;
        int gh = y1 - y0;

        GlyphMetrics metrics;
        metrics.size    = Vector2(static_cast<float>(gw) * inv_os, static_cast<float>(gh) * inv_os);
        metrics.bearing = Vector2(static_cast<float>(x0) * inv_os, static_cast<float>(-y0) * inv_os);
        metrics.advance = std::round(static_cast<float>(advance_w) * scale * inv_os);

        if (gw > 0 && gh > 0) {
            if (pen_x + gw + 1 >= ATLAS_WIDTH) {
                pen_x = 1;
                pen_y += max_row_h + 1;
                max_row_h = 0;
            }

            if (pen_y + gh + 1 >= ATLAS_HEIGHT) {
                std::cerr << "\033[33m[RenderingText] Atlas desbordado para la fuente.\033[0m\n";
                break;
            }

            glyph_bitmap.resize(static_cast<size_t>(gw * gh));
            stbtt_MakeCodepointBitmap(
                &font_info,
                glyph_bitmap.data(),
                gw, gh, gw,
                scale, scale,
                static_cast<int>(codepoint)
            );

            // Blanco puro en RGB y forma del glifo en el canal Alpha
            for (int row = 0; row < gh; ++row) {
                for (int col = 0; col < gw; ++col) {
                    uint8_t alpha = glyph_bitmap[row * gw + col];
                    int dst_idx = ((pen_y + row) * ATLAS_WIDTH + (pen_x + col)) * 4;

                    atlas_pixels[dst_idx + 0] = 255;
                    atlas_pixels[dst_idx + 1] = 255;
                    atlas_pixels[dst_idx + 2] = 255;
                    atlas_pixels[dst_idx + 3] = alpha;
                }
            }

            metrics.uv_min = Vector2(
                static_cast<float>(pen_x) / static_cast<float>(ATLAS_WIDTH),
                static_cast<float>(pen_y) / static_cast<float>(ATLAS_HEIGHT)
            );
            metrics.uv_max = Vector2(
                static_cast<float>(pen_x + gw) / static_cast<float>(ATLAS_WIDTH),
                static_cast<float>(pen_y + gh) / static_cast<float>(ATLAS_HEIGHT)
            );

            max_row_h = std::max(max_row_h, gh);
            pen_x += gw + 1;
        }

        m_glyphs[codepoint] = metrics;
    }

    // Subir la textura a OpenGL
    GLuint tex_id = 0;
    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, ATLAS_WIDTH, ATLAS_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, atlas_pixels.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    atlas_texture.set_id(tex_id);
    atlas_texture.set_size(Size2i(ATLAS_WIDTH, ATLAS_HEIGHT));
    return true;
}

const GlyphMetrics* RenderingText::get_glyph(uint32_t p_codepoint) const {
    auto it = m_glyphs.find(p_codepoint);
    return (it != m_glyphs.end()) ? &it->second : nullptr;
}

Vector2 RenderingText::get_string_size(const String& p_text) const {
    float cur_line_w = 0.0f;
    float max_w = 0.0f;
    float total_h = line_height;

    size_t i = 0;
    while (i < p_text.length()) {
        uint32_t codepoint = decode_utf8(p_text, i);
        if (codepoint == '\n') {
            max_w = std::max(max_w, cur_line_w);
            cur_line_w = 0.0f;
            total_h += line_height;
            continue;
        }

        const GlyphMetrics* g = get_glyph(codepoint);
        if (g) cur_line_w += g->advance;
    }

    max_w = std::max(max_w, cur_line_w);
    return Vector2(max_w, total_h);
}

void RenderingText::draw_text(
    RenderingGeometry* p_geometry,
    const Vector2& p_position,
    const String& p_text,
    const Color& p_color
) const {
    if (!p_geometry || p_text.empty() || atlas_texture.get_id() == 0) return;

    Vector2 cursor = p_position;
    cursor.y += ascender;

    size_t i = 0;
    while (i < p_text.length()) {
        uint32_t codepoint = decode_utf8(p_text, i);

        if (codepoint == '\n') {
            cursor.x = p_position.x;
            cursor.y += line_height;
            continue;
        }

        const GlyphMetrics* g = get_glyph(codepoint);
        if (!g) continue;

        if (g->size.x > 0.0f && g->size.y > 0.0f) {
            // Pixel-snapping para evitar desenfoque en coordenadas fraccionarias
            float x0 = std::floor(cursor.x + g->bearing.x + 0.5f);
            float y0 = std::floor(cursor.y - g->bearing.y + 0.5f);
            float x1 = x0 + g->size.x;
            float y1 = y0 + g->size.y;

            atlas_texture.draw_quad(
                p_geometry,
                Vector2(x0, y0), Vector2(x1, y0), Vector2(x1, y1), Vector2(x0, y1),
                Vector2(g->uv_min.x, g->uv_min.y), Vector2(g->uv_max.x, g->uv_min.y),
                Vector2(g->uv_max.x, g->uv_max.y), Vector2(g->uv_min.x, g->uv_max.y),
                p_color
            );
        }

        cursor.x += g->advance;
    }
}