#include "Box.hpp"

#include <iostream>

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
    // Draw one 32x32 interactable tile from the sprite sheet.
    sprite = addComponent<SpriteComponent>();
    if (sprite->loadSprite(renderer, "sprite.png", 32.0f, 32.0f))
    {
        sprite->setSourceRect(
            static_cast<int>(sourceFrame.x),
            static_cast<int>(sourceFrame.y),
            static_cast<int>(sourceFrame.w),
            static_cast<int>(sourceFrame.h));
        sprite->setPosition(x, y);
    }

    // Interactable occupies one tile-sized blocking volume.
    setCollisionBox(SDL_FRect{x, y, 32.0f, 32.0f});
}

void Box::update(InputHandler&, float deltaTime)
{
    // Box has no continuous logic yet; keep component updates for consistency.
    updateComponents(deltaTime);
}

void Box::render(SDL_Renderer* renderer)
{
    renderComponents(renderer);
    (void)renderer;
}

void Box::onInteract()
{
    // Set current dialogue text each time interaction happens.
    if (!opened)
    {
        currentInteractText = firstInteractText;
        opened = true;
        return;
    }

    if (!repeatInteractText.empty())
    {
        currentInteractText = repeatInteractText;
    }
    else
    {
        currentInteractText = firstInteractText;
    }
}

bool Box::isOpened() const
{
    return opened;
}

const std::string& Box::getInteractText() const
{
    return currentInteractText;
}
