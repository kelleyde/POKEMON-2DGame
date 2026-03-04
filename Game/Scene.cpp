#include "Scene.hpp"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>

// SCENE - GAME STATE MANAGER IMPLEMENTATION


// World initialization (player, objects, sprites, collision volumes)
// Input handling and state transitions
// Per-frame updates and rendering for each game mode
// State machine logic for mode switching (Roaming <-> Battle)
// Monster encounter triggering based on world geometry

namespace {
// Tile size constant used throughout the scene for positioning and sizing.
constexpr float kCell = 32.0f;
}

// UTILITY - AABB COLLISION TEST
// Axis-aligned bounding box intersection test used for collision detection
// and interaction probes.
bool Scene::intersects(const SDL_FRect& a, const SDL_FRect& b)
{
    return (a.x < b.x + b.w &&
            a.x + a.w > b.x &&
            a.y < b.y + b.h &&
            a.y + a.h > b.y);
}

// DESTRUCTOR - CLEANUP
// Clean up SDL textures on scene destruction. Unique_ptr members clean themselves.
Scene::~Scene()
{
    SDL_DestroyTexture(spriteSheet);
}


// INITIALIZATION - WORLD SETUP
// One-time setup called before first roaming frame.
// Player at spawn point
// Interactable objects (boxes)
// World geometry (tree, grass, etc.)
// Collision volume lists
void Scene::initialize(SDL_Renderer* inRenderer, int inWindowWidth, int inWindowHeight)
{
    renderer = inRenderer;
    windowWidth = inWindowWidth;
    windowHeight = inWindowHeight;

    // Reset containers in case initialize() is called multiple times.
    worldSprites.clear();
    collisionVolumes.clear();

    // Define playable area with 32px invisible border wall on all sides.
    roomBounds = SDL_FRect{32.0f, 32.0f, static_cast<float>(windowWidth - 64), static_cast<float>(windowHeight - 64)};

    // Load sprite sheet (contains player, monsters, UI icons, decorative tiles).
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

    // Create player instance and position at spawn location.
    player = std::make_unique<Player>(renderer, "sprite.png");
    player->setSpawnPoint(96.0f, 192.0f);
    player->resetToSpawn();

    // Create three interactable boxes with customized dialogue.

    // Box 1 : Rock 
    rock = std::make_unique<Box>(
        renderer,
        //500.0f,
        220.0f,
        365.0f,
        SDL_FRect{64.0f, 768.0f, 32.0f, 32.0f},
        "It's a rock.",
        "Yep. Still a rock.");

    // Box 2: Chest 
    chest = std::make_unique<Box>(
        renderer,
        360.0f,
        365.0f,
        SDL_FRect{32.0f, 768.0f, 32.0f, 32.0f},
        "You open the chest. It's empty... bruh",
        "Just like the fridge... Still empty.");

    // Box 3: Chest 
     box = std::make_unique<Box>(
        renderer,
        //220.0f,
        500.0f,
        365.0f,
        SDL_FRect{0.0f, 768.0f, 32.0f, 32.0f},
        "Hey theres a fishing poll in here!",
        "Now the box is just filled with imagination");

    // Add tree sprite: 96x128 source, placed at center-top, blocks movement.
    // Tree collision is custom-computed in rebuildCollisionVolumes().
    worldSprites.push_back(WorldSprite{
        SDL_FRect{0.0f, 640.0f, 96.0f, 128.0f},
        SDL_FRect{352.0f, 180.0f, 96.0f, 128.0f},
        true,
        false});

    // Add grass tiles as foreground (rendered over player).
    // Grass encounter volume spans bottom portion of screen.
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

    // Build initial collision volume list from world objects.
    rebuildCollisionVolumes();
}

