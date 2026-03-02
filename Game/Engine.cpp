#include "Engine.hpp"
#include <SDL3_image/SDL_image.h>
#include <iostream>

// Anonymous namespace limits helper visibility to this translation unit.
namespace {
// Helper for loading image files into renderable textures.
// Returns nullptr on failure and logs the SDL error details.
SDL_Texture* loadTexture(SDL_Renderer* renderer, const char* path)
{
    // Load file bytes into a CPU-side surface first.
    SDL_Surface* surface = IMG_Load(path);
    if (!surface)
    {
        std::cerr << "IMG_Load failed for '" << path << "': " << SDL_GetError() << '\n';
        return nullptr;
    }

    // Convert CPU surface to GPU texture for fast rendering.
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    
    // Surface no longer needed after texture upload.
    SDL_DestroySurface(surface);
    if (!texture)
    {
        std::cerr << "SDL_CreateTextureFromSurface failed for '" << path << "': " << SDL_GetError() << '\n';
    }

    return texture;
}
}

bool Engine::init(const char* title, int width, int height)
{
    // Cache window size for later full-screen draw rectangles.
    windowWidth = width;
    windowHeight = height;

    // SDL3 returns true on success, false on failure.
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        // Print SDL-provided diagnostic information.
        std::cerr << "SDL_Init failed: " << SDL_GetError() << '\n';
        return false;
    }

    // Create the game window.
    window = SDL_CreateWindow(title, width, height, 0);
    if (!window)
    {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << '\n';
        // SDL must be shut down if initialization partially succeeded.
        SDL_Quit();
        return false;
    }

    // Create a renderer attached to the window.
    renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer)
    {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << '\n';
        // Clean up already-created resources on failure.
        SDL_DestroyWindow(window);
        window = nullptr;
        SDL_Quit();
        return false;
    }

    // Load background textures used by each game state.
    roamingBackground = loadTexture(renderer, "background.png");
    battleBackground = loadTexture(renderer, "battleground.png");

    // Scene owns world objects/player/battle frameworks.
    scene.initialize(renderer, width, height);

    // Engine is ready to enter run loop.
    return true;
}

void Engine::run()
{
    // Target a nominal 60 FPS.
    constexpr float targetFrameTime = 1.0f / 60.0f;
    // Last frame timestamp for delta-time calculation.
    Uint64 lastCounter = SDL_GetTicksNS();

    // Standard game loop: input -> update -> render with delta-time.
    while (running)
    {
        // Timestamp at frame start.
        Uint64 frameStart = SDL_GetTicksNS();
        // Delta time in seconds from previous frame.
        float deltaTime = static_cast<float>(frameStart - lastCounter) / 1000000000.0f;
        // Store current timestamp for next frame's delta.
        lastCounter = frameStart;

        // Poll OS/SDL events and refresh keyboard state.
        input.update(running);

        // Global hotkeys:
        // O => roaming mode
        // P => battle mode
        // A => accept/confirm action (currently debug log)
        // B => back/cancel action (currently debug log)
        if (input.wasKeyPressed(SDLK_O))
        {
            if (gameState == GameState::Battle)
            {
                // Keep scene state consistent when forcing a battle exit.
                scene.onBattleExited();
            }
            gameState = GameState::Roaming;
            std::cout << "State: Roaming\n";
        }
        if (input.wasKeyPressed(SDLK_P))
        {
            if (gameState == GameState::Roaming)
            {
                // Ensure battle mode has a valid monster when forced by dev key.
                scene.onBattleEntered(MonsterType::Red);
            }
            gameState = GameState::Battle;
            std::cout << "State: Battle\n";
        }
        if (input.wasKeyPressed(SDLK_A))
        {
            std::cout << "A pressed: accept/yes/select\n";
        }
        if (input.wasKeyPressed(SDLK_B))
        {
            std::cout << "B pressed: back/no/decline\n";
        }

        // Scene pipeline:
        // 1) Process state-specific input
        // 2) Update state-specific logic
        if (gameState == GameState::Roaming)
        {
            scene.processInput(input, Scene::Mode::Roaming);
            scene.update(deltaTime, Scene::Mode::Roaming);

            MonsterType encounter{};
            if (scene.consumeRequestedBattleStart(encounter))
            {
                gameState = GameState::Battle;
                scene.onBattleEntered(encounter);
                std::cout << "Encounter started.\n";
            }
        }
        else
        {
            scene.processInput(input, Scene::Mode::Battle);
            scene.update(deltaTime, Scene::Mode::Battle);

            if (scene.consumeRequestedBattleExit())
            {
                scene.onBattleExited();
                gameState = GameState::Roaming;
                std::cout << "Returned to roaming.\n";
            }
        }

        // Clear to fallback black. Background texture draw happens next.
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Choose background based on current game mode.
        SDL_Texture* stateBackground = (gameState == GameState::Battle) ? battleBackground : roamingBackground;
        if (stateBackground)
        {
            // Fill the entire window with the chosen background texture.
            SDL_FRect backgroundDest = {0.0f, 0.0f, static_cast<float>(windowWidth), static_cast<float>(windowHeight)};
            SDL_RenderTexture(renderer, stateBackground, nullptr, &backgroundDest);
        }

        if (gameState == GameState::Roaming)
        {
            scene.render(renderer, Scene::Mode::Roaming);
        }
        else
        {
            scene.render(renderer, Scene::Mode::Battle);
        }
        // Present rendered frame to the window.
        SDL_RenderPresent(renderer);

        // Cap frame rate while accounting for time already spent this frame.
        Uint64 frameEnd = SDL_GetTicksNS();
        // Duration of this frame so far, in seconds.
        float frameSeconds = static_cast<float>(frameEnd - frameStart) / 1000000000.0f;
        // Delay only if frame finished earlier than target frame time.
        if (frameSeconds < targetFrameTime)
        {
            Uint32 delayMs = static_cast<Uint32>((targetFrameTime - frameSeconds) * 1000.0f);
            SDL_Delay(delayMs);
        }
    }
}

void Engine::clean()
{
    // Destroy in reverse order of creation where practical.
    // SDL destroy calls are safe with nullptr.
    SDL_DestroyTexture(roamingBackground);
    SDL_DestroyTexture(battleBackground);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    // Shut down SDL global state.
    SDL_Quit();
}
