#include "Box.hpp"

#include <iostream>

// Constructor: creates sprite component and sets up collision box.
Box::Box(
    SDL_Renderer* renderer,
    float x,
    float y,
    const SDL_FRect& sourceFrame,
    const std::string& firstInteractText,
    const std::string& repeatInteractText)
    : firstInteractText(firstInteractText),
      repeatInteractText(repeatInteractText)
{
    // Create sprite component for this box and load from sprite sheet.
    sprite = addComponent<SpriteComponent>();
    if (sprite->loadSprite(renderer, "sprite.png", 32.0f, 32.0f))
    {
        // Set the source rectangle to display the correct sprite sheet tile.
        sprite->setSourceRect(
            static_cast<int>(sourceFrame.x),
            static_cast<int>(sourceFrame.y),
            static_cast<int>(sourceFrame.w),
            static_cast<int>(sourceFrame.h));
        sprite->setPosition(x, y);
    }

    // Define collision box for this box (one standard 32x32 tile).
    setCollisionBox(SDL_FRect{x, y, 32.0f, 32.0f});
}

// Per-frame update: propagate to components.
void Box::update(InputHandler&, float deltaTime)
{
    // Box has no continuous logic; just update any attached components.
    updateComponents(deltaTime);
}

// Per-frame render: propagate to components.
void Box::render(SDL_Renderer* renderer)
{
    renderComponents(renderer);
    (void)renderer;
}

// Interaction handler: sets dialogue text based on interaction state.
void Box::onInteract()
{
    // On first interaction, show the firstInteractText.
    if (!opened)
    {
        currentInteractText = firstInteractText;
        opened = true;
        return;
    }

    // On subsequent interactions, show repeat text (or cycle back to first).
    if (!repeatInteractText.empty())
    {
        currentInteractText = repeatInteractText;
    }
    else
    {
        currentInteractText = firstInteractText;
    }
}

// Query method: is this box opened?
bool Box::isOpened() const
{
    return opened;
}

// Query method: get the current interaction text.
const std::string& Box::getInteractText() const
{
    return currentInteractText;
}
