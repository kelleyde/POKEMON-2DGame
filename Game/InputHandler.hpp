#pragma once
#include <SDL3/SDL.h>
#include <vector>

// Polls SDL events and exposes current keyboard button states.
class InputHandler {
public:
    // Poll all pending SDL events, handle quit, cache key events,
    // and refresh the keyboard state snapshot for held-key checks.
    void update(bool& running);
    // Returns true while a key is held down this frame.
    bool isKeyDown(SDL_Scancode key) const;
    // Returns true only on the frame a key-down event is received.
    // Useful for one-shot actions (toggle states, confirm/cancel).
    bool wasKeyPressed(SDL_Keycode key) const;

private:
    // Pointer to SDL-managed keyboard state array from SDL_GetKeyboardState.
    const bool* keyboardState = nullptr;
    // Key-down events copied during this frame's poll pass.
    std::vector<SDL_Event> keyDownEvents;

    // store a pointer to each command
    Command* up;
    Command* down;
    Command* right;
    Command* left;
    Command* selecta;

};
