#include "Player.hpp"

#include <iostream>

namespace {
constexpr float kPlayerSize = 32.0f;
constexpr float kProbeSize = 32.0f;
constexpr float kProbeDistance = 24.0f;
}

Player::Player(SDL_Renderer* renderer, const char* texturePath)
{
    sprite = addComponent<SpriteComponent>();
    if (!sprite->loadSprite(renderer, texturePath, kPlayerSize, kPlayerSize))
    {
        std::cerr << "Player sprite load failed for '" << texturePath << "'\n";
        return;
    }

    sprite->setPosition(spawnX, spawnY);
    sprite->setSourceRect(32, 0, 32, 32); // Idle/down in the middle column.

    setCollisionBox(SDL_FRect{spawnX, spawnY, kPlayerSize, kPlayerSize});
}

void Player::setMovementInput(bool up, bool down, bool left, bool right)
{
    wantsUp = up;
    wantsDown = down;
    wantsLeft = left;
    wantsRight = right;
}

void Player::setCollisionEnvironment(const SDL_FRect& bounds, const std::vector<SDL_FRect>& blockers)
{
    roomBounds = bounds;
    blockingVolumes = blockers;
}

void Player::setSpawnPoint(float x, float y)
{
    spawnX = x;
    spawnY = y;
}

void Player::resetToSpawn()
{
    if (!sprite)
    {
        return;
    }

    sprite->setPosition(spawnX, spawnY);
    setCollisionBox(SDL_FRect{spawnX, spawnY, kPlayerSize, kPlayerSize});
}

SDL_FRect Player::getInteractionProbe() const
{
    SDL_FRect probe = getCollisionBox();
    probe.w = kProbeSize;
    probe.h = kProbeSize;

    switch (facing)
    {
    case FacingDirection::Down:
        probe.y += kProbeDistance;
        break;
    case FacingDirection::Left:
        probe.x -= kProbeDistance;
        break;
    case FacingDirection::Right:
        probe.x += kProbeDistance;
        break;
    case FacingDirection::Up:
        probe.y -= kProbeDistance;
        break;
    }

    return probe;
}

void Player::renderBattleCursor(int optionIndex, const SDL_FRect& battleOptionsArea)
{
    if (!sprite)
    {
        return;
    }

    // Three horizontal options in the bottom panel.
    const float slotWidth = battleOptionsArea.w / 3.0f;
    SDL_FRect cursorDest{};
    cursorDest.w = kPlayerSize;
    cursorDest.h = kPlayerSize;
    cursorDest.x = battleOptionsArea.x + (slotWidth * static_cast<float>(optionIndex)) + 8.0f;
    cursorDest.y = battleOptionsArea.y + 8.0f;

    sprite->drawAt(cursorDest);
}

void Player::setFishinpole(bool enabled)
{
    fishinpole = enabled;
}

bool Player::hasFishinpole() const
{
    return fishinpole;
}

void Player::update(InputHandler&, float deltaTime)
{
    if (!sprite)
    {
        return;
    }

    float dx = 0.0f;
    float dy = 0.0f;

    if (wantsUp)
    {
        dy -= pixelsPerSecond * deltaTime;
        setFacing(FacingDirection::Up);
    }
    if (wantsDown)
    {
        dy += pixelsPerSecond * deltaTime;
        setFacing(FacingDirection::Down);
    }
    if (wantsLeft)
    {
        dx -= pixelsPerSecond * deltaTime;
        setFacing(FacingDirection::Left);
    }
    if (wantsRight)
    {
        dx += pixelsPerSecond * deltaTime;
        setFacing(FacingDirection::Right);
    }

    const bool movedThisFrame = (dx != 0.0f || dy != 0.0f);

    // Move on each axis separately to keep collision response simple.
    if (dx != 0.0f)
    {
        moveBy(dx, 0.0f);
    }
    if (dy != 0.0f)
    {
        moveBy(0.0f, dy);
    }

    updateAnimation(deltaTime, movedThisFrame);
    updateComponents(deltaTime);
}

void Player::render(SDL_Renderer* renderer)
{
    renderComponents(renderer);
    (void)renderer;
}

void Player::moveBy(float dx, float dy)
{
    if (!sprite)
    {
        return;
    }

    SDL_FRect current = getCollisionBox();
    SDL_FRect candidate = current;
    candidate.x += dx;
    candidate.y += dy;

    // Clamp to room bounds (32px border wall behavior).
    if (candidate.x < roomBounds.x)
        candidate.x = roomBounds.x;
    if (candidate.y < roomBounds.y)
        candidate.y = roomBounds.y;
    if (candidate.x + candidate.w > roomBounds.x + roomBounds.w)
        candidate.x = roomBounds.x + roomBounds.w - candidate.w;
    if (candidate.y + candidate.h > roomBounds.y + roomBounds.h)
        candidate.y = roomBounds.y + roomBounds.h - candidate.h;

    // Reject movement if resulting box overlaps a blocker.
    if (collidesWithAny(candidate))
    {
        return;
    }

    setCollisionBox(candidate);
    sprite->setPosition(candidate.x, candidate.y);
}

void Player::setFacing(FacingDirection newFacing)
{
    facing = newFacing;
}

void Player::updateAnimation(float deltaTime, bool movedThisFrame)
{
    if (!sprite)
    {
        return;
    }

    if (!movedThisFrame)
    {
        // Middle column is idle.
        animationColumn = 1;
        animationTimer = 0.0f;
    }
    else
    {
        // Toggle between side transition columns while moving.
        animationTimer += deltaTime;
        if (animationTimer >= 0.14f)
        {
            animationTimer = 0.0f;
            animationColumn = (animationColumn == 0) ? 2 : 0;
        }
    }

    int row = 0;
    switch (facing)
    {
    case FacingDirection::Down: row = 0; break;
    case FacingDirection::Left: row = 1; break;
    case FacingDirection::Right: row = 2; break;
    case FacingDirection::Up: row = 3; break;
    }
    const int srcX = animationColumn * 32;
    const int srcY = row * 32;
    sprite->setSourceRect(srcX, srcY, 32, 32);
}

bool Player::collidesWithAny(const SDL_FRect& candidate) const
{
    for (const SDL_FRect& blocker : blockingVolumes)
    {
        if (intersects(candidate, blocker))
        {
            return true;
        }
    }
    return false;
}

bool Player::intersects(const SDL_FRect& a, const SDL_FRect& b)
{
    return (a.x < b.x + b.w &&
            a.x + a.w > b.x &&
            a.y < b.y + b.h &&
            a.y + a.h > b.y);
}