// INPUT PROCESSING - MAIN DISPATCHER
// Central entry point for input handling. Dialogue modal blocks normal input.
// Dispatches to mode-specific handlers (Roaming vs Battle).
void Scene::processInput(const InputHandler& input, Mode mode)
{
    if (!player)
        return;

    // Dialogue is modal: only clear with A button, block all other input.
    if (dialogueBox.visible)
    {
        const bool wasVisible = dialogueBox.visible;
        clearDialogueBoxOnA(dialogueBox, input);
        // Cancel all movement during dialogue.
        player->setMovementInput(false, false, false, false);
        
        // If dialogue was just closed, process pending state transitions.
        if (wasVisible && !dialogueBox.visible)
        {
            if (pendingBattleExitAfterDialogue)
            {
                pendingBattleExitAfterDialogue = false;
                requestBattleExit = true;
            }
            if (pendingEndStateAfterDialogue)
            {
                pendingEndStateAfterDialogue = false;
                requestEndState = true;
            }
        }
        return;
    }

    // Dispatch to mode-specific input handler.
    if (mode == Mode::Roaming)
        processRoamingInput(input);
    if (mode == Mode::Battle)
        processBattleInput(input);
}

// INPUT PROCESSING - ROAMING MODE
// Handle input during world exploration.
// Arrow keys move, A button interacts with nearby objects.
void Scene::processRoamingInput(const InputHandler& input)
{
    // Map arrow keys to movement directions.
    player->setMovementInput(
        input.isKeyDown(SDL_SCANCODE_UP),
        input.isKeyDown(SDL_SCANCODE_DOWN),
        input.isKeyDown(SDL_SCANCODE_LEFT),
        input.isKeyDown(SDL_SCANCODE_RIGHT));

    // A button: trigger interaction with nearby interactable.
    if (input.wasKeyPressed(SDLK_A))
        tryInteractWithBox();
}

// INPUT PROCESSING - BATTLE MODE
// Handle input during monster encounters.
// Left/Right arrows select action, A confirms, B cancels.
void Scene::processBattleInput(const InputHandler& input)
{
    // Battle mode disables movement in the world.
    player->setMovementInput(false, false, false, false);
    
    // Left arrow: cycle selection left (attack -> run -> talk).
    if (input.wasKeyPressed(SDLK_LEFT))
    {
        selectedBattleOption--;
        if (selectedBattleOption < 0)
            selectedBattleOption = 2;  // Wrap to Run
    }
    
    // Right arrow: cycle selection right (attack -> talk -> run).
    if (input.wasKeyPressed(SDLK_RIGHT))
    {
        selectedBattleOption++;
        if (selectedBattleOption > 2)
            selectedBattleOption = 0;  // Wrap to Attack
    }

    // A button: confirm and execute selected action.
    if (input.wasKeyPressed(SDLK_A))
        handleBattleSelection();

    // B button: reserved for cancel/back functionality 
    if (input.wasKeyPressed(SDLK_B))
    {
        //(currently unused)
        //Boo-Whoomp
    }
}

// INTERACTION HANDLING
// Attempt to interact with interactable objects in front of the player.
// Uses player's interaction probe and tests against all boxes.
// Special handling for the fishing pole box (grantsPole = true).
void Scene::tryInteractWithBox()
{
    const SDL_FRect probe = player->getInteractionProbe();

    // Lambda to test and execute interaction with a box.
    // Returns true if interaction occurred, allowing early-exit.
    auto interact = [&](std::unique_ptr<Box>& object, bool grantsPole) -> bool
    {
        // Check if probe overlaps this object's collision box.
        if (!object || !intersects(probe, object->getCollisionBox()))
            return false;

        // Track open state before interaction.
        const bool wasOpened = object->isOpened();
        object->onInteract();
        
        // Award fishing pole on first interaction with the special box.
        if (grantsPole && !wasOpened && object->isOpened())
            player->setFishinpole(true);
        
        // Show dialogue to player.
        showDialogueBox(dialogueBox, object->getInteractText());
        return true;
    };

    // Try interactions with all boxes in priority order.
    if (interact(box, true)) return;      // Fishing pole box
    if (interact(chest, false)) return;   // Chest
    (void)interact(rock, false);          // Rock (no early exit)
}

