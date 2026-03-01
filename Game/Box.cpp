#include "Box.hpp"

#include <iostream>

Box::Box(SDL_Renderer* renderer, float x, float y)
{
    // Use the world-object box frame from sprite.png:
    // segment starts at y=768, row 1 col 1, frame size 32x32.
    sprite = addComponent<SpriteComponent>();
    if (sprite->loadSprite(renderer, "sprite.png", 32.0f, 32.0f))
    {
        sprite->setSourceRect(0, 768, 32, 32);
        sprite->setPosition(x, y);
    }

    // Box occupies one tile-sized blocking volume.
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
    // Minimal framework behavior requested by spec.
    // Only logs once to avoid spamming every frame.
    if (!opened)
    {
        std::cout << "open\n";
        opened = true;
    }
}
