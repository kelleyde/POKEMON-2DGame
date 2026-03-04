#pragma once
// GAMEOBJECT - BASE CLASS FOR ALL IN-WORLD Objects
#include <SDL3/SDL.h>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>
#include "components.hpp"
#include "InputHandler.hpp"

//didnt get to use these, game changed to something more cute
struct CombatStats {
    int maxHP = 10;         // Maximum health points
    int currentHP = 10;     // Current health points (decreases when damaged)
    int attack = 5;         // Damage dealt to opponents
    int defense = 5;        // Damage reduction when hit
    int speed = 5;          // Turn order priority (higher is faster)
};


//  Modular component system for attaching behaviors
//  Collision detection bounds
//  Combat stats storage (for combat-capable entities)
//  Virtual lifecycle hooks (update, render, interact)

class GameObject {
public:
    // Virtual destructor for safe deletion through base pointer.
    virtual ~GameObject() = default;

    // COMPONENT MANAGEMENT

    // Create a new component of type T, attach it to this GameObject, 
    // and transfer ownership to this object.
    // 
    // Template Args:
    //   T: component class type (must derive from Component)
    //   Args: parameter types for T's constructor
    //
    // Args:
    //   args: constructor arguments for the component
    //
    // Returns: non-owning pointer to the created component
    //          (use immediately to store or configure; owned by GameObject)
    //
    // Example: sprite = addComponent<SpriteComponent>();
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

    // Find and return the first attached component of type T.
    // Returns nullptr if no component of that type is attached.
    //
    // Template Args:
    // T: component class type to search for
    // Returns: pointer to component, or nullptr if not found
    // Example: if (auto sprite = getComponent<SpriteComponent>()) { ... }
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

    // LIFECYCLE HOOKS 
    // Per-frame behavior update. Override to implement logic.
    // Args:
    // input: current input state
    // deltaTime: elapsed seconds since last frame
    virtual void update(InputHandler& input, float deltaTime) = 0;

    // Per-frame rendering. Override to implement drawing.
    // Default implementation calls renderComponents() to delegate to attached components.
    // Args:
    //   renderer: SDL renderer for draw calls
    virtual void render(SDL_Renderer* renderer)
    {
        renderComponents(renderer);
    }

    // Called when the player interacts with this object.
    // Default implementation does nothing; deriving classes override as needed.
    virtual void onInteract()
    {
    }

    // COLLISION AND MOVEMENT

    // Query whether this object blocks player movement.
    // Returns true to make this object a solid obstacle.
    // Default: true (most objects are solid).
    virtual bool blocksMovement() const
    {
        return true;
    }

    // Get the current collision box for this object.
    // Used for movement collision detection and interaction probes.
    // Returns: axis-aligned rectangle in world coordinates
    const SDL_FRect& getCollisionBox() const
    {
        return collisionBox;
    }

    // Set the collision box for this object.
    // Args:
    //   box: new collision box in world coordinates
    void setCollisionBox(const SDL_FRect& box)
    {
        collisionBox = box;
    }


    // COMBAT STATS ACCESS
/*
    CombatStats& getStats()
    {
        return stats;
    }

    const CombatStats& getStats() const
    {
        return stats;
    }
*/

protected:

    // COMPONENT HELPERS 

    // Update all attached components.
    // Called by render() and derived classes during their update loop.
    // Args:
    //   deltaTime: elapsed seconds since last frame
    void updateComponents(float deltaTime)
    {
        for (auto& c : components)
            c->update(deltaTime);
    }

    // Render all attached components.
    // Called by render() and derived classes during their render loop.
    // Args:
    //   renderer: SDL renderer for draw calls
    void renderComponents(SDL_Renderer* renderer)
    {
        for (auto& c : components)
            c->render(renderer);
    }

private:

    // Owned component storage (unique_ptr for automatic cleanup).
    std::vector<std::unique_ptr<Component>> components;

    // Axis-aligned collision volume for this object in world coordinates.
    SDL_FRect collisionBox{0.0f, 0.0f, 0.0f, 0.0f};

    // Combat stat block (HP, attack, defense, speed).
    CombatStats stats{};
};
