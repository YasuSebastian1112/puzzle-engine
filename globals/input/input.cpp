// =============================================================================
// PUZZLE ENGINE — 2D/3D GAME ENGINE
// -----------------------------------------------------------------------------
// Copyright (c) 2026 Sebastian Alonso Sanchez (Billilol / Yatagarasu)
// All rights reserved.
// =============================================================================

#include "input.h"

std::array<bool, Input::KEY_COUNT> Input::current_keys = { false };
std::array<bool, Input::KEY_COUNT> Input::previous_keys = { false };

std::array<bool, Input::MOUSE_BUTTON_COUNT> Input::current_mouse_buttons = { false };
std::array<bool, Input::MOUSE_BUTTON_COUNT> Input::previous_mouse_buttons = { false };

std::array<Vector2, Input::MOUSE_BUTTON_COUNT> Input::mouse_press_positions = {
    Vector2(0.0f, 0.0f), Vector2(0.0f, 0.0f), Vector2(0.0f, 0.0f),
    Vector2(0.0f, 0.0f), Vector2(0.0f, 0.0f), Vector2(0.0f, 0.0f),
    Vector2(0.0f, 0.0f), Vector2(0.0f, 0.0f)
};

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
    return (idx < KEY_COUNT) ? current_keys[idx] : false;
}

bool Input::is_key_just_pressed(Key p_key) {
    uint16_t idx = static_cast<uint16_t>(p_key);
    return (idx < KEY_COUNT) ? (current_keys[idx] && !previous_keys[idx]) : false;
}

bool Input::is_key_just_released(Key p_key) {
    uint16_t idx = static_cast<uint16_t>(p_key);
    return (idx < KEY_COUNT) ? (!current_keys[idx] && previous_keys[idx]) : false;
}

bool Input::is_mouse_button_pressed(MouseButton p_button) {
    uint8_t idx = static_cast<uint8_t>(p_button);
    return (idx < MOUSE_BUTTON_COUNT) ? current_mouse_buttons[idx] : false;
}

bool Input::is_mouse_button_just_pressed(MouseButton p_button) {
    uint8_t idx = static_cast<uint8_t>(p_button);
    return (idx < MOUSE_BUTTON_COUNT) ? (current_mouse_buttons[idx] && !previous_mouse_buttons[idx]) : false;
}

bool Input::is_mouse_button_just_released(MouseButton p_button) {
    uint8_t idx = static_cast<uint8_t>(p_button);
    return (idx < MOUSE_BUTTON_COUNT) ? (!current_mouse_buttons[idx] && previous_mouse_buttons[idx]) : false;
}

Vector2 Input::get_mouse_position() {
    return mouse_position;
}

Vector2 Input::get_mouse_delta() {
    return mouse_position - previous_mouse_position;
}

Vector2 Input::get_mouse_direction() {
    Vector2 delta = get_mouse_delta();
    if (delta.length_squared() <= 0.00001f) {
        return Vector2(0.0f, 0.0f);
    }
    return delta.normalized();
}

Vector2 Input::get_mouse_scroll() {
    return mouse_scroll_offset;
}

bool Input::is_mouse_dragging(MouseButton p_button) {
    uint8_t idx = static_cast<uint8_t>(p_button);
    if (idx >= MOUSE_BUTTON_COUNT || !current_mouse_buttons[idx]) return false;

    float dist_sq = (mouse_position - mouse_press_positions[idx]).length_squared();
    return dist_sq >= (DRAG_THRESHOLD * DRAG_THRESHOLD);
}

Vector2 Input::get_mouse_drag_origin(MouseButton p_button) {
    uint8_t idx = static_cast<uint8_t>(p_button);
    return (idx < MOUSE_BUTTON_COUNT) ? mouse_press_positions[idx] : Vector2(0.0f, 0.0f);
}

Vector2 Input::get_mouse_drag_delta(MouseButton p_button) {
    if (!is_mouse_dragging(p_button)) return Vector2(0.0f, 0.0f);
    uint8_t idx = static_cast<uint8_t>(p_button);
    return mouse_position - mouse_press_positions[idx];
}

void Input::on_key_event(int p_key, int p_action) {
    if (p_key < 0 || static_cast<size_t>(p_key) >= KEY_COUNT) return;
    if (p_action == 1) current_keys[p_key] = true;
    else if (p_action == 0) current_keys[p_key] = false;
}

void Input::on_mouse_button_event(int p_button, int p_action) {
    if (p_button < 0 || static_cast<size_t>(p_button) >= MOUSE_BUTTON_COUNT) return;

    if (p_action == 1) { // GLFW_PRESS
        current_mouse_buttons[p_button] = true;
        mouse_press_positions[p_button] = mouse_position;
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