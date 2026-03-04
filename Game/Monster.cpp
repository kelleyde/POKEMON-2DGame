#include "Monster.hpp"


namespace {
MonsterDefinition makeDefinition(
    const char* name,

    //these have no affect, emplement later
    //int hp,
    //int attack,
    //int defense,
    //int speed,
    float srcX,
    float srcY,
    float srcW,
    float srcH,
    float drawW,
    float drawH)
{
    MonsterDefinition def{};
   def.displayName = name;

   //these have no affect, emplement later
    //def.stats.maxHP = hp;
    // def.stats.currentHP = hp;
    // def.stats.attack = attack;
   // def.stats.defense = defense;
  //  def.stats.speed = speed; 

    def.sourceFrame = SDL_FRect{srcX, srcY, srcW, srcH};
    def.drawWidth = drawW;
    def.drawHeight = drawH;
    return def;
}
}

Monster::Monster(SDL_Renderer* renderer, float x, float y, MonsterType type)
    : type(type)
{
    // Sprite component is used for rendering and frame selection.
    sprite = addComponent<SpriteComponent>();

    // Load sprite sheet once for this monster object.
    const MonsterDefinition& def = getDefinition(type);
    if (sprite->loadSprite(renderer, "sprite.png", def.drawWidth, def.drawHeight))
    {
        sprite->setSourceRect(
            static_cast<int>(def.sourceFrame.x),
            static_cast<int>(def.sourceFrame.y),
            static_cast<int>(def.sourceFrame.w),
            static_cast<int>(def.sourceFrame.h));
        sprite->setPosition(x, y);
    }

    applyDefinition();
}

void Monster::update(InputHandler&, float deltaTime)
{
    // Framework placeholder for monster AI/status effects.
    updateComponents(deltaTime);
}

void Monster::render(SDL_Renderer* renderer)
{
    renderComponents(renderer);
    (void)renderer;
}


const MonsterDefinition& Monster::getDefinition(MonsterType type)
{
    // Monster atlas layout in sprite.png:
    // - Red monster segment:    y = 512, size = 96x128
    // - Blue monster segment:   y = 256, size = 96x128
    // - Yellow monster segment: y = 384, size = 96x128

    static const MonsterDefinition red = makeDefinition(
        "Red",
        //18, 6, 4, 5,
        0.0f, 512.0f, 96.0f, 128.0f,
        192.0f, 256.0f);

    static const MonsterDefinition blue = makeDefinition(
        "Blue",
        //14, 9, 3, 8,
        0.0f, 256.0f, 96.0f, 128.0f,
        192.0f, 256.0f);

    static const MonsterDefinition yellow = makeDefinition(
        "Yellow",
        //30, 7, 10, 2,
        0.0f, 384.0f, 96.0f, 128.0f,
        192.0f, 256.0f);

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

void Monster::applyDefinition()
{
    // Sync stats with selected type definition.
    const MonsterDefinition& def = getDefinition(type);
    getStats() = def.stats;

    // Update sprite frame if we have a loaded sprite.
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
