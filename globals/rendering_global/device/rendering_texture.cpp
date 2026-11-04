// =============================================================================
// PUZZLE ENGINE — 2D/3D GAME ENGINE
// -----------------------------------------------------------------------------
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// =============================================================================

// 1. Las cabeceras del runtime de C y C++ PRIMERO que nada
#include <string.h>
#include <cstring>
#include <cstdlib>

// 2. Cabeceras propias del proyecto
#include "rendering_texture.h"
#include <glad/glad.h>

// 3. Implementación de STB Image
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

RenderingTexture::RenderingTexture() {
    // Inicialización por defecto en 0 (invalida hasta cargar o crear)
}

RenderingTexture::~RenderingTexture() {
    release();
}

RenderingTexture::RenderingTexture(RenderingTexture&& p_other) noexcept
    : m_texture_id(p_other.m_texture_id), m_width(p_other.m_width), m_height(p_other.m_height) {
    p_other.m_texture_id = 0;
    p_other.m_width = 0;
    p_other.m_height = 0;
}

RenderingTexture& RenderingTexture::operator=(RenderingTexture&& p_other) noexcept {
    if (this != &p_other) {
        release();
        m_texture_id = p_other.m_texture_id;
        m_width = p_other.m_width;
        m_height = p_other.m_height;

        p_other.m_texture_id = 0;
        p_other.m_width = 0;
        p_other.m_height = 0;
    }
    return *this;
}

void RenderingTexture::release() {
    if (m_texture_id != 0) {
        glDeleteTextures(1, &m_texture_id);
        m_texture_id = 0;
        m_width = 0;
        m_height = 0;
    }
}

bool RenderingTexture::load_from_file(const std::string& p_filepath, bool p_filter_linear) {
    release();

    // Las coordenadas UV de OpenGL empiezan abajo a la izquierda (0,0)
    stbi_set_flip_vertically_on_load(false);

    int width = 0, height = 0, channels = 0;
    // Forzamos 4 canales (RGBA) para mantener el formato interno uniforme
    stbi_uc* data = stbi_load(p_filepath.c_str(), &width, &height, &channels, 4);

    if (!data) {
        return false;
    }

    bool success = create_from_memory(static_cast<uint32_t>(width), static_cast<uint32_t>(height), data, p_filter_linear);
    stbi_image_free(data);

    return success;
}

bool RenderingTexture::create_from_memory(uint32_t p_width, uint32_t p_height, const uint8_t* p_pixels, bool p_filter_linear) {
    release();

    m_width = p_width;
    m_height = p_height;

    glGenTextures(1, &m_texture_id);
    glBindTexture(GL_TEXTURE_2D, m_texture_id);

    // Modos de envoltura (Wrapping): CLAMP_TO_EDGE evita artefactos en bordes transparentes
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Modos de filtrado: NEAREST para pixel art, LINEAR para gráficos suaves
    GLint filter = p_filter_linear ? GL_LINEAR : GL_NEAREST;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

    // Subida de píxeles a memoria VRAM
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA8,
        m_width,
        m_height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        p_pixels
    );

    glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}

bool RenderingTexture::load_from_memory(const uint8_t* p_buffer, size_t p_size, bool p_filter_linear) {
    if (!p_buffer || p_size == 0) {
        return false;
    }

    release();

    // Mantener coherente con la proyección ortográfica del motor
    stbi_set_flip_vertically_on_load(false);

    int width = 0;
    int height = 0;
    int channels = 0;

    // Decodifica el stream PNG/JPG/BMP directamente desde RAM forzando 4 canales RGBA
    stbi_uc* data = stbi_load_from_memory(
        p_buffer,
        static_cast<int>(p_size),
        &width,
        &height,
        &channels,
        4
    );

    if (!data) {
        return false;
    }

    bool success = create_from_memory(static_cast<uint32_t>(width), static_cast<uint32_t>(height), data, p_filter_linear);
    stbi_image_free(data);

    return success;
}

void RenderingTexture::set_id(uint32_t p_id) {
    if (m_texture_id != p_id) {
        release();
        m_texture_id = p_id;
    }
}

void RenderingTexture::set_size(const Size2i& p_size) {
    m_width = static_cast<uint32_t>(p_size.width > 0 ? p_size.width : 0);
    m_height = static_cast<uint32_t>(p_size.height > 0 ? p_size.height : 0);
}

void RenderingTexture::set_native_handle(uint32_t p_texture_id, uint32_t p_width, uint32_t p_height) {
    release();
    m_texture_id = p_texture_id;
    m_width = p_width;
    m_height = p_height;
}

void RenderingTexture::bind(uint32_t p_slot) const {
    glActiveTexture(GL_TEXTURE0 + p_slot);
    glBindTexture(GL_TEXTURE_2D, m_texture_id);
}

void RenderingTexture::unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}