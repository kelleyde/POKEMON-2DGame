#pragma once

#include "GameObject.hpp"
#include "components.hpp"
#include <vector>

// PLAYER - PRIMARY CHARACTER ENTITY

// Sprite Sheet Layout:
//   Columns: 0 (transition frames), 1 (idle), 2 (transition frames)
//   Rows: Down=0, Left=1, Right=2, Up=3
class Player : public GameObject {
public:

    // Constructor: loads player sprite from texture path and initializes state.
    // Args:
    //   renderer: SDL renderer for sprite loading
    //   texturePath: file path to sprite sheet (e.g., "sprite.png")

    Player(SDL_Renderer* renderer, const char* texturePath);

    // Per-frame update: applies input, collision detection, and animation.
    void update(InputHandler& input, float deltaTime) override;

    // Per-frame render: draws player sprite at current position.
    void render(SDL_Renderer* renderer) override;

    // Set movement input state for this frame.
    // Input handling is external; player just applies queued movement.
    void setMovementInput(bool up, bool down, bool left, bool right);

    // Configure movement bounds and collision obstacles.
    // Called before each roaming session to set up collision environment.
    void setCollisionEnvironment(const SDL_FRect& roomBounds, const std::vector<SDL_FRect>& blockers);

    // Define the spawn location used when leaving battle.
    void setSpawnPoint(float x, float y);

    // Teleport player back to the spawn point.
    // Called when exiting battle to return player to encounter location.
    void resetToSpawn();

    // Get the interaction probe box in front of the player.
    // Used to detect nearby interactive objects.
    // Returns: collision rect in front of player (size and direction depend on facing)
    SDL_FRect getInteractionProbe() const;
    
    // Draw the player selection cursor over a battle menu option.
    // Args:
    //   optionIndex: 0=left, 1=center, 2=right option
    //   battleOptionsArea: screen rect of the battle menu area
    void renderBattleCursor(int optionIndex, const SDL_FRect& battleOptionsArea);

    // Set the fishing pole state (reward for opening the starting box).
    void setFishinpole(bool enabled);

    // Query whether player has obtained the fishing pole.
    bool hasFishinpole() const;

private:
    // Cardinal direction player is facing.
    enum class FacingDirection {
        Down,   // 0 - Facing downward
        Left,   // 1 - Facing leftward
        Right,  // 2 - Facing rightward
        Up      // 3 - Facing upward
    };

    // Internal movement: apply displacement with collision response.
    void moveBy(float dx, float dy);

    // Change facing direction and update sprite frame.
    void setFacing(FacingDirection newFacing);

    // Update animation frame based on movement state.
    void updateAnimation(float deltaTime, bool movedThisFrame);

    // Check if position collides with any blocking volume.
    bool collidesWithAny(const SDL_FRect& candidate) const;

    // Utility: axis-aligned rectangle intersection test.
    static bool intersects(const SDL_FRect& a, const SDL_FRect& b);

    SpriteComponent* sprite = nullptr;      // Sprite rendering component

    // Current-frame input state (set by external input handler).
    bool wantsUp = false;
    bool wantsDown = false;
    bool wantsLeft = false;
    bool wantsRight = false;

    // Movement collision environment.
    SDL_FRect roomBounds{32.0f, 32.0f, 736.0f, 536.0f};    // Navigable area
    std::vector<SDL_FRect> blockingVolumes;                 // Obstacles

    // Spawn location (set by Scene during roaming initialization).
    float spawnX = 96.0f;
    float spawnY = 96.0f;

    // Movement and animation state.
    FacingDirection facing = FacingDirection::Down;     // Current facing direction
    float pixelsPerSecond = 140.0f;                     // Movement speed
    float animationTimer = 0.0f;                        // For animation frame cycling
    int animationColumn = 1;                            // 0=transition, 1=idle, 2=transition
    bool fishinpole = false;                            // Has player obtained the fishing pole?
};
