// ========================================================================= //
// Puzzle Engine - 2D/3D Hybrid Game Engine
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// ========================================================================= //

#pragma once
#include "key_codes.h"
#include "core/mathematics/math_2d.h"
#include <array>

class Input {
private:
    static constexpr size_t KEY_COUNT = 512;
    static constexpr size_t MOUSE_BUTTON_COUNT = 8;

    // Estados de teclado
    static std::array<bool, KEY_COUNT> current_keys;
    static std::array<bool, KEY_COUNT> previous_keys;

    // Estados de ratón
    static std::array<bool, MOUSE_BUTTON_COUNT> current_mouse_buttons;
    static std::array<bool, MOUSE_BUTTON_COUNT> previous_mouse_buttons;

    static Vector2 mouse_position;
    static Vector2 previous_mouse_position;
    static Vector2 mouse_scroll_offset;

public:
    // Ciclo de actualización por fotograma (limpia estados transitorios)
    static void update();

    // Consultas de Teclado
    static bool is_key_pressed(Key p_key);
    static bool is_key_just_pressed(Key p_key);
    static bool is_key_just_released(Key p_key);

    // Consultas de Ratón
    static bool is_mouse_button_pressed(MouseButton p_button);
    static bool is_mouse_button_just_pressed(MouseButton p_button);
    static bool is_mouse_button_just_released(MouseButton p_button);

    static Vector2 get_mouse_position();
    static Vector2 get_mouse_delta();
    static Vector2 get_mouse_scroll();

    // Callbacks internos llamados por la ventana/backend
    static void on_key_event(int p_key, int p_action);
    static void on_mouse_button_event(int p_button, int p_action);
    static void on_mouse_moved_event(double p_xpos, double p_ypos);
    static void on_mouse_scrolled_event(double p_xoffset, double p_yoffset);
};