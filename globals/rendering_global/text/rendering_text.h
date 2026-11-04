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
#include "globals/rendering_global/device/rendering_texture.h"

#include <unordered_map>

class RenderingGeometry;

// Métricas de posicionamiento y mapeo UV de cada glifo individual
struct GlyphMetrics {
    Vector2 uv_min;      // Coordenada UV normalizada superior izquierda (0.0 - 1.0)
    Vector2 uv_max;      // Coordenada UV normalizada inferior derecha (0.0 - 1.0)
    Vector2 size;        // Dimensiones físicas del glifo en píxeles (ancho, alto)
    Vector2 bearing;     // Desfase horizontal y vertical respecto a la línea base
    float advance = 0.0f; // Espacio horizontal que debe recorrer el cursor tras colocar el glifo
};

class RenderingText : public Resource {

    CLASS_TYPE(RenderingText, Resource);

public:
    RenderingText();
    virtual ~RenderingText() override;

    // Carga el archivo tipográfico (.ttf, .otf) y precomputa el atlas de glifos
    bool load_font(const String& p_filepath, uint32_t p_pixel_size = 24);

    // Consulta de glifo individual por código Unicode / ASCII
    const GlyphMetrics* get_glyph(uint32_t p_codepoint) const;

    // Métricas tipográficas globales
    float get_line_height() const { return line_height; }
    float get_ascender() const { return ascender; }
    float get_descender() const { return descender; }
    uint32_t get_font_size() const { return font_size; }

    // Acceso al atlas generado en GPU
    const RenderingTexture& get_texture() const { return atlas_texture; }

    // Medición geométrica de una cadena completa sin dibujarla (vital para centrar o ajustar cajas de botones)
    Vector2 get_string_size(const String& p_text) const;

    // Rutina de emisión: itera el texto y genera quads hacia el lote de RenderingGeometry
    void draw_text(
        RenderingGeometry* p_geometry,
        const Vector2& p_position,
        const String& p_text,
        const Color& p_color = Color(1.0f, 1.0f, 1.0f, 1.0f)
    ) const;

private:
    // Punteros opacos a las estructuras de FreeType (FT_Library y FT_Face)
    void* m_ft_library = nullptr;
    void* m_ft_face = nullptr;

    RenderingTexture atlas_texture;
    std::unordered_map<uint32_t, GlyphMetrics> m_glyphs;

    uint32_t font_size = 24;
    float line_height = 0.0f;
    float ascender = 0.0f;
    float descender = 0.0f;

    // Rutina interna para empaquetar caracteres (ASCII + Latin-1 Extendido) en la textura atlas
    void _build_texture_atlas();
};