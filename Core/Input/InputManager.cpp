#include "InputManager.hpp"

/* ════════════════════════════════════════
   Frame
════════════════════════════════════════ */

void InputManager::begin_frame() {
    // salva estado anterior
    for (int i = 0; i < KEY_COUNT;   i++) prev_keys_[i]  = cur_keys_[i];
    for (int i = 0; i < MOUSE_COUNT; i++) prev_mouse_[i] = cur_mouse_[i];

    // delta do mouse
    mdx_ = mx_ - prev_mx_;
    mdy_ = my_ - prev_my_;
    prev_mx_ = mx_;
    prev_my_ = my_;

    scroll_ = 0;
}

/* ════════════════════════════════════════
   Teclado
════════════════════════════════════════ */

void InputManager::on_key(Key k, bool down) {
    cur_keys_[(int)k] = down;
}

bool InputManager::is_down(Key k) const {
    return cur_keys_[(int)k];
}

bool InputManager::is_pressed(Key k) const {
    return cur_keys_[(int)k] && !prev_keys_[(int)k];
}

bool InputManager::is_released(Key k) const {
    return !cur_keys_[(int)k] && prev_keys_[(int)k];
}

/* ════════════════════════════════════════
   Mouse
════════════════════════════════════════ */

void InputManager::on_mouse_move(float x, float y) {
    mx_ = x;
    my_ = y;
}

void InputManager::on_mouse_button(MouseButton b, bool down) {
    cur_mouse_[(int)b] = down;
}

void InputManager::on_scroll(float delta) {
    scroll_ = delta;
}

bool InputManager::mouse_down(MouseButton b) const {
    return cur_mouse_[(int)b];
}

bool InputManager::mouse_pressed(MouseButton b) const {
    return cur_mouse_[(int)b] && !prev_mouse_[(int)b];
}

/* ════════════════════════════════════════
   Gamepad
════════════════════════════════════════ */

void InputManager::on_axis(float lx, float ly, float rx, float ry) {
    axis_x_  = lx;
    axis_y_  = ly;
    axis_rx_ = rx;
    axis_ry_ = ry;
}
