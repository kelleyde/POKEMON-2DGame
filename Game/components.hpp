#pragma once

#include <SDL3/SDL.h>

class GameObject;

// Base component type. Components are owned by a GameObject.
class Component {
public:
    // Virtual destructor for safe deletion through base pointer.
    virtual ~Component() = default;
    
    // Optional logic hook executed each frame.
    virtual void update(float deltaTime) {}
    
    // Optional draw hook executed each frame.
    virtual void render(SDL_Renderer* renderer) {}


private:
    // Called by GameObject when the component is attached.
    void setOwner(GameObject* go);

    // GameObject is allowed to call setOwner().
    friend class GameObject;

    // Back-pointer to owner, not owning memory.
    GameObject* owner = nullptr;
};

// Sprite component handles texture loading and sprite rendering.
class SpriteComponent : public Component {
public:
    // Releases owned texture memory.
    ~SpriteComponent() override;

    // Load a texture from file and configure destination draw size.
    bool loadSprite(SDL_Renderer* renderer, const char* path, float width = 64.0f, float height = 64.0f);
    
    // Set top-left draw position in world/screen coordinates.
    void setPosition(float x, float y);
   
    // Set source rectangle (sprite-sheet frame) in pixels.
    void setSourceRect(int x, int y, int w, int h);

    // Set render size without changing position.
    void setSize(float width, float height);

    // Render this sprite at a custom destination rectangle.
    void drawAt(const SDL_FRect& destination);

    // Draw sprite each frame.
    void render(SDL_Renderer* renderer) override;

private:
    // Renderer used for draw calls.
    SDL_Renderer* ownerRenderer = nullptr;
    // Texture owned by this component.
    SDL_Texture* texture = nullptr;
    // Whether to crop source texture region.
    bool useSourceRect = false;
    // Source frame inside texture (for sprite sheets).
    SDL_FRect sourceRect{0.0f, 0.0f, 0.0f, 0.0f};
    // Destination position/size on screen.
    SDL_FRect destRect{0.0f, 0.0f, 64.0f, 64.0f};
};
