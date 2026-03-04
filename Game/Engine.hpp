#pragma once

#include <SDL3/SDL.h>
#include "Scene.hpp"
#include "InputHandler.hpp"

// ENGINE - MAIN GAME LOOP AND SYSTEM MANAGER
// Engine owns and manages all high-level game systems:
// - SDL initialization and shutdown
// - Window and renderer lifecycle
// - Main game loop (input -> update -> render)
// - Frame timing and delta time calculation
// - Game state transitions (Start -> Roaming -> Battle -> End)
// - Background texture management for each game state
//
// The Engine is responsible for orchestrating the overall game flow
// and maintaining frame rate consistency.
class Engine {
public:

    // Initialize SDL, create window/renderer, load textures, and set up the scene.
    // Returns false if initialization fails.

    //   title: window title string
    //   width, height: window dimensions in pixels
    bool init(const char* title, int width, int height);
    
    // Run the main game loop.
    // Continues until a quit event is received or the running flag is cleared.
    // Loop stages per frame:
    // 1 Poll SDL events and update input state
    // 2 Update active game objects and logic based on current game state
    // 3 Render frame with appropriate backgrounds/objects
    // 4 Frame cap timing (targets 60 FPS)
    void run();
    
    // Release textures, renderer, window, and shut down SDL.
    // Must be called to properly clean up resources.
    void clean();

private:
    // GAME STATE ENUM
    // Represents the major screen/mode displayed by the engine.
    enum class GameState {
        Start,          // Title/start screen shown at launch
        Roaming,        // Normal world exploration mode
        Battle,         // Active combat encounter
        End             // Victory screen shown after successful battle
    };

    // SDL RESOURCES
    SDL_Window* window = nullptr;           // Native window handle
    SDL_Renderer* renderer = nullptr;       // SDL rendering context

    // BACKGROUND TEXTURES
    SDL_Texture* roamingBackground = nullptr;   // Texture for world exploration
    SDL_Texture* battleBackground = nullptr;    // Texture for combat scenes
    SDL_Texture* startBackground = nullptr;     // Texture for title screen
    SDL_Texture* endBackground = nullptr;       // Texture for victory screen


    // Used for full-screen background rendering.
    // Updated during init().
    int windowWidth = 800;
    int windowHeight = 600;

    // GAME LOOP STATE
    bool running = true;                    // Main loop flag controlled by quit events
    GameState gameState = GameState::Start;  // Current screen mode
    Scene scene;                            // Game scene (world objects, player, battles)
    InputHandler input;                     // Input polling and state management
};
