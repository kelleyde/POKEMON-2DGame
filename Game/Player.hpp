#pragma once
#include "GameObject.hpp"
#include "components.hpp"
#include <vector>

// Player handles movement input, animation frame selection, collision,
// and interaction probe generation.
class Player : public GameObject {
public:
    Player(SDL_Renderer* renderer, const char* texturePath);

    // Apply latest input/collision settings and move player.
    void update(InputHandler& input, float deltaTime) override;

    // Draw the player in-world.
    void render(SDL_Renderer* renderer) override;

    // Set current movement buttons from input.
    void setMovementInput(bool up, bool down, bool left, bool right);

    // Update room bounds + blocking collision volumes.
    void setCollisionEnvironment(const SDL_FRect& roomBounds, const std::vector<SDL_FRect>& blockers);

    // Define spawn location used when leaving battle.
    void setSpawnPoint(float x, float y);

    // Move player back to spawn location.
    void resetToSpawn();

    // Box in front of player used for interaction checks.
    SDL_FRect getInteractionProbe() const;
    
    // Render a small player cursor over battle menu options.
    void renderBattleCursor(int optionIndex, const SDL_FRect& battleOptionsArea);

    // Flag set after opening box and receiving the fishing pole.
    void setFishinpole(bool enabled);
    bool hasFishinpole() const;

private:
    enum class FacingDirection {
        Down,
        Left,
        Right,
        Up
    };

    void moveBy(float dx, float dy);
    void setFacing(FacingDirection newFacing);
    void updateAnimation(float deltaTime, bool movedThisFrame);
    bool collidesWithAny(const SDL_FRect& candidate) const;
    static bool intersects(const SDL_FRect& a, const SDL_FRect& b);

    SpriteComponent* sprite = nullptr;

    // Input state for this frame.
    bool wantsUp = false;
    bool wantsDown = false;
    bool wantsLeft = false;
    bool wantsRight = false;

    // World/collision environment.
    SDL_FRect roomBounds{32.0f, 32.0f, 736.0f, 536.0f};
    std::vector<SDL_FRect> blockingVolumes;

    // Spawn location.
    float spawnX = 96.0f;
    float spawnY = 96.0f;

    // Movement/animation.
    FacingDirection facing = FacingDirection::Down;
    float pixelsPerSecond = 140.0f;
    float animationTimer = 0.0f;
    int animationColumn = 1; // 0=transition, 1=idle, 2=transition
    bool fishinpole = false;
};