// STATE UPDATE
// Route per-frame update to appropriate state handler.
void Scene::update(float deltaTime, Mode mode)
{
    if (mode == Mode::Roaming)
        updateRoaming(deltaTime);
    else
        updateBattle(deltaTime);
}

// STATE RENDER 
// Route per-frame render to appropriate state handler.
void Scene::render(SDL_Renderer* renderer, Mode mode)
{
    if (mode == Mode::Roaming)
        renderRoaming(renderer);
    else
        renderBattle(renderer);
}

// STATE TRANSITION QUERIES
// These methods allow Engine to poll for requested state transitions.
// Each method "consumes" its flag when queried (set to false on return).
// Check if battle should start. Returns true and sets outMonsterType if so.
bool Scene::consumeRequestedBattleStart(MonsterType& outMonsterType)
{
    if (!requestBattleStart)
        return false;

    requestBattleStart = false;
    outMonsterType = pendingEncounterMonster;
    return true;
}

// Check if player should exit current battle.
bool Scene::consumeRequestedBattleExit()
{
    if (!requestBattleExit)
        return false;

    requestBattleExit = false;
    return true;
}

// Check if game should transition to victory/end screen.
bool Scene::consumeRequestedEndState()
{
    if (!requestEndState)
        return false;

    requestEndState = false;
    return true;
}

// BATTLE LIFECYCLE - ENTRY
// Called by Engine when transitioning into battle mode.
// Initializes battle UI state and spawns the encountered monster.
void Scene::onBattleEntered(MonsterType type)
{
    activeMonsterType = type;
    selectedBattleOption = static_cast<int>(BattleOption::Attack);
    buildBattleMonster(renderer, type);
}

// BATTLE LIFECYCLE - EXIT
// Called by Engine when transitioning out of battle mode.
// Returns player to their original spawn point.
void Scene::onBattleExited()
{
    if (player)
        player->resetToSpawn();
}


