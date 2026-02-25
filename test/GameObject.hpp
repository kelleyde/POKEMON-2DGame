#pragma once
#include <SDL3/SDL.h>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>
#include "components.hpp"
#include "InputHandler.hpp"

// Polymorphic base type for anything that updates/renders in the scene.
// Also owns optional reusable components (sprite, transform, etc.).
class GameObject {
public:
    // Polymorphic base class requires virtual destructor.
    virtual ~GameObject() = default;

    // Attach a new component of type T and forward constructor args.
    // Returns a raw pointer for quick access, while ownership remains internal.
    template <typename T, typename... Args>
    T* addComponent(Args&&... args)
    {
        // Enforce component inheritance at compile time.
        static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
        
        // Create component and take unique ownership.
        auto component = std::make_unique<T>(std::forward<Args>(args)...);

        // Set back-reference so component can access its owner.
        component->setOwner(this);

        // Preserve stable raw pointer for caller convenience.
        T* ptr = component.get();

        // Store in owning component list.
        components.push_back(std::move(component));
        return ptr;
    }

    // Find first attached component matching type T.
    // Returns nullptr when component is not attached.
    template <typename T>
    T* getComponent()
    {
        for (auto& c : components)
        {
            // Safe runtime cast across component inheritance tree.
            if (auto casted = dynamic_cast<T*>(c.get()))
                return casted;
        }
        return nullptr;
    }

    // Per-frame gameplay behavior update.
    virtual void update(InputHandler& input, float deltaTime) = 0;
    // Per-frame render entry point.
    // Default behavior renders all attached components.
    virtual void render(SDL_Renderer* renderer)
    {
        renderComponents(renderer);
    }

protected:
    // Update all attached components (logic pass).
    void updateComponents(float deltaTime)
    {
        for (auto& c : components)
            c->update(deltaTime);
    }

    // Render all attached components (draw pass).
    void renderComponents(SDL_Renderer* renderer)
    {
        for (auto& c : components)
            c->render(renderer);
    }

private:
    // Owning component storage.
    std::vector<std::unique_ptr<Component>> components;
};
