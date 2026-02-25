#pragma once
#include <SDL3/SDL.h>
#include "Scene.hpp"
#include "InputHandler.hpp"

// Engine owns SDL window/renderer setup and runs the main loop.
class Engine {
public:
    // Initializes SDL, creates window/renderer, loads textures,
    // and creates initial world objects.
    bool init(const char* title, int width, int height);
    // Runs the game loop:
    // 1) poll input
    // 2) update world state
    // 3) draw frame
    // 4) frame-cap timing
    void run();
    // Releases textures/window/renderer and shuts down SDL.
    void clean();

private:
    // High-level scene mode used to choose which background is rendered.
    enum class GameState {
        // Normal world exploration.
        Roaming,
        // Battle presentation mode.
        Battle
    };

    // Native SDL window handle.
    SDL_Window* window = nullptr;
    // SDL renderer used for all draw calls.
    SDL_Renderer* renderer = nullptr;
    // Background texture for roaming state.
    SDL_Texture* roamingBackground = nullptr;
    // Background texture for battle state.
    SDL_Texture* battleBackground = nullptr;

    // Cached window size used when drawing full-screen backgrounds.
    int windowWidth = 800;
    int windowHeight = 600;

    // Main-loop flag controlled by input/events.
    bool running = true;
    // Current game-state mode (roaming by default).
    GameState gameState = GameState::Roaming;

    // Current scene containing game objects.
    Scene scene;
    // Input polling and key state helper.
    InputHandler input;
};
