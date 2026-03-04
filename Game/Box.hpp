#pragma once

#include "GameObject.hpp"
#include "components.hpp"
#include <string>

// BOX - INTERACTIVE WORLD OBJECTS

class Box : public GameObject {
public:

    // Constructor initializes sprite and collision box.
    // Args:
    //   renderer: SDL renderer for sprite loading
    //   x, y: world position for the sprite
    //   sourceFrame: sprite sheet region (x, y, width, height)
    //   firstInteractText: dialogue shown on first interaction
    //   repeatInteractText: dialogue shown on repeat interactions (optional)
    
    Box(
        SDL_Renderer* renderer,
        float x,
        float y,
        const SDL_FRect& sourceFrame,
        const std::string& firstInteractText,
        const std::string& repeatInteractText = "");

    // Per-frame update (currently just updates components).
    void update(InputHandler& input, float deltaTime) override;

    // Per-frame render.
    void render(SDL_Renderer* renderer) override;

    // Called when player interacts with this box.
    // Advances dialogue state and updates current interaction text.
    void onInteract() override;

    // Query whether this box has been opened/interacted with.
    bool isOpened() const;

    // Get the current dialogue text to display.
    const std::string& getInteractText() const;

private:
    SpriteComponent* sprite = nullptr;          // Owns sprite rendering and texture
    bool opened = false;                        // Has this box been interacted with?
    std::string firstInteractText;              // First Dialogue 
    std::string repeatInteractText;             // Next Dialouge
    std::string currentInteractText;            // Currently active dialogue text
};
