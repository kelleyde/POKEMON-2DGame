#include "Scene.hpp"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>

namespace {
constexpr float kCell = 32.0f;
}

bool Scene::intersects(const SDL_FRect& a, const SDL_FRect& b)
{
    return (a.x < b.x + b.w &&
            a.x + a.w > b.x &&
            a.y < b.y + b.h &&
            a.y + a.h > b.y);
}

Scene::~Scene()
{
    SDL_DestroyTexture(spriteSheet);
}

void Scene::initialize(SDL_Renderer* inRenderer, int inWindowWidth, int inWindowHeight)
{
    renderer = inRenderer;
    windowWidth = inWindowWidth;
    windowHeight = inWindowHeight;

    // Reset scene-owned containers in case initialize() is called again.
    worldSprites.clear();
    collisionVolumes.clear();

    // Keep a 32px invisible border wall around the full roaming area.
    roomBounds = SDL_FRect{32.0f, 32.0f, static_cast<float>(windowWidth - 64), static_cast<float>(windowHeight - 64)};

    // Load sprite sheet used for world objects, UI icons, and overlays.
    if (spriteSheet)
    {
        SDL_DestroyTexture(spriteSheet);
        spriteSheet = nullptr;
    }
    SDL_Surface* surface = IMG_Load("sprite.png");
    if (surface)
    {
        spriteSheet = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_DestroySurface(surface);
    }

    //// create monster
    MonsterType Wisp = MonsterType::Wisp;
    const MonsterDefinition& def = Monster::getDefinition(Wisp);
    const float monsterX = (static_cast<float>(windowWidth) - def.drawWidth) * 0.5f;
    const float monsterY = 48.0f;
    monst = <Monster>(renderer, monsterX, monsterY, Wisp);
    

    // Create player and define stable spawn point.
    player = std::make_unique<Player>(renderer, "sprite.png");
    player->setSpawnPoint(96.0f, 96.0f);
    player->resetToSpawn();

    // Create one interactable box object.
    box = std::make_unique<Box>(renderer, 220.0f, 340.0f);

    // World object row 1 from world-object segment.
    // Segment starts at y=768, each icon is 32x32.
    // We draw all as in-world object layer.
    worldSprites.push_back(WorldSprite{
        SDL_FRect{32.0f, 768.0f, 32.0f, 32.0f},  // chest icon
        SDL_FRect{360.0f, 340.0f, 32.0f, 32.0f},
        true,
        false});
    worldSprites.push_back(WorldSprite{
        SDL_FRect{64.0f, 768.0f, 32.0f, 32.0f},  // rock icon
        SDL_FRect{500.0f, 340.0f, 32.0f, 32.0f},
        true,
        false});

    // Tree segment starts at y=640, frame is 96x128.
    // Place tree near center with collision (64x64 base).
    worldSprites.push_back(WorldSprite{
        SDL_FRect{0.0f, 640.0f, 96.0f, 128.0f},
        SDL_FRect{352.0f, 180.0f, 96.0f, 128.0f},
        true,
        false});

    // Grass row tile (segment y=768, second row y=800, width=96, height=32).
    // Draw as foreground so it overlays player.
    grassVolume = SDL_FRect{32.0f, static_cast<float>(windowHeight - 68), static_cast<float>(windowWidth - 64), 36.0f};
    const int grassTiles = static_cast<int>((windowWidth - 64) / 96.0f) + 1;
    for (int i = 0; i < grassTiles; ++i)
    {
        worldSprites.push_back(WorldSprite{
            SDL_FRect{0.0f, 800.0f, 96.0f, 32.0f},
            SDL_FRect{32.0f + (96.0f * i), static_cast<float>(windowHeight - 72), 96.0f, 32.0f},
            false,
            true});
    }

    rebuildCollisionVolumes();
}

void Scene::processInput(const InputHandler& input, Mode mode)
{
    if (!player)
    {
        return;
    }

    if (mode == Mode::Roaming)
    {
        // Arrow keys control movement.
        player->setMovementInput(
            input.isKeyDown(SDL_SCANCODE_UP),
            input.isKeyDown(SDL_SCANCODE_DOWN),
            input.isKeyDown(SDL_SCANCODE_LEFT),
            input.isKeyDown(SDL_SCANCODE_RIGHT));

        // A button interacts with the first object in front of the player.
        if (input.wasKeyPressed(SDLK_A))
        {
            const SDL_FRect probe = player->getInteractionProbe();
            if (box && intersects(probe, box->getCollisionBox()))
            {
                box->onInteract();
            }
        }
    }
    else
    {
        // In battle we use left/right to change action selection.
        player->setMovementInput(false, false, false, false);
        if (input.wasKeyPressed(SDLK_LEFT))
        {
            selectedBattleOption--;
            if (selectedBattleOption < 0)
                selectedBattleOption = 2;
        }
        if (input.wasKeyPressed(SDLK_RIGHT))
        {
            selectedBattleOption++;
            if (selectedBattleOption > 2)
                selectedBattleOption = 0;
        }

        // A confirms selected action.
        if (input.wasKeyPressed(SDLK_A))
        {
            handleBattleSelection();
        }

        // B is cancel/back (for now just log).
        if (input.wasKeyPressed(SDLK_B))
        {
            std::cout << "Battle: cancel/back\n";
        }
    }
}