// COLLISION MANAGEMENT - VOLUME REBUILDING
// Aggregate collision boxes from all world objects (sprites, boxes, etc.).
// Called during initialization and whenever world geometry changes.
// Player uses this list to resolve movement collisions.
void Scene::rebuildCollisionVolumes()
{
    collisionVolumes.clear();

    // Add blocking volumes from world sprites (trees, walls, etc.).
    for (const WorldSprite& sprite : worldSprites)
    {
        if (!sprite.blocksMovement)
            continue;
        
        // Special case: tree uses only its base section as blocking area.
        // Tree source y=640; use bottom 64x64 region instead of full 96x128.
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

    // Add collision boxes from all interactable objects.
    if (box)
        collisionVolumes.push_back(box->getCollisionBox());
    if (chest)
        collisionVolumes.push_back(chest->getCollisionBox());
    if (rock)
        collisionVolumes.push_back(rock->getCollisionBox());
}

// UPDATE - ROAMING MODE
// Per-frame update during world exploration.
// Updates player, objects, and checks for grass encounter triggers.
void Scene::updateRoaming(float deltaTime)
{
    if (!player)
        return;

    // Configure player collision environment for this frame.
    player->setCollisionEnvironment(roomBounds, collisionVolumes);
    
    // Update player (movement, animation).
    static InputHandler dummyInput;
    player->update(dummyInput, deltaTime);

    // Update all world objects (mostly animation state).
    if (box)
    {
        static InputHandler dummyInputForBox;
        box->update(dummyInputForBox, deltaTime);
    }
    if (chest)
    {
        static InputHandler dummyInputForChest;
        chest->update(dummyInputForChest, deltaTime);
    }
    if (rock)
    {
        static InputHandler dummyInputForRock;
        rock->update(dummyInputForRock, deltaTime);
    }

    // Grass encounter trigger: detect when player enters grass region.
    const bool inGrassNow = intersects(player->getCollisionBox(), grassVolume);
    if (inGrassNow && !playerInGrassLastFrame)
    {
        // Entering grass for the first time this frame: trigger battle.
        pendingEncounterMonster = getRandomMonsterType();
        requestBattleStart = true;
    }
    playerInGrassLastFrame = inGrassNow;
}

// UPDATE - BATTLE MODE
// Per-frame update during combat encounters.
// Currently just updates the active monster (animation, etc.).
void Scene::updateBattle(float deltaTime)
{
    if (activeMonster)
    {
        static InputHandler dummyInput;
        activeMonster->update(dummyInput, deltaTime);
    }
}

// RENDER - ROAMING MODE
// 1 Background sprites (objects behind player)
// 2 Interactable boxes
// 3 Player
// 4 Foreground sprites (grass over player)
// 5 Dialogue (if visible)
void Scene::renderRoaming(SDL_Renderer* renderer)
{
    // Layer 1: Background geometry (trees, decorations).
    for (const WorldSprite& sprite : worldSprites)
    {
        if (sprite.foreground)
            continue;  // Skip foreground; draw in layer 4.
        if (spriteSheet)
            SDL_RenderTexture(renderer, spriteSheet, &sprite.source, &sprite.destination);
    }

    // Layer 2: Interactable objects (boxes).
    if (box)
        box->render(renderer);
    if (chest)
        chest->render(renderer);
    if (rock)
        rock->render(renderer);

    // Layer 2: Debug collision visualization (disable for release).
    constexpr bool showCollisionLayer = false;
    if (showCollisionLayer)
    {
        SDL_SetRenderDrawColor(renderer, 255, 64, 64, 190);
        for (const SDL_FRect& collBox : collisionVolumes)
            SDL_RenderRect(renderer, &collBox);
    }

    // Layer 3: Player sprite.
    if (player)
        player->render(renderer);

    // Layer 4: Foreground geometry (grass, foliage over player).
    for (const WorldSprite& sprite : worldSprites)
    {
        if (!sprite.foreground)
            continue;  // Already drawn in layer 1.
        if (spriteSheet)
            SDL_RenderTexture(renderer, spriteSheet, &sprite.source, &sprite.destination);
    }
    
    // Layer 5: Modal dialogue box (if active).
    renderDialogueBox(renderer, spriteSheet, windowWidth, windowHeight, dialogueBox);
}

// RENDER - BATTLE MODE
// Render the battle screen with UI overlay:
// 1 Monster in upper area
// 2 Battle menu icons (Attack/Talk/Run)
// 3 Player cursor over selected option
// 4 Dialogue modal (if visible)
void Scene::renderBattle(SDL_Renderer* renderer)
{
    // Layer 1: Active monster centered in upper section.
    if (activeMonster)
        activeMonster->render(renderer);

    // Layer 2: Battle UI panels and option icons.
    // Panel geometry: top half for monster, bottom half for options.
    SDL_FRect topPanel = {20.0f, 20.0f, static_cast<float>(windowWidth - 40), static_cast<float>(windowHeight * 0.48f)};
    SDL_FRect bottomPanel = {20.0f, static_cast<float>(windowHeight * 0.56f), static_cast<float>(windowWidth - 40), static_cast<float>(windowHeight * 0.38f)};

    // Note: battle background (battleground.png) is rendered by Engine.
    // Uncomment below to draw panel borders for debugging:
    // SDL_SetRenderDrawColor(renderer, 225, 225, 230, 255);
    // SDL_RenderRect(renderer, &topPanel);
    // SDL_RenderRect(renderer, &bottomPanel);

    // Draw three battle action icons from sprite sheet segment 2, row 1.
    // Each icon is 32x32; source y=128, x positions 0, 32, 64.
    const SDL_FRect optionSrcAttack = {0.0f, 128.0f, 32.0f, 32.0f};
    const SDL_FRect optionSrcTalk = {32.0f, 128.0f, 32.0f, 32.0f};
    const SDL_FRect optionSrcRun = {64.0f, 128.0f, 32.0f, 32.0f};

    // Place icons evenly across bottom panel in three slots.
    const float slotW = bottomPanel.w / 3.0f;
    SDL_FRect attackDst = {bottomPanel.x + 50.0f, bottomPanel.y + 48.0f, 64.0f, 64.0f};
    SDL_FRect talkDst = {bottomPanel.x + slotW + 50.0f, bottomPanel.y + 48.0f, 64.0f, 64.0f};
    SDL_FRect runDst = {bottomPanel.x + (2.0f * slotW) + 50.0f, bottomPanel.y + 48.0f, 64.0f, 64.0f};

    if (spriteSheet)
    {
        SDL_RenderTexture(renderer, spriteSheet, &optionSrcAttack, &attackDst);
        SDL_RenderTexture(renderer, spriteSheet, &optionSrcTalk, &talkDst);
        SDL_RenderTexture(renderer, spriteSheet, &optionSrcRun, &runDst);
    }

    // Layer 3: Player cursor (small sprite) above selected option.
    if (player)
        player->renderBattleCursor(selectedBattleOption, bottomPanel);

    // Layer 4: Modal dialogue box (if active).
    renderDialogueBox(renderer, spriteSheet, windowWidth, windowHeight, dialogueBox);
}

// BATTLE LOGIC - MONSTER SPAWNING
// Create the active monster for the current battle encounter.
// Positions monster in top-center based on its defined draw size.
void Scene::buildBattleMonster(SDL_Renderer* renderer, MonsterType type)
{
    const MonsterDefinition& def = Monster::getDefinition(type);
    const float monsterX = (static_cast<float>(windowWidth) - def.drawWidth) * 0.5f;
    const float monsterY = 48.0f;
    activeMonster = std::make_unique<Monster>(renderer, monsterX, monsterY, type);
}

// BATTLE LOGIC - RANDOM MONSTER SELECTION
// Select a random monster type for wild grass encounters.
// Currently uses uniform distribution over Red/Blue/Yellow types.
MonsterType Scene::getRandomMonsterType()
{
    std::uniform_int_distribution<int> pick(0, 2);
    const int value = pick(rng);
    if (value == 0) return MonsterType::Red;
    if (value == 1) return MonsterType::Blue;
    return MonsterType::Yellow;
}

// BATTLE LOGIC - ACTION EXECUTION
// Execute the player's selected battle action.
// Handles Attack, Talk, and Run options with context-specific dialogue.
void Scene::handleBattleSelection()
{
    const BattleOption option = static_cast<BattleOption>(selectedBattleOption);
    switch (option)
    {
    case BattleOption::Attack:
        // Attack action: check if player has fishing pole to win.
        if (player && player->hasFishinpole())
        {
            // Victory condition: attack with fishing pole -> end game.
            showDialogueBox(dialogueBox, "lets go fishing");
            pendingEndStateAfterDialogue = true;
        }
        else
        {
            // No fishing pole: battle is lost, return to roaming.
            showDialogueBox(dialogueBox, "OOP you need a fishin rod bud!");
            if (player)
                player->resetToSpawn();
            pendingBattleExitAfterDialogue = true;
        }
        break;

    case BattleOption::Talk:
        // Talk action: show monster-specific dialogue.
        switch (activeMonsterType)
        {
        case MonsterType::Red:
            showDialogueBox(dialogueBox, "Anything bitin?");
            break;
        case MonsterType::Blue:
            showDialogueBox(dialogueBox, "Might go swimming again today");
            break;
        case MonsterType::Yellow:
            showDialogueBox(dialogueBox, "Believe it");
            break;
        }
        break;

    case BattleOption::Run:
        // Run action: flee battle and return to roaming.
        showDialogueBox(dialogueBox, "Later bud :]");
        pendingBattleExitAfterDialogue = true;
        break;
    }
}
