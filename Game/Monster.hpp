#pragma once

#include "GameObject.hpp"
#include "components.hpp"

// MONSTER TYPES - BATTLE ENCOUNTERS
enum class MonsterType {
    // Slime,
    // Wisp,
    // Golem,
    Red,       // Red colored monster variant
    Blue,      // Blue colored monster variant
    Yellow     // Yellow colored monster variant
};

// MONSTER DEFINITION - STATIC MONSTER DATA
// Data structure for one monster type containing sprite information

struct MonsterDefinition {
    const char* displayName = "Unknown";    // Display name shown during battles
    const char* talkLine = "...";           // Dialogue when player talks to monster
    CombatStats stats{};                    // HP, attack, defense, speed values
    SDL_FRect sourceFrame{0.0f, 0.0f, 64.0f, 64.0f};  // Sprite sheet region
    float drawWidth = 128.0f;               // Display width on screen
    float drawHeight = 128.0f;              // Display height on screen
};

// MONSTER - BATTLE ENTITY
// - Sprite rendering from sprite sheet
// - Type-based definition system (data-driven)
// - Combat stats (HP, attack, defense, speed)
class Monster : public GameObject {
public:
    // Constructor: creates a monster of the specified type at given position.
    // Args:
    //   renderer: SDL renderer for sprite loading
    //   x, y: world position for rendering
    //   type: monster type (determines sprite frame and stats)
    Monster(SDL_Renderer* renderer, float x, float y, MonsterType type);

    // Per-frame update (currently just updates components).
    void update(InputHandler& input, float deltaTime) override;

    // Per-frame render.
    void render(SDL_Renderer* renderer) override;

    // Resolve monster type to its complete definition.
    // This is the central lookup point for monster data.
    static const MonsterDefinition& getDefinition(MonsterType type);

private:
    // Apply the current type's stats and sprite frame configuration.
    void applyDefinition();

    SpriteComponent* sprite = nullptr;      // Sprite rendering component
    MonsterType type = MonsterType::Red;    // Current monster type
};
