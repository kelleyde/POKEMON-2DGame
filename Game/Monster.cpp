#include "Monster.hpp"

namespace {
MonsterDefinition makeDefinition(
    const char* name,
    int hp,
    int attack,
    int defense,
    int speed,
    float srcX,
    float srcY,
    float srcW,
    float srcH,
    float drawW,
    float drawH
)
{
    MonsterDefinition def{};
    def.displayName = name;
    def.stats.maxHP = hp;
    def.stats.currentHP = hp;
    def.stats.attack = attack;
    def.stats.defense = defense;
    def.stats.speed = speed;
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

void Monster::setType(MonsterType newType)
{
    type = newType;
    applyDefinition();
}

MonsterType Monster::getType() const
{
    return type;
}

const char* Monster::getDisplayName() const
{
    return getDefinition(type).displayName;
}

const MonsterDefinition& Monster::getDefinition(MonsterType type)
{
    // Monster atlas layout in sprite.png:
    // - Blue monster segment:   y = 256, size = 96x128
    // - Yellow monster segment: y = 384, size = 96x128
    // - Red monster segment:    y = 512, size = 96x128
    static const MonsterDefinition slime = makeDefinition(
        "Slime",
        18, 6, 4, 5,
        0.0f, 256.0f, 96.0f, 128.0f,
        192.0f, 256.0f);

    static const MonsterDefinition wisp = makeDefinition(
        "Wisp",
        14, 9, 3, 8,
        0.0f, 384.0f, 96.0f, 128.0f,
        192.0f, 256.0f);

    static const MonsterDefinition golem = makeDefinition(
        "Golem",
        30, 7, 10, 2,
        0.0f, 512.0f, 96.0f, 128.0f,
        192.0f, 256.0f);

    switch (type)
    {
    case MonsterType::Slime:
        return slime;
    case MonsterType::Wisp:
        return wisp;
    case MonsterType::Golem:
        return golem;
    default:
        return slime;
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

//void Monster::monsterPos(SDL_Renderer* renderer, float x, float y)
//{
//    SDL_Texture* monstText = IMG_LoadTexture(renderer, "sprite.png");
//    int w, h;
//    SDL_GetTextureSize(monstText, &w, &h);
//    SDL_FRect pos = { x, y, float(w), float(h) };
//    SDL_RenderTexture(renderer, monstText, nullptr, &pos);
//    SDL_RenderPresent(renderer);
//
//}

void Monster::Right()
{
    if (!sprite) {
        return;
    }
    sprite->moveBy(0.1f, 0.0f);
}

void Monster::Left()
{
    if (!sprite) {
        return;
    }
    sprite->moveBy(-0.1f, 0.0f);
}

void Monster::Up()
{
    if (!sprite) {
        return;
    }
    sprite->moveBy(0.0f, -0.1f);
}

void Monster::Down()
{
    if (!sprite) {
        return;
    }
    sprite->moveBy(0.0f, -0.1f);
}

//void Monster::Draw()
//{ 
//    if (!sprite) {
//        return;
//    }
//    SDL_FRect* dest = sprite->getRect();
//    sprite->drawAt(dest);
//}

void Monster::NewSize(float w, float h)
{
    sprite->setSize(w, h);
}

