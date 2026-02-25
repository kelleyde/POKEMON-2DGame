#pragma once
#include <memory>
#include <vector>
#include "GameObject.hpp"
#include "InputHandler.hpp"

// Scene owns game objects and forwards update/render to each object.
class Scene {
public:
    // Adds an object to the scene.
    // Ownership is transferred to Scene via unique_ptr.
    void addObject(std::unique_ptr<GameObject> obj);
    // Calls update on all objects once per frame.
    void update(InputHandler& input, float deltaTime);
    // Calls render on all objects once per frame.
    void render(SDL_Renderer* renderer);

private:
    // Owning container for all active scene entities.
    std::vector<std::unique_ptr<GameObject>> objects;
};
