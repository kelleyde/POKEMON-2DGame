#pragma once

#include <SDL3/SDL.h>
#include <vector>

// INPUT HANDLER - KEYBOARD INPUT POLLING AND STATE MANAGEMENT

class InputHandler {
public:

    // Poll all pending SDL events for this frame and refresh keyboard state.
    // Must be called once per game loop iteration.
    void update(bool& running);

    // Query whether a key is currently held down this frame.
    // Returns true continuously while the key is physically pressed.
    // Returns: true if key is currently held, false otherwise
    bool isKeyDown(SDL_Scancode key) const;

    // Query whether a key was pressed this frame (one-shot event).
    // Returns true only on the exact frame a key-down event is received.
    // Useful for menu navigation and one-time actions.
    // Returns: true if key-press event occurred this frame, false otherwise
    bool wasKeyPressed(SDL_Keycode key) const;

private:

    // Pointer to SDL-managed keyboard state array from SDL_GetKeyboardState().
    // Updated each frame; index with SDL_Scancode values.
    const bool* keyboardState = nullptr;
    
    // Key-down events recorded during this frame's SDL_PollEvent() loop.
    // Cleared at the start of each update() call.
    std::vector<SDL_Event> keyDownEvents;
};
