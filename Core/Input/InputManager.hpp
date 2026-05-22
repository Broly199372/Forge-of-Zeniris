#pragma once
#include <unordered_map>
#include <string>

/* ─────────────────────────────────────────
   Teclas — baseado em keycodes padrão
───────────────────────────────────────── */
enum class Key {
    // Movimento
    W, A, S, D,
    Up, Down, Left, Right,

    // Ação
    Space, Shift, Ctrl, Alt,
    Enter, Escape, Tab,

    // Números
    N0, N1, N2, N3, N4, N5, N6, N7, N8, N9,

    // Funções
    F1, F2, F3, F4, F5,

    COUNT // sempre último — tamanho do array
};


/* ─────────────────────────────────────────
   Botões do mouse
───────────────────────────────────────── */
enum class MouseButton { Left, Right, Middle, COUNT };


/* ─────────────────────────────────────────
   InputManager
───────────────────────────────────────── */
class InputManager {
public:

    /* ── Teclado ── */
    bool is_down(Key k)     const;   // segurado
    bool is_pressed(Key k)  const;   // só no primeiro frame
    bool is_released(Key k) const;   // só no frame que soltou

    /* ── Mouse ── */
    bool  mouse_down(MouseButton b)    const;
    bool  mouse_pressed(MouseButton b) const;
    float mouse_x()    const { return mx_; }
    float mouse_y()    const { return my_; }
    float mouse_dx()   const { return mdx_; }  // delta do frame
    float mouse_dy()   const { return mdy_; }
    float scroll()     const { return scroll_; }

    /* ── Gamepad (analógico) ── */
    float axis_x()     const { return axis_x_; }  // -1..1
    float axis_y()     const { return axis_y_; }
    float axis_rx()    const { return axis_rx_; } // analógico direito
    float axis_ry()    const { return axis_ry_; }

    /* ── Chamados pela engine a cada frame ── */
    void begin_frame();               // copia estado atual → anterior
    void on_key(Key k, bool down);
    void on_mouse_move(float x, float y);
    void on_mouse_button(MouseButton b, bool down);
    void on_scroll(float delta);
    void on_axis(float lx, float ly, float rx, float ry);

private:
    static constexpr int KEY_COUNT   = (int)Key::COUNT;
    static constexpr int MOUSE_COUNT = (int)MouseButton::COUNT;

    bool cur_keys_[KEY_COUNT]   = {};
    bool prev_keys_[KEY_COUNT]  = {};

    bool cur_mouse_[MOUSE_COUNT]  = {};
    bool prev_mouse_[MOUSE_COUNT] = {};

    float mx_=0, my_=0, mdx_=0, mdy_=0, scroll_=0;
    float prev_mx_=0, prev_my_=0;

    float axis_x_=0, axis_y_=0, axis_rx_=0, axis_ry_=0;
};
