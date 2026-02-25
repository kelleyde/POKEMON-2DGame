#pragma once
#include "GameObject.hpp"
#include "components.hpp"

// Player is a controllable scene object with a single sprite texture.
class Player : public GameObject {
public:
    // Creates player and loads sprite-sheet texture.
    Player(SDL_Renderer* renderer, const char* texturePath);

    // Handles movement input and updates components.
    void update(InputHandler& input, float deltaTime) override;
    // Draws player components (currently SpriteComponent).
    void render(SDL_Renderer* renderer) override;

private:
    // Direction used to choose source frame in sprite sheet.
    enum class FacingDirection {
        Left,
        Right
    };

    // Movement helper functions (delta-time based).
    void up(float deltaTime);
    void down(float deltaTime);
    void left(float deltaTime);
    void right(float deltaTime);

    // Cached pointer to attached sprite component.
    SpriteComponent* sprite = nullptr;
    // Current facing direction for frame selection.
    FacingDirection facing = FacingDirection::Left;
    // Pixels per second movement speed (delta-time based).
    float pps = 180.0f;
};
