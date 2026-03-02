#pragma once

#include "GameObject.hpp"
#include "components.hpp"

// Box is a simple interactable world object framework.
// For now it renders a placeholder frame from sprite.png and prints
// "open" when interacted with.
class Box : public GameObject {
public:
    Box(SDL_Renderer* renderer, float x, float y);

    void update(InputHandler& input, float deltaTime) override;

    void render(SDL_Renderer* renderer) override;
    
    void onInteract() override;

private:
    SpriteComponent* sprite = nullptr;
    bool opened = false;
};

