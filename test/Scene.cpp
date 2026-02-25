#include "Scene.hpp"

void Scene::addObject(std::unique_ptr<GameObject> obj)
{
    // Move ownership into the scene container.
    objects.push_back(std::move(obj));
}

void Scene::update(InputHandler& input, float deltaTime)
{
    // Update every object in insertion order.
    // (Later: you can split by systems/layers for performance.)
    for (auto& obj : objects)
        obj->update(input, deltaTime);
}

void Scene::render(SDL_Renderer* renderer)
{
    // Render every object in insertion order.
    // (Later: sort by y/layer for Pokemon-like depth.)
    for (auto& obj : objects)
        obj->render(renderer);
}
