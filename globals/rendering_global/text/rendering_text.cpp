// ========================================================================= //
// PUZZLE ENGINE - 2D/3D GAME ENGINE
// ------------------------------------------------------------------------- //
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// ========================================================================= //

#include "globals/rendering_global/text/rendering_text.h"
#include "globals/rendering_global/device/rendering_geometry.h"

#include <iostream>
#include <vector>
#include <algorithm>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <glad/glad.h>

RenderingText::RenderingText() {
}

RenderingText::~RenderingText() {
    if (m_ft_face) {
        FT_Done_Face(static_cast<FT_Face>(m_ft_face));
        m_ft_face = nullptr;
    }
    if (m_ft_library) {
        FT_Done_FreeType(static_cast<FT_Library>(m_ft_library));
        m_ft_library = nullptr;
    }
}

bool RenderingText::load_font(const String& p_filepath, uint32_t p_pixel_size) {
    font_size = p_pixel_size;

    FT_Library ft_lib = nullptr;
    if (FT_Init_FreeType(&ft_lib)) {
        std::cerr << "\033[31m[RenderingText] Error al inicializar FreeType.\033[0m\n";
        return false;
    }
    m_ft_library = ft_lib;

    FT_Face face = nullptr;
    if (FT_New_Face(ft_lib, p_filepath.c_str(), 0, &face)) {
        std::cerr << "\033[31m[RenderingText] No se pudo cargar la fuente: " << p_filepath << "\033[0m\n";
        FT_Done_FreeType(ft_lib);
        m_ft_library = nullptr;
        return false;
    }
    m_ft_face = face;

    // Configurar el alto en píxeles (ancho en 0 calcula automáticamente la proporción)
    FT_Set_Pixel_Sizes(face, 0, font_size);

    // Métricas generales de la fuente (FreeType entrega en 1/64 de píxel -> shift >> 6)
    line_height = static_cast<float>(face->size->metrics.height >> 6);
    ascender    = static_cast<float>(face->size->metrics.ascender >> 6);
    descender   = static_cast<float>(face->size->metrics.descender >> 6);

    _build_texture_atlas();
    return true;
}

void RenderingText::_build_texture_atlas() {
    if (!m_ft_face) return;
    FT_Face face = static_cast<FT_Face>(m_ft_face);

    // Atlas cuadrado de 1024x1024 para almacenar caracteres ASCII y Latin-1 Extendido
    const int ATLAS_WIDTH = 1024;
    const int ATLAS_HEIGHT = 1024;

    // 4 canales (RGBA) para empatar directamente con los shaders estándar del batcher
    std::vector<uint8_t> atlas_pixels(ATLAS_WIDTH * ATLAS_HEIGHT * 4, 0);

    int pen_x = 1;
    int pen_y = 1;
    int max_row_h = 0;

    m_glyphs.clear();

    // Rango extendido: ASCII (32-126) y Latin-1 (128-255: acentos, ñ, ¿, ¡)
    for (uint32_t codepoint = 32; codepoint < 256; ++codepoint) {
        if (FT_Load_Char(face, codepoint, FT_LOAD_RENDER)) {
            continue;
        }

        FT_GlyphSlot slot = face->glyph;
        int gw = slot->bitmap.width;
        int gh = slot->bitmap.rows;

        // Salto a una nueva fila si excede el ancho del atlas
        if (pen_x + gw + 1 >= ATLAS_WIDTH) {
            pen_x = 1;
            pen_y += max_row_h + 1;
            max_row_h = 0;
        }

        if (pen_y + gh + 1 >= ATLAS_HEIGHT) {
            std::cerr << "\033[33m[RenderingText] Atlas desbordado para la fuente.\033[0m\n";
            break;
        }

        // Copiar el canal monocromático a color blanco puro en RGB y la forma al Alpha
        for (int row = 0; row < gh; ++row) {
            for (int col = 0; col < gw; ++col) {
                uint8_t alpha = slot->bitmap.buffer[row * slot->bitmap.pitch + col];
                int dst_idx = ((pen_y + row) * ATLAS_WIDTH + (pen_x + col)) * 4;

                atlas_pixels[dst_idx + 0] = 255;   // R
                atlas_pixels[dst_idx + 1] = 255;   // G
                atlas_pixels[dst_idx + 2] = 255;   // B
                atlas_pixels[dst_idx + 3] = alpha; // A
            }
        }

        GlyphMetrics metrics;
        metrics.size = Vector2(static_cast<float>(gw), static_cast<float>(gh));
        metrics.bearing = Vector2(static_cast<float>(slot->bitmap_left), static_cast<float>(slot->bitmap_top));
        metrics.advance = static_cast<float>(slot->advance.x >> 6);

        metrics.uv_min = Vector2(
            static_cast<float>(pen_x) / static_cast<float>(ATLAS_WIDTH),
            static_cast<float>(pen_y) / static_cast<float>(ATLAS_HEIGHT)
        );
        metrics.uv_max = Vector2(
            static_cast<float>(pen_x + gw) / static_cast<float>(ATLAS_WIDTH),
            static_cast<float>(pen_y + gh) / static_cast<float>(ATLAS_HEIGHT)
        );

        m_glyphs[codepoint] = metrics;

        max_row_h = std::max(max_row_h, gh);
        pen_x += gw + 1;
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
}

const GlyphMetrics* RenderingText::get_glyph(uint32_t p_codepoint) const {
    auto it = m_glyphs.find(p_codepoint);
    if (it != m_glyphs.end()) {
        return &it->second;
    }
    return nullptr;
}

Vector2 RenderingText::get_string_size(const String& p_text) const {
    float cur_line_w = 0.0f;
    float max_w = 0.0f;
    float total_h = line_height;

    for (size_t i = 0; i < p_text.length(); ++i) {
        uint8_t c = static_cast<uint8_t>(p_text[i]);
        if (c == '\n') {
            max_w = std::max(max_w, cur_line_w);
            cur_line_w = 0.0f;
            total_h += line_height;
            continue;
        }

        const GlyphMetrics* g = get_glyph(c);
        if (g) {
            cur_line_w += g->advance;
        }
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
    cursor.y += ascender; // Colocar el cursor sobre la línea base tipográfica

    for (size_t i = 0; i < p_text.length(); ++i) {
        uint8_t c = static_cast<uint8_t>(p_text[i]);

        if (c == '\n') {
            cursor.x = p_position.x;
            cursor.y += line_height;
            continue;
        }

        const GlyphMetrics* g = get_glyph(c);
        if (!g) continue;

        // Vértices del glifo en coordenadas de pantalla
        float x0 = cursor.x + g->bearing.x;
        float y0 = cursor.y - g->bearing.y;
        float x1 = x0 + g->size.x;
        float y1 = y0 + g->size.y;

        Vector2 v0(x0, y0);
        Vector2 v1(x1, y0);
        Vector2 v2(x1, y1);
        Vector2 v3(x0, y1);

        Vector2 uv0(g->uv_min.x, g->uv_min.y);
        Vector2 uv1(g->uv_max.x, g->uv_min.y);
        Vector2 uv2(g->uv_max.x, g->uv_max.y);
        Vector2 uv3(g->uv_min.x, g->uv_max.y);

        p_geometry->draw_quad_textured(v0, v1, v2, v3, uv0, uv1, uv2, uv3, atlas_texture, p_color);

        cursor.x += g->advance;
    }
}