// =============================================================================
// PUZZLE ENGINE — 2D/3D GAME ENGINE
// -----------------------------------------------------------------------------
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// =============================================================================


#pragma once

#include <cstdint>
#include <string>

class RenderingTexture {
public:
    RenderingTexture();
    ~RenderingTexture();

    // Evitar copias accidentales que borren el handle de OpenGL
    RenderingTexture(const RenderingTexture&) = delete;
    RenderingTexture& operator=(const RenderingTexture&) = delete;

    // Permitir semántica de movimiento (Move semantics)
    RenderingTexture(RenderingTexture&& p_other) noexcept;
    RenderingTexture& operator=(RenderingTexture&& p_other) noexcept;

    // Carga desde archivo de imagen en disco (PNG, JPG, BMP)
    bool load_from_file(const std::string& p_filepath, bool p_filter_linear = false);
    bool load_from_memory(const uint8_t* p_buffer, size_t p_size, bool p_filter_linear = false);

    // Creación directa desde búfer de memoria (píxeles crudos RGBA8)
    bool create_from_memory(uint32_t p_width, uint32_t p_height, const uint8_t* p_pixels, bool p_filter_linear = false);

    // Activación y enlace en la unidad de textura especificada
    void bind(uint32_t p_slot = 0) const;
    void unbind() const;

    // Destrucción explícita del recurso en GPU
    void release();

    [[nodiscard]] uint32_t get_id() const { return m_texture_id; }
    [[nodiscard]] uint32_t get_width() const { return m_width; }
    [[nodiscard]] uint32_t get_height() const { return m_height; }
    [[nodiscard]] bool is_valid() const { return m_texture_id != 0; }

private:
    uint32_t m_texture_id = 0;
    uint32_t m_width = 0;
    uint32_t m_height = 0;
};