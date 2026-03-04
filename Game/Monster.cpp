#include "Monster.hpp"

// MONSTER DEFINITION BUILDER
// Helper function to construct MonsterDefinition structures with sprite and stat
// information. This centralizes monster data assembly.
namespace {

// Builds a complete monster definition from sprite and stat parameters.
// Args:
//   name: display name for the monster
//   srcX, srcY: sprite sheet position (pixels)
//   srcW, srcH: sprite sheet source dimensions (pixels)
//   drawW, drawH: on-screen display dimensions (pixels)
// Returns: fully initialized MonsterDefinition struct

MonsterDefinition makeDefinition(
    const char* name,
    float srcX,
    float srcY,
    float srcW,
    float srcH,
    float drawW,
    float drawH)
{
    MonsterDefinition def{};
    def.displayName = name;

    // def.stats.maxHP = hp;
    // def.stats.currentHP = hp;
    // def.stats.attack = attack;
    // def.stats.defense = defense;
    // def.stats.speed = speed;

    def.sourceFrame = SDL_FRect{srcX, srcY, srcW, srcH};
    def.drawWidth = drawW;
    def.drawHeight = drawH;
    return def;
}

}  // End anonymous namespace

// Constructor: initializes sprite and applies type definition.
Monster::Monster(SDL_Renderer* renderer, float x, float y, MonsterType type)
    : type(type)
{
    // Create sprite component for this monster.
    sprite = addComponent<SpriteComponent>();

    // Get the definition for this monster type.
    const MonsterDefinition& def = getDefinition(type);
    
    // Load sprite and set initial frame from definition.
    if (sprite->loadSprite(renderer, "sprite.png", def.drawWidth, def.drawHeight))
    {
        sprite->setSourceRect(
            static_cast<int>(def.sourceFrame.x),
            static_cast<int>(def.sourceFrame.y),
            static_cast<int>(def.sourceFrame.w),
            static_cast<int>(def.sourceFrame.h));
        sprite->setPosition(x, y);
    }

    // Sync all stats and sprite settings from the definition.
    applyDefinition();
}

// Per-frame update: propagate to components.
void Monster::update(InputHandler&, float deltaTime)
{
    // Monster currently has no AI behavior; just update components.
    updateComponents(deltaTime);
}

// Per-frame render: propagate to components.
void Monster::render(SDL_Renderer* renderer)
{
    renderComponents(renderer);
    (void)renderer;
}

// Static method: resolve monster type to its complete data definition.
// Uses static storage to avoid repeated allocations.
// This is the central lookup point for all monster data.
const MonsterDefinition& Monster::getDefinition(MonsterType type)
{
    // Monster atlas layout in sprite.png:
    // - Red monster:    y = 512, size = 96x128
    // - Blue monster:   y = 256, size = 96x128
    // - Yellow monster: y = 384, size = 96x128

    // Define Red monster - largest/strongest archetype.
    static const MonsterDefinition red = makeDefinition(
        "Red",

        0.0f, 512.0f, 96.0f, 128.0f,
        192.0f, 256.0f);

    // Define Blue monster - balanced archetype.
    static const MonsterDefinition blue = makeDefinition(
        "Blue",
        0.0f, 256.0f, 96.0f, 128.0f,
        192.0f, 256.0f);

    // Define Yellow monster - glass cannon archetype.
    static const MonsterDefinition yellow = makeDefinition(
        "Yellow",
        0.0f, 384.0f, 96.0f, 128.0f,
        192.0f, 256.0f);

    // Return the appropriate definition based on type.
    switch (type)
    {
    case MonsterType::Blue:
        return blue;
    case MonsterType::Red:
        return red;
    case MonsterType::Yellow:
        return yellow;
    default:
        return blue;
    }
}

// Apply the current type's definition to this monster instance.
void Monster::applyDefinition()
{
    // Sync combat stats with the type definition.
    const MonsterDefinition& def = getDefinition(type);
    //getStats() = def.stats;

    // Update sprite dimensions and frame if sprite exists.
    if (sprite)
    {
        sprite->setSize(def.drawWidth, def.drawHeight);
        sprite->setSourceRect(
            static_cast<int>(def.sourceFrame.x),
            static_cast<int>(def.sourceFrame.y),
            static_cast<int>(def.sourceFrame.w),
            static_cast<int>(def.sourceFrame.h));
    }
}