void Scene::update(float deltaTime, Mode mode)
{
    if (mode == Mode::Roaming)
    {
        updateRoaming(deltaTime);
    }
    else
    {
        updateBattle(deltaTime);
    }
}

void Scene::render(SDL_Renderer* renderer, Mode mode)
{
    if (mode == Mode::Roaming)
    {
        renderRoaming(renderer);
    }
    else
    {
        renderBattle(renderer);
    }
}

bool Scene::consumeRequestedBattleStart(MonsterType& outMonsterType)
{
    if (!requestBattleStart)
    {
        return false;
    }

    requestBattleStart = false;
    outMonsterType = pendingEncounterMonster;
    return true;
}

bool Scene::consumeRequestedBattleExit()
{
    if (!requestBattleExit)
    {
        return false;
    }

    requestBattleExit = false;
    return true;
}

void Scene::onBattleEntered(MonsterType type)
{
    activeMonsterType = type;
    selectedBattleOption = static_cast<int>(BattleOption::Attack);
    buildBattleMonster(renderer, type);
}

void Scene::onBattleExited()
{
    // Return player to the original spawn point.
    if (player)
    {
        player->resetToSpawn();
    }
}

Player* Scene::getPlayer() const
{
    return player.get();
}

void Scene::rebuildCollisionVolumes()
{
    collisionVolumes.clear();

    // Add blocking world sprites.
    for (const WorldSprite& sprite : worldSprites)
    {
        if (sprite.blocksMovement)
        {
            // Tree uses only base section as blocking area.
            if (sprite.source.y == 640.0f)
            {
                collisionVolumes.push_back(SDL_FRect{
                    sprite.destination.x + 16.0f,
                    sprite.destination.y + 64.0f,
                    64.0f,
                    64.0f});
            }
            else
            {
                collisionVolumes.push_back(sprite.destination);
            }
        }
    }

    // Add box collision.
    if (box)
    {
        collisionVolumes.push_back(box->getCollisionBox());
    }
}

void Scene::updateRoaming(float deltaTime)
{
    if (!player)
    {
        return;
    }

    // Apply current collision environment before movement update.
    player->setCollisionEnvironment(roomBounds, collisionVolumes);
    static InputHandler dummyInput;
    player->update(dummyInput, deltaTime);

    if (box)
    {
        static InputHandler dummyInputForBox;
        box->update(dummyInputForBox, deltaTime);
    }

    // Grass encounter trigger: entering grass has chance to start battle.
    const bool inGrassNow = intersects(player->getCollisionBox(), grassVolume);
    if (inGrassNow && !playerInGrassLastFrame)
    {
        // Requirement: walking into grass transitions into battle.
        pendingEncounterMonster = getRandomMonsterType();
        requestBattleStart = true;
    }
    playerInGrassLastFrame = inGrassNow;
}

void Scene::updateBattle(float deltaTime)
{
    if (activeMonster)
    {
        static InputHandler dummyInput;
        activeMonster->update(dummyInput, deltaTime);
    }
}

void Scene::renderRoaming(SDL_Renderer* renderer)
{
    // Layer 1: object layer (ground object sprites behind player).
    for (const WorldSprite& sprite : worldSprites)
    {
        if (sprite.foreground)
            continue;
        if (spriteSheet)
        {
            SDL_RenderTexture(renderer, spriteSheet, &sprite.source, &sprite.destination);
        }
    }

    // Render Box entity.
    if (box)
    {
        box->render(renderer);
    }

    // Collision layer framework:
    // Toggle this to true while building collision-heavy scenes.
    constexpr bool showCollisionLayer = false;
    if (showCollisionLayer)
    {
        SDL_SetRenderDrawColor(renderer, 255, 64, 64, 190);
        for (const SDL_FRect& box : collisionVolumes)
        {
            SDL_RenderRect(renderer, &box);
        }
    }

    // Layer 2: player.
    if (player)
    {
        player->render(renderer);
    }

    if (monst) {
        monst->render(renderer);
    }
     
    //monsterPos(renderer, 48.0f, 48.0f);


    // Layer 3: foreground objects (grass over the player).
    for (const WorldSprite& sprite : worldSprites)
    {
        if (!sprite.foreground)
            continue;
        if (spriteSheet)
        {
            SDL_RenderTexture(renderer, spriteSheet, &sprite.source, &sprite.destination);
        }
    }

    // Layer 4: text box framework (placeholder frame at bottom).
    SDL_FRect textBox = {24.0f, static_cast<float>(windowHeight - 170), static_cast<float>(windowWidth - 48), 146.0f};
    SDL_SetRenderDrawColor(renderer, 20, 20, 28, 220);
    SDL_RenderFillRect(renderer, &textBox);
    SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
    SDL_RenderRect(renderer, &textBox);
}

