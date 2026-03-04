#include "Player.hpp"

#include <iostream>

// Detailed, annotated Player implementation.
// Responsibilities:
// - Handle per-frame movement input and collision response
// - Manage sprite animation and facing direction
// - Provide interaction probe for nearby objects
// - Render battle cursor when in battle menus

namespace {
// Constants describing player/tile geometry. Kept local to this file.
constexpr float kPlayerSize = 32.0f;      // On-screen size of the player sprite
constexpr float kProbeSize = 32.0f;       // Size of the interaction probe
constexpr float kProbeDistance = 24.0f;   // How far the probe sits from player
}

// Constructor: loads the player sprite and initializes collision box.
Player::Player(SDL_Renderer* renderer, const char* texturePath)
{
    sprite = addComponent<SpriteComponent>();
    if (!sprite->loadSprite(renderer, texturePath, kPlayerSize, kPlayerSize))
    {
        std::cerr << "Player sprite load failed for '" << texturePath << "'\n";
        // Leave object in a safe-but-uninitialized state; callers should
        // ensure a valid sprite exists before relying on rendering.
        return;
    }

    // Position sprite at spawn and set initial source frame (idle facing down).
    sprite->setPosition(spawnX, spawnY);
    sprite->setSourceRect(32, 0, 32, 32); // Idle/down in the middle column.

    // Collision box corresponds to sprite footprint.
    setCollisionBox(SDL_FRect{spawnX, spawnY, kPlayerSize, kPlayerSize});
}

// Set the desired directional inputs for this frame. Input collection is
// handled externally; Player consumes the boolean state each update.
void Player::setMovementInput(bool up, bool down, bool left, bool right)
{
    wantsUp = up;
    wantsDown = down;
    wantsLeft = left;
    wantsRight = right;
}

// Configure the room bounds and blocking volumes used for collision checks.
void Player::setCollisionEnvironment(const SDL_FRect& bounds, const std::vector<SDL_FRect>& blockers)
{
    roomBounds = bounds;
    blockingVolumes = blockers;
}

// Define where the player should respawn (used after exiting battles).
void Player::setSpawnPoint(float x, float y)
{
    spawnX = x;
    spawnY = y;
}

// Teleport player back to spawn (used on battle exit or map transitions).
void Player::resetToSpawn()
{
    if (!sprite)
        return;

    sprite->setPosition(spawnX, spawnY);
    setCollisionBox(SDL_FRect{spawnX, spawnY, kPlayerSize, kPlayerSize});
}

// Build an axis-aligned box in front of the player used to detect interactables.
SDL_FRect Player::getInteractionProbe() const
{
    SDL_FRect probe = getCollisionBox();
    probe.w = kProbeSize;
    probe.h = kProbeSize;

    // Move the probe in the facing direction.
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

// Render a small player sprite as a cursor above the selected battle option.
void Player::renderBattleCursor(int optionIndex, const SDL_FRect& battleOptionsArea)
{
    if (!sprite)
        return;

    // Three horizontal options in the bottom panel. Slot width determined by area.
    const float slotWidth = battleOptionsArea.w / 3.0f;
    SDL_FRect cursorDest{};
    cursorDest.w = kPlayerSize;
    cursorDest.h = kPlayerSize;
    cursorDest.x = battleOptionsArea.x + (slotWidth * static_cast<float>(optionIndex)) + 8.0f;
    cursorDest.y = battleOptionsArea.y + 8.0f;

    sprite->drawAt(cursorDest);
}

// Award or remove the fishing pole flag for the player.
void Player::setFishinpole(bool enabled)
{
    fishinpole = enabled;
}

bool Player::hasFishinpole() const
{
    return fishinpole;
}

// Main per-frame update: applies movement input, collision, and animation.
void Player::update(InputHandler&, float deltaTime)
{
    if (!sprite)
        return;

    // Compute desired displacement from held input. Diagonal movement is
    // allowed and will be resolved axis-by-axis for simple collision.
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

    // Apply movement separately for X and Y to simplify collision resolution.
    if (dx != 0.0f)
        moveBy(dx, 0.0f);
    if (dy != 0.0f)
        moveBy(0.0f, dy);

    // Update animation state and any attached components.
    updateAnimation(deltaTime, movedThisFrame);
    updateComponents(deltaTime);
}

// Render hook defers to attached components (e.g., SpriteComponent).
void Player::render(SDL_Renderer* renderer)
{
    renderComponents(renderer);
    (void)renderer;
}

// Attempt to move the player by (dx,dy). Movement is clamped to room bounds
// and rejected if it would overlap any blocking volume.
void Player::moveBy(float dx, float dy)
{
    if (!sprite)
        return;

    SDL_FRect current = getCollisionBox();
    SDL_FRect candidate = current;
    candidate.x += dx;
    candidate.y += dy;

    // Clamp candidate into room bounds so player can't leave the navigable area.
    if (candidate.x < roomBounds.x)
        candidate.x = roomBounds.x;
    if (candidate.y < roomBounds.y)
        candidate.y = roomBounds.y;
    if (candidate.x + candidate.w > roomBounds.x + roomBounds.w)
        candidate.x = roomBounds.x + roomBounds.w - candidate.w;
    if (candidate.y + candidate.h > roomBounds.y + roomBounds.h)
        candidate.y = roomBounds.y + roomBounds.h - candidate.h;

    // If the candidate collides with any blocker, cancel movement.
    if (collidesWithAny(candidate))
        return;

    // Commit movement: update collision box and sprite position.
    setCollisionBox(candidate);
    sprite->setPosition(candidate.x, candidate.y);
}

// Update facing direction; kept separate to allow future facing transitions.
void Player::setFacing(FacingDirection newFacing)
{
    facing = newFacing;
}

// Update sprite frame based on movement state and facing direction.
// Uses a simple two-frame walking toggle for motion and a single idle frame.
void Player::updateAnimation(float deltaTime, bool movedThisFrame)
{
    if (!sprite)
        return;

    if (!movedThisFrame)
    {
        // Idle: middle column (
        animationColumn = 1;
        animationTimer = 0.0f;
    }
    else
    {
        // While moving, toggle between transition columns at a fixed rate.
        animationTimer += deltaTime;
        if (animationTimer >= 0.14f)
        {
            animationTimer = 0.0f;
            animationColumn = (animationColumn == 0) ? 2 : 0;
        }
    }

    // Determine row from facing direction and update source rect.
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

// Check candidate rectangle against all blocking volumes.
bool Player::collidesWithAny(const SDL_FRect& candidate) const
{
    for (const SDL_FRect& blocker : blockingVolumes)
    {
        if (intersects(candidate, blocker))
            return true;
    }
    return false;
}

// AABB intersection test used for collision checks.
bool Player::intersects(const SDL_FRect& a, const SDL_FRect& b)
{
    return (a.x < b.x + b.w &&
            a.x + a.w > b.x &&
            a.y < b.y + b.h &&
            a.y + a.h > b.y);
}
