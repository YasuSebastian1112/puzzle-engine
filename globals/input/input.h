// =============================================================================
// PUZZLE ENGINE — 2D/3D GAME ENGINE
// -----------------------------------------------------------------------------
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// =============================================================================

#pragma once

#include "key_codes.h"
#include "core/mathematics/math_2d.h"
#include <array>

class Input {
public:
    static constexpr size_t KEY_COUNT = 512;
    static constexpr size_t MOUSE_BUTTON_COUNT = 8;

    // Ciclo de sincronización: se invoca al inicio de cada frame
    static void update();

    // Consultas de Teclado
    static bool is_key_pressed(Key p_key);
    static bool is_key_just_pressed(Key p_key);
    static bool is_key_just_released(Key p_key);

    // Consultas de Ratón
    static bool is_mouse_button_pressed(MouseButton p_button);
    static bool is_mouse_button_just_pressed(MouseButton p_button);
    static bool is_mouse_button_just_released(MouseButton p_button);

    // Posición, Desplazamiento y Dirección
    static Vector2 get_mouse_position();
    static Vector2 get_mouse_delta();
    static Vector2 get_mouse_direction(); // Vector normalizado hacia donde se mueve el cursor
    static Vector2 get_mouse_scroll();

    // Detección de Arrastre (Drag)
    static bool is_mouse_dragging(MouseButton p_button = MouseButton::Left);
    static Vector2 get_mouse_drag_origin(MouseButton p_button = MouseButton::Left);
    static Vector2 get_mouse_drag_delta(MouseButton p_button = MouseButton::Left);

    // Callbacks del backend (GLFW / SDL)
    static void on_key_event(int p_key, int p_action);
    static void on_mouse_button_event(int p_button, int p_action);
    static void on_mouse_moved_event(double p_xpos, double p_ypos);
    static void on_mouse_scrolled_event(double p_xoffset, double p_yoffset);

private:
    static std::array<bool, KEY_COUNT> current_keys;
    static std::array<bool, KEY_COUNT> previous_keys;

    static std::array<bool, MOUSE_BUTTON_COUNT> current_mouse_buttons;
    static std::array<bool, MOUSE_BUTTON_COUNT> previous_mouse_buttons;

    // Origen de posición cuando se presionó el botón para calcular drag
    static std::array<Vector2, MOUSE_BUTTON_COUNT> mouse_press_positions;

    static Vector2 mouse_position;
    static Vector2 previous_mouse_position;
    static Vector2 mouse_scroll_offset;

    static constexpr float DRAG_THRESHOLD = 3.0f; // Píxeles mínimos para considerarlo arrastre
};