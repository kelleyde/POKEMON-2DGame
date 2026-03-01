#pragma once
#include <SDL3/SDL.h>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>
#include "components.hpp"
#include "InputHandler.hpp"

// Shared stat block used by player/monsters.
// This is framework-level data for future combat logic.
struct CombatStats {
    int maxHP = 10;
    int currentHP = 10;
    int attack = 5;
    int defense = 5;
    int speed = 5;
};

// GameObject is the base for all in-world entities.
// It owns optional components, supports collision bounds, and exposes
// optional interaction hooks for things like boxes/signs/NPCs.
class GameObject {
public:
    virtual ~GameObject() = default;

    // Attach a new component instance and transfer ownership to this object.
    template <typename T, typename... Args>
    T* addComponent(Args&&... args)
    {
        static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
        auto component = std::make_unique<T>(std::forward<Args>(args)...);
        component->setOwner(this);
        T* ptr = component.get();
        components.push_back(std::move(component));
        return ptr;
    }

    // Find the first attached component matching T.
    template <typename T>
    T* getComponent()
    {
        for (auto& c : components)
        {
            if (auto casted = dynamic_cast<T*>(c.get()))
                return casted;
        }
        return nullptr;
    }

    // Per-frame behavior update.
    virtual void update(InputHandler& input, float deltaTime) = 0;
    // Per-frame render entry point.
    virtual void render(SDL_Renderer* renderer)
    {
        renderComponents(renderer);
    }

    // Called when the player interacts with this object.
    // Default implementation is no-op.
    virtual void onInteract()
    {
    }

    // Returns true for objects that should block player movement.
    virtual bool blocksMovement() const
    {
        return true;
    }

    // Access the current collision box.
    const SDL_FRect& getCollisionBox() const
    {
        return collisionBox;
    }

    // Set collision box in world-space.
    void setCollisionBox(const SDL_FRect& box)
    {
        collisionBox = box;
    }

    // Optional stat block access (used by player/monster frameworks).
    CombatStats& getStats()
    {
        return stats;
    }

    const CombatStats& getStats() const
    {
        return stats;
    }

protected:
    // Update all attached components.
    void updateComponents(float deltaTime)
    {
        for (auto& c : components)
            c->update(deltaTime);
    }

    // Render all attached components.
    void renderComponents(SDL_Renderer* renderer)
    {
        for (auto& c : components)
            c->render(renderer);
    }

private:
    // Owned component storage.
    std::vector<std::unique_ptr<Component>> components;

    // Axis-aligned collision volume for this object in world coordinates.
    SDL_FRect collisionBox{0.0f, 0.0f, 0.0f, 0.0f};

    // Framework stat data for player/monsters.
    CombatStats stats{};
};
