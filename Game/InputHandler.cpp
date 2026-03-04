#include "InputHandler.hpp"

// Poll all SDL events for this frame and update keyboard state.
// Must be called once per game loop iteration to maintain accurate input state.
void InputHandler::update(bool& running)
{
    // Reusable local event object for SDL_PollEvent().
    SDL_Event event;
    
    // Clear previous frame's one-shot key press events.
    keyDownEvents.clear();

    // Consume all queued OS/SDL events for this frame.
    while (SDL_PollEvent(&event))
    {
        // Check for window close or OS quit request.
        if (event.type == SDL_EVENT_QUIT)
        {
            running = false;
        }

        // Record key-down events for one-shot action checks.
        if (event.type == SDL_EVENT_KEY_DOWN)
        {
            keyDownEvents.push_back(event);
        }
    }

    // Snapshot current held-key state after processing all events.
    // SDL owns the memory; pointer is valid until next update() call.
    keyboardState = SDL_GetKeyboardState(nullptr);
}

// Query whether a key is currently held down.
// This checks the continuous keyboard state, not discrete events.
bool InputHandler::isKeyDown(SDL_Scancode key) const
{
    // Guard against null keyboard state before array access.
    return keyboardState && keyboardState[key];
}

// Query whether a key was pressed this frame (one-shot event).
// This checks the discrete key-down events collected this frame only.
bool InputHandler::wasKeyPressed(SDL_Keycode key) const
{
    // Iterate through this frame's key-down events looking for exact keycode match.
    for (const auto& event : keyDownEvents)
    {
        if (event.key.key == key)
        {
            return true;
        }
    }
    return false;
}
