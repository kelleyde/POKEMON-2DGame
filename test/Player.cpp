#include "Player.hpp"
#include <iostream>

Player::Player(SDL_Renderer* renderer, const char* texturePath)
{
    // Attach a sprite component to this GameObject.
    sprite = addComponent<SpriteComponent>();
    // Load sprite sheet. The texture is expected to contain:
    // frame 0 (x=0): left-facing
    // frame 1 (x=64): right-facing
    if (!sprite->loadSprite(renderer, texturePath, 64.0f, 64.0f))
    {
        std::cerr << "Player sprite load failed for '" << texturePath << "'\n";
        return;
    }
    // Initial player spawn position.
    sprite->setPosition(100.0f, 100.0f);
    // Start in left-facing frame.
    sprite->setSourceRect(0, 0, 64, 64);
}

void Player::update(InputHandler& input, float deltaTime)
{
    // Movement keys:
    // - arrows control position
    // - left/right also control facing animation frame
    // Note: A/B keys are reserved globally by Engine for confirm/cancel.
    if (input.isKeyDown(SDL_SCANCODE_UP))
        up(deltaTime);
    if (input.isKeyDown(SDL_SCANCODE_DOWN))
        down(deltaTime);
    if (input.isKeyDown(SDL_SCANCODE_LEFT))
        left(deltaTime);
    if (input.isKeyDown(SDL_SCANCODE_RIGHT))
        right(deltaTime);

    updateComponents(deltaTime);
}

void Player::render(SDL_Renderer* renderer)
{
    // Renderer argument is unused here because SpriteComponent stores
    // the renderer internally when loaded.
    (void)renderer;
    // Render all components owned by this object.
    renderComponents(renderer);
}

void Player::up(float deltaTime)
{
    // Guard against missing component in case loading failed.
    if (!sprite) return;
    // Move up by speed * elapsed time.
    sprite->moveBy(0.0f, -pps * deltaTime);
}

void Player::down(float deltaTime)
{
    if (!sprite) return;
    // Move down by speed * elapsed time.
    sprite->moveBy(0.0f, pps * deltaTime);
}

void Player::left(float deltaTime)
{
    if (!sprite) return;
    // Update facing direction + source frame in sprite sheet.
    facing = FacingDirection::Left;
    // Left-facing frame is first 64x64 cell.
    sprite->setSourceRect(0, 0, 64, 64);
    // Move left.
    sprite->moveBy(-pps * deltaTime, 0.0f);
}

void Player::right(float deltaTime)
{
    if (!sprite) return;
    // Update facing direction + source frame in sprite sheet.
    facing = FacingDirection::Right;
    // Right-facing frame is second 64x64 cell.
    sprite->setSourceRect(64, 0, 64, 64);
    // Move right.
    sprite->moveBy(pps * deltaTime, 0.0f);
}
