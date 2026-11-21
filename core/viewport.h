// ========================================================================= //
// PUZZLE ENGINE - 2D/3D GAME ENGINE
// ------------------------------------------------------------------------- //
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// ========================================================================= //

#pragma once

#include "core/mathematics/math_2d.h"
#include "color.h"

class RenderingDevice;
class Camera2D;

class Viewport {
public:
    // Equivalentes a Display -> Window -> Stretch en Godot
    enum class StretchMode {
        Disabled,    // Sin escalado: la resolución lógica coincide con la física
        CanvasItems, // Escala la proyección ortográfica suavemente manteniendo nitidez
        Viewport     // Escala sobre la resolución base del viewport
    };

    enum class StretchAspect {
        Ignore,     // Estira para llenar todo el espacio ignorando relación de aspecto
        Keep,       // Mantiene relación de aspecto agregando barras negras (letterbox)
        KeepWidth,  // Fija el ancho base y expande/recorta el alto
        KeepHeight, // Fija el alto base y expande/recorta el ancho
        Expand      // Mantiene proporción y expande el área visible sin barras negras
    };

    enum class StretchScaleMode {
        Fractional, // Permite escalas decimales (ej. 1.35x)
        Integer     // Redondea la escala a múltiplos enteros (1x, 2x, 3x — ideal para pixel art)
    };

    Viewport() = default;
    explicit Viewport(const Rect2& p_screen_rect);

    // Área física disponible dentro de la ventana del SO
    void set_screen_rect(const Rect2& p_rect);
    const Rect2& get_screen_rect() const { return m_screen_rect; }

    // Rectángulo físico final tras aplicar Aspect (ej. con letterboxing en modo Keep)
    const Rect2& get_render_rect() const { return m_render_rect; }

    // Resolución base de diseño (ej. 1280x720)
    void set_base_size(const Size2i& p_size);
    Size2i get_base_size() const { return m_base_size; }

    // Configuración de Stretch
    void set_stretch_mode(StretchMode p_mode);
    StretchMode get_stretch_mode() const { return m_stretch_mode; }

    void set_stretch_aspect(StretchAspect p_aspect);
    StretchAspect get_stretch_aspect() const { return m_stretch_aspect; }

    void set_stretch_scale(float p_scale);
    float get_stretch_scale() const { return m_stretch_scale; }

    void set_stretch_scale_mode(StretchScaleMode p_mode);
    StretchScaleMode get_stretch_scale_mode() const { return m_stretch_scale_mode; }

    // Tamaño lógico útil para la escena y la UI (el que usa la proyección ortográfica)
    float get_width() const { return m_logical_size.x; }
    float get_height() const { return m_logical_size.y; }
    Vector2 get_logical_size() const { return m_logical_size; }
    Size2i get_size() const {
        return Size2i(static_cast<int>(m_logical_size.x), static_cast<int>(m_logical_size.y));
    }

    // Gestión de cámara activa
    void set_current_camera(Camera2D* p_camera) { m_current_camera = p_camera; }
    Camera2D* get_current_camera() const { return m_current_camera; }

    // Activa glViewport, glScissor y la proyección ortográfica adaptada
    void bind(RenderingDevice* p_device, int p_total_window_height);
    void unbind(RenderingDevice* p_device = nullptr);

    // Transforma coordenadas de ventana física -> coordenadas lógicas del Viewport (para Input/UI)
    Vector2 to_local(const Vector2& p_screen_pos) const;

    // Transforma entre coordenadas locales del Viewport y coordenadas de mundo (Camera2D)
    Vector2 to_world_coordinates(const Vector2& p_local_pos) const;
    Vector2 to_local_coordinates(const Vector2& p_world_pos) const;

    bool contains_screen_point(const Vector2& p_screen_pos) const {
        return m_render_rect.has_point(p_screen_pos);
    }

private:
    void _recalculate_layout();

    Rect2 m_screen_rect;
    Rect2 m_render_rect;
    Size2i m_base_size = Size2i(0, 0);
    Vector2 m_logical_size = Vector2(0.0f, 0.0f);

    StretchMode m_stretch_mode = StretchMode::Disabled;
    StretchAspect m_stretch_aspect = StretchAspect::Keep;
    float m_stretch_scale = 1.0f;
    StretchScaleMode m_stretch_scale_mode = StretchScaleMode::Fractional;

    Camera2D* m_current_camera = nullptr;
};