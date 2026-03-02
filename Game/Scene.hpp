#pragma once
#include <memory>
#include <random>
#include <vector>

#include "InputHandler.hpp"
#include "GameObject.hpp"

#include "Box.hpp"

#include "Monster.hpp"

#include "Player.hpp"

// Scene manages both major game states:
// - Roaming: world objects/collision/player movement/grass encounters
// - Battle: monster + battle menu framework

class Scene {
public:
    ~Scene();

    enum class Mode {
        Roaming,
        Battle
    };

    enum class BattleOption {
        Attack = 0,
        Talk = 1,
        Run = 2
    };

    // One-time setup for player, world objects, and initial collision volumes.
    void initialize(SDL_Renderer* renderer, int windowWidth, int windowHeight);

    // Input stage for current state.
    void processInput(const InputHandler& input, Mode mode);
    
    // Update stage for current state.
    void update(float deltaTime, Mode mode);
    
    // Render stage for current state.
    void render(SDL_Renderer* renderer, Mode mode);

    // State transition hooks called by Engine.
    bool consumeRequestedBattleStart(MonsterType& outMonsterType);
    bool consumeRequestedBattleExit();
    void onBattleEntered(MonsterType type);
    void onBattleExited();


private:
    struct WorldSprite {
        SDL_FRect source{};
        SDL_FRect destination{};
        bool blocksMovement = false;
        bool foreground = false;
    };

    static bool intersects(const SDL_FRect& a, const SDL_FRect& b);
    void rebuildCollisionVolumes();
    void updateRoaming(float deltaTime);
    void updateBattle(float deltaTime);
    void renderRoaming(SDL_Renderer* renderer);
    void renderBattle(SDL_Renderer* renderer);
    void buildBattleMonster(SDL_Renderer* renderer, MonsterType type);
    MonsterType getRandomMonsterType();
    void handleBattleSelection();

    SDL_Renderer* renderer = nullptr;
    SDL_Texture* spriteSheet = nullptr;
    int windowWidth = 800;
    int windowHeight = 600;

    // Roaming world bounds with 32px border wall.
    //The orginal had a 32 pixel frame this was left in //fix later
    SDL_FRect roomBounds{32.0f, 32.0f, 736.0f, 536.0f};

    // Player and interactables in roaming world.
    std::unique_ptr<Player> player;
    std::unique_ptr<Box> box;
    std::vector<WorldSprite> worldSprites;
    std::vector<SDL_FRect> collisionVolumes;

    // Grass trigger volume. Entering this can start battle.
    SDL_FRect grassVolume{0.0f, 0.0f, 0.0f, 0.0f};
    bool playerInGrassLastFrame = false;

    // Battle state framework.
    std::unique_ptr<Monster> activeMonster;
    MonsterType activeMonsterType = MonsterType::Red;
    int selectedBattleOption = static_cast<int>(BattleOption::Attack);

    // Transition requests consumed by Engine.
    bool requestBattleStart = false;
    bool requestBattleExit = false;
    MonsterType pendingEncounterMonster = MonsterType::Red;

    // RNG for random encounters.
    std::mt19937 rng{std::random_device{}()};
};
