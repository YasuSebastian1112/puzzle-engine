// ========================================================================= //
// Puzzle Engine - 2D/3D Hybrid Game Engine
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// ========================================================================= //

#include "input.h"

// Inicialización de variables estáticas
std::array<bool, Input::KEY_COUNT> Input::current_keys = { false };
std::array<bool, Input::KEY_COUNT> Input::previous_keys = { false };

std::array<bool, Input::MOUSE_BUTTON_COUNT> Input::current_mouse_buttons = { false };
std::array<bool, Input::MOUSE_BUTTON_COUNT> Input::previous_mouse_buttons = { false };

Vector2 Input::mouse_position = Vector2(0.0f, 0.0f);
Vector2 Input::previous_mouse_position = Vector2(0.0f, 0.0f);
Vector2 Input::mouse_scroll_offset = Vector2(0.0f, 0.0f);

void Input::update() {
    previous_keys = current_keys;
    previous_mouse_buttons = current_mouse_buttons;
    previous_mouse_position = mouse_position;
    mouse_scroll_offset = Vector2(0.0f, 0.0f);
}

bool Input::is_key_pressed(Key p_key) {
    uint16_t idx = static_cast<uint16_t>(p_key);
    if (idx >= KEY_COUNT) return false;
    return current_keys[idx];
}

bool Input::is_key_just_pressed(Key p_key) {
    uint16_t idx = static_cast<uint16_t>(p_key);
    if (idx >= KEY_COUNT) return false;
    return current_keys[idx] && !previous_keys[idx];
}

bool Input::is_key_just_released(Key p_key) {
    uint16_t idx = static_cast<uint16_t>(p_key);
    if (idx >= KEY_COUNT) return false;
    return !current_keys[idx] && previous_keys[idx];
}

bool Input::is_mouse_button_pressed(MouseButton p_button) {
    uint8_t idx = static_cast<uint8_t>(p_button);
    if (idx >= MOUSE_BUTTON_COUNT) return false;
    return current_mouse_buttons[idx];
}

bool Input::is_mouse_button_just_pressed(MouseButton p_button) {
    uint8_t idx = static_cast<uint8_t>(p_button);
    if (idx >= MOUSE_BUTTON_COUNT) return false;
    return current_mouse_buttons[idx] && !previous_mouse_buttons[idx];
}

bool Input::is_mouse_button_just_released(MouseButton p_button) {
    uint8_t idx = static_cast<uint8_t>(p_button);
    if (idx >= MOUSE_BUTTON_COUNT) return false;
    return !current_mouse_buttons[idx] && previous_mouse_buttons[idx];
}

Vector2 Input::get_mouse_position() {
    return mouse_position;
}

Vector2 Input::get_mouse_delta() {
    return mouse_position - previous_mouse_position;
}

Vector2 Input::get_mouse_scroll() {
    return mouse_scroll_offset;
}

void Input::on_key_event(int p_key, int p_action) {
    if (p_key < 0 || static_cast<size_t>(p_key) >= KEY_COUNT) return;

    if (p_action == 1) { // GLFW_PRESS
        current_keys[p_key] = true;
    } else if (p_action == 0) { // GLFW_RELEASE
        current_keys[p_key] = false;
    }
}

void Input::on_mouse_button_event(int p_button, int p_action) {
    if (p_button < 0 || static_cast<size_t>(p_button) >= MOUSE_BUTTON_COUNT) return;

    if (p_action == 1) { // GLFW_PRESS
        current_mouse_buttons[p_button] = true;
    } else if (p_action == 0) { // GLFW_RELEASE
        current_mouse_buttons[p_button] = false;
    }
}

void Input::on_mouse_moved_event(double p_xpos, double p_ypos) {
    mouse_position = Vector2(static_cast<float>(p_xpos), static_cast<float>(p_ypos));
}

void Input::on_mouse_scrolled_event(double p_xoffset, double p_yoffset) {
    mouse_scroll_offset = Vector2(static_cast<float>(p_xoffset), static_cast<float>(p_yoffset));
}