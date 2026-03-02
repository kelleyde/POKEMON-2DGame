#include "Engine.hpp"

// Program entry point for the game process.
// SDL3 on this target uses the normal C/C++ main() function.
int main(int argc, char* argv[])
{
    // Unused right now, but kept to make it easy to add
    // command-line flags later (window size, debug modes, etc.).
    (void)argc;
    (void)argv;

    // Engine owns all high-level systems:
    // - SDL initialization/shutdown
    // - window + renderer
    // - game loop + timing
    // - input polling
    // - scene update + render
    Engine engine;
    
    // Initialize engine and core resources.
    // Return non-zero if startup fails.
    if (!engine.init("Creature", 800, 600))
        return -1;

    // Run the main loop until a quit event is received.
    engine.run();

    // Explicit teardown of SDL resources and subsystems.
    engine.clean();

    // Normal exit status.
    return 0;
}