void Scene::renderBattle(SDL_Renderer* renderer)
{
    // Layer 1: monster in upper section center.
    if (activeMonster)
    {
        activeMonster->render(renderer);
    }

    // Layer 2: battle UI framework (upper/lower panels + option icons).
    SDL_FRect topPanel = {20.0f, 20.0f, static_cast<float>(windowWidth - 40), static_cast<float>(windowHeight * 0.48f)};
    SDL_FRect bottomPanel = {20.0f, static_cast<float>(windowHeight * 0.56f), static_cast<float>(windowWidth - 40), static_cast<float>(windowHeight * 0.38f)};

    // Background image (battleground.png) is already rendered by Engine.
    // We only draw panel outlines here so the authored texture stays visible.
    SDL_SetRenderDrawColor(renderer, 225, 225, 230, 255);
    SDL_RenderRect(renderer, &topPanel);
    SDL_RenderRect(renderer, &bottomPanel);

    // UI options from sprite sheet segment 2 row 1: attack/talk/run.
    // Segment starts at y=128, row1 y=128, each icon 32x32.
    const SDL_FRect optionSrcAttack = {0.0f, 128.0f, 32.0f, 32.0f};
    const SDL_FRect optionSrcTalk = {32.0f, 128.0f, 32.0f, 32.0f};
    const SDL_FRect optionSrcRun = {64.0f, 128.0f, 32.0f, 32.0f};

    const float slotW = bottomPanel.w / 3.0f;
    SDL_FRect attackDst = {bottomPanel.x + 18.0f, bottomPanel.y + 48.0f, 64.0f, 64.0f};
    SDL_FRect talkDst = {bottomPanel.x + slotW + 18.0f, bottomPanel.y + 48.0f, 64.0f, 64.0f};
    SDL_FRect runDst = {bottomPanel.x + (2.0f * slotW) + 18.0f, bottomPanel.y + 48.0f, 64.0f, 64.0f};

    if (spriteSheet)
    {
        SDL_RenderTexture(renderer, spriteSheet, &optionSrcAttack, &attackDst);
        SDL_RenderTexture(renderer, spriteSheet, &optionSrcTalk, &talkDst);
        SDL_RenderTexture(renderer, spriteSheet, &optionSrcRun, &runDst);
    }

    // Layer 3: player cursor rendered over options.
    if (player)
    {
        player->renderBattleCursor(selectedBattleOption, bottomPanel);
    }
}

void Scene::buildBattleMonster(SDL_Renderer* renderer, MonsterType type)
{
    // Place battle monster in top-center area based on each type's draw size.
    const MonsterDefinition& def = Monster::getDefinition(type);
    const float monsterX = (static_cast<float>(windowWidth) - def.drawWidth) * 0.5f;
    const float monsterY = 48.0f;
    activeMonster = std::make_unique<Monster>(renderer, monsterX, monsterY, type);
}

MonsterType Scene::getRandomMonsterType()
{
    std::uniform_int_distribution<int> pick(0, 2);
    const int value = pick(rng);
    if (value == 0) return MonsterType::Slime;
    if (value == 1) return MonsterType::Wisp;
    return MonsterType::Golem;
}

void Scene::handleBattleSelection()
{
    const BattleOption option = static_cast<BattleOption>(selectedBattleOption);
    switch (option)
    {
    case BattleOption::Attack:
        std::cout << "Battle action selected: ATTACK\n";
        break;
    case BattleOption::Talk:
        std::cout << "Battle action selected: TALK\n";
        // Monster dialog for now just prints color label.
        switch (activeMonsterType)
        {
        case MonsterType::Slime:
            std::cout << "Monster dialog: blue\n";
            break;
        case MonsterType::Wisp:
            std::cout << "Monster dialog: yellow\n";
            break;
        case MonsterType::Golem:
            std::cout << "Monster dialog: red\n";
            break;
        }
        break;
    case BattleOption::Run:
        std::cout << "Battle action selected: RUN\n";
        requestBattleExit = true;
        break;
    }
}



void Scene::monsterPos(SDL_Renderer* renderer, float x, float y)
{
    SDL_Texture* monstText = IMG_LoadTexture(renderer, "sprite.png");
    float w, h;
    SDL_GetTextureSize(monstText, &w, &h);
    SDL_FRect pos = { x, y, float(w), float(h) };
    SDL_RenderTexture(renderer, monstText, nullptr, &pos);
    SDL_RenderPresent(renderer);

}