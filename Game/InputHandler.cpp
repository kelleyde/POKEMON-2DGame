#include "InputHandler.hpp"

void InputHandler::update(bool& running)
{
    // Reused local event object used by SDL_PollEvent().
    SDL_Event event;
    
    // Start new frame with empty one-shot key press list.
    keyDownEvents.clear();

    // Consume all queued events for this frame.
    while (SDL_PollEvent(&event))
    {
        // Window close button or OS quit signal.
        if (event.type == SDL_EVENT_QUIT)
            running = false;

        // Capture key-down events for one-frame button actions.
        if (event.type == SDL_EVENT_KEY_DOWN)
            keyDownEvents.push_back(event);
    }

    // Snapshot held-key state after events are processed.
    // SDL owns the underlying memory.
    keyboardState = SDL_GetKeyboardState(nullptr);
}

bool InputHandler::isKeyDown(SDL_Scancode key) const
{
    // Guard against null keyboard state before indexing.
    return keyboardState && keyboardState[key];
}

bool InputHandler::wasKeyPressed(SDL_Keycode key) const
{
    // Check this frame's key-down events for an exact keycode match.
    for (const auto& event : keyDownEvents)
    {
        if (event.key.key == key)
            return true;
    }
    return false;
}
