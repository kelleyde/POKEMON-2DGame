#pragma once

#include <memory>
#include <random>
#include <vector>

#include "InputHandler.hpp"
#include "GameObject.hpp"
#include "components.hpp"
#include "Box.hpp"
#include "Monster.hpp"
#include "Player.hpp"

// SCENE - GAME STATE MANAGER
// Scene manages the two primary game modes and transitions between them:
// 1. Roaming: Free-form world exploration with player movement, collision,
//             and grass encounters
// 2. Battle: Monster encounter with menu-based action selection system
//
// Scene owns the Player, all world objects (Box, Monster), collision detection,
// and state machine logic for mode transitions.
class Scene {
public:
    // Destructor cleans up texture resources.
    ~Scene();

    // Enumeration of all major game modes.
    enum class Mode {
        Roaming,        // Player navigates the world
        Battle,         // Combat encounter active
        Start,          // Start screen (not managed by Scene)
        End             // Victory screen (not managed by Scene)
    };

    // Battle action menu options available during combat.
    enum class BattleOption {
        Attack = 0,     // Attack the monster
        Talk = 1,       // Talk to the monster
        Run = 2         // Attempt to flee
    };

    // One-time setup: initializes player, world objects, and collision system.
    // Should be called once before entering Roaming mode.
    void initialize(SDL_Renderer* renderer, int windowWidth, int windowHeight);

    // Process input for the current game state.
    void processInput(const InputHandler& input, Mode mode);
    
    // Update logic for the current game state.
    void update(float deltaTime, Mode mode);
    
    // Render the current game state.
    void render(SDL_Renderer* renderer, Mode mode);

    // STATE TRANSITION QUERIES
    // These methods check if a state transition has been requested and
    // "consume" the request (set flag to false) when called.
    // Check if a battle should start; returns the monster type encountered.
    bool consumeRequestedBattleStart(MonsterType& outMonsterType);

    // Check if the player should exit the current battle.
    bool consumeRequestedBattleExit();

    // Check if the game should show the victory screen.
    bool consumeRequestedEndState();

    // Called by Engine when entering battle mode.
    void onBattleEntered(MonsterType type);

    // Called by Engine when exiting battle mode.
    void onBattleExited();

private:
    // WORLD SPRITE STRUCT
    // Lightweight representation of decorative/blocking world geometry.
    // Used for rendering backgrounds and collision obstacles.
    struct WorldSprite {
        SDL_FRect source{};         // Source region in sprite sheet
        SDL_FRect destination{};    // On-screen position and size
        bool blocksMovement = false;  // Does player collide with this?
        bool foreground = false;      // Render before or after player?
    };

    // UTILITY METHODS
    // AABB collision test for two rectangles.
    static bool intersects(const SDL_FRect& a, const SDL_FRect& b);

    // Handle input while dialogue box is open.
    bool handleModalDialogueInput(const InputHandler& input);

    // INPUT PROCESSING
    // Handle input during roaming state.
    void processRoamingInput(const InputHandler& input);
    // Handle input during battle state.
    void processBattleInput(const InputHandler& input);
    // Attempt to interact with nearby interactable objects.
    void tryInteractWithBox();

    // COLLISION MANAGEMENT
    // Rebuild collision volume list from all world objects.
    void rebuildCollisionVolumes();

    // UPDATE LOGIC
    // Update logic during roaming state (player movement, encounters, etc.).
    void updateRoaming(float deltaTime);
    // Update logic during battle state (animation, AI, etc.).
    void updateBattle(float deltaTime);

    // RENDERING 
    // Render static world sprites (foreground, background, tiles).
    void renderWorldSprites(SDL_Renderer* renderer, bool foreground);
    // Render the roaming world.
    void renderRoaming(SDL_Renderer* renderer);
    // Render the battle menu options (Attack/Talk/Run).
    void renderBattleOptions(SDL_Renderer* renderer, const SDL_FRect& bottomPanel);
    // Render the selection cursor over the current battle option.
    void renderBattleSelectionCursor(SDL_Renderer* renderer, const SDL_FRect& bottomPanel);
    // Render the battle scene state.
    void renderBattle(SDL_Renderer* renderer);

    // BATTLE LOGIC
    // Create the active monster for the current battle encounter.
    void buildBattleMonster(SDL_Renderer* renderer, MonsterType type);
    // Select a random monster type for a wild encounter.
    MonsterType getRandomMonsterType();
    // Process the player's selected battle action.
    void handleBattleSelection();

    // MEMBER VARIABLES
    SDL_Renderer* renderer = nullptr;           // SDL render target
    SDL_Texture* spriteSheet = nullptr;         // Master sprite sheet texture

    int windowWidth = 800;                      // Window dimensions (cached)
    int windowHeight = 600;

    // Roaming world bounds with 32px invisible border wall.
    SDL_FRect roomBounds{32.0f, 32.0f, 736.0f, 536.0f};

    // Player and interactable objects in roaming world.
    std::unique_ptr<Player> player;             // Primary player character
    std::unique_ptr<Box> box;                   // Fishing pole box
    std::unique_ptr<Box> chest;                 // Chest (unused, placeholder)
    std::unique_ptr<Box> rock;                  // Rock (unused, placeholder)
    std::vector<WorldSprite> worldSprites;      // Decorative/blocking geometry
    std::vector<SDL_FRect> collisionVolumes;    // All collision obstacles

    // Grass trigger volume for wild encounters.
    SDL_FRect grassVolume{0.0f, 0.0f, 0.0f, 0.0f};
    bool playerInGrassLastFrame = false;        // For encounter triggering logic

    // Active monster and battle state.
    std::unique_ptr<Monster> activeMonster;     // Current battle opponent
    MonsterType activeMonsterType = MonsterType::Red;
    int selectedBattleOption = static_cast<int>(BattleOption::Attack);

    // State transition requests consumed by Engine.
    bool requestBattleStart = false;            // Transition to battle mode
    bool requestBattleExit = false;             // Exit battle mode
    bool requestEndState = false;               // Show victory screen
    bool pendingBattleExitAfterDialogue = false;  // Queue battle exit after text
    bool pendingEndStateAfterDialogue = false;    // Queue victory after text
    MonsterType pendingEncounterMonster = MonsterType::Red;  // Type to spawn
    DialogueBoxState dialogueBox{};             // Current dialogue state

    // Random number generator for monster type selection.
    std::mt19937 rng{std::random_device{}()};
};
