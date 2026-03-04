#pragma once

#include "GameObject.hpp"
#include "components.hpp"
#include <string>

// For now it renders a placeholder frame from sprite.png and prints "open" when interacted with.
class Box : public GameObject {
public:
    Box(
        SDL_Renderer* renderer,
        float x,
        float y,
        const SDL_FRect& sourceFrame,
        const std::string& firstInteractText,
        const std::string& repeatInteractText = "");
    void update(InputHandler& input, float deltaTime) override;
    void render(SDL_Renderer* renderer) override;
    void onInteract() override;
    bool isOpened() const;
    const std::string& getInteractText() const;

private:
    SpriteComponent* sprite = nullptr;
    bool opened = false;
    std::string firstInteractText;
    std::string repeatInteractText;
    std::string currentInteractText;
};
