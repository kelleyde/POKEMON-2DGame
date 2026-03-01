#pragma once

#include "GameObject.hpp"
#include "components.hpp"

// Supported monster archetypes.
// Add new values here as you introduce more monster content.
enum class MonsterType {
    Slime,
    Wisp,
    Golem
};

// Data definition for one monster type.
// This is deliberately lightweight and easy to migrate to data files later.
struct MonsterDefinition {
    const char* displayName = "Unknown";
    CombatStats stats{};
    SDL_FRect sourceFrame{0.0f, 0.0f, 64.0f, 64.0f};
    float drawWidth = 128.0f;
    float drawHeight = 128.0f;
};

// Monster is a framework entity for battle mode.
// It currently uses a placeholder sprite frame and exposes stats through
// the GameObject base CombatStats storage.
class Monster : public GameObject {
public:
    // Construct a monster instance from a built-in monster type.
    Monster(SDL_Renderer* renderer, float x, float y, MonsterType type);

    void update(InputHandler& input, float deltaTime) override;
    void render(SDL_Renderer* renderer) override;

    // Runtime type switching support for evolution/forms/testing.
    void setType(MonsterType newType);
    MonsterType getType() const;
    const char* getDisplayName() const;

    // Resolve type -> definition. Central point for new monster data.
    static const MonsterDefinition& getDefinition(MonsterType type);

    // Battle monster should not block roaming movement.
    bool blocksMovement() const override
    {
        return false;
    }

private:
    // Apply the current type's stats + sprite frame.
    void applyDefinition();

    SpriteComponent* sprite = nullptr;
    MonsterType type = MonsterType::Slime;
};
