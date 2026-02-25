#include "components.hpp"

#include <SDL3_image/SDL_image.h>
#include <iostream>

// Return the GameObject that owns this component.
GameObject* Component::getOwner() const
{
    return owner;
}

// Internal attachment API used only by GameObject.
void Component::setOwner(GameObject* go)
{
    owner = go;
}

// Free owned texture when component is destroyed.
SpriteComponent::~SpriteComponent()
{
    SDL_DestroyTexture(texture);
}

bool SpriteComponent::loadSprite(SDL_Renderer* renderer, const char* path, float width, float height)
{
    // Store renderer to use during render().
    ownerRenderer = renderer;
    // Configure final on-screen sprite size.
    destRect.w = width;
    destRect.h = height;

    // Load image file into CPU surface first.
    SDL_Surface* surface = IMG_Load(path);
    if (!surface)
    {
        std::cerr << "IMG_Load failed for '" << path << "': " << SDL_GetError() << '\n';
        return false;
    }

    // Convert to GPU texture for fast rendering.
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    // Surface is no longer needed after upload.
    SDL_DestroySurface(surface);

    if (!texture)
    {
        std::cerr << "SDL_CreateTextureFromSurface failed: " << SDL_GetError() << '\n';
        return false;
    }

    return true;
}

void SpriteComponent::setPosition(float x, float y)
{
    // Move the destination rectangle's top-left corner.
    destRect.x = x;
    destRect.y = y;
}

void SpriteComponent::moveBy(float dx, float dy)
{
    // Apply frame-relative movement deltas.
    destRect.x += dx;
    destRect.y += dy;
}

void SpriteComponent::setSourceRect(int x, int y, int w, int h)
{
    // Define which frame/region to read from the source texture.
    sourceRect.x = x;
    sourceRect.y = y;
    sourceRect.w = w;
    sourceRect.h = h;
    // Enable cropped source rendering mode.
    useSourceRect = true;
}

void SpriteComponent::clearSourceRect()
{
    // Render full texture region instead of cropped frame.
    useSourceRect = false;
}

SDL_FRect* SpriteComponent::getRect()
{
    // Gives external systems direct access to sprite destination.
    return &destRect;
}

void SpriteComponent::render(SDL_Renderer*)
{
    // If no texture or renderer is available, skip draw safely.
    if (!texture || !ownerRenderer)
    {
        return;
    }

    // Draw using either full texture or a specific source frame.
    SDL_RenderTexture(ownerRenderer, texture, useSourceRect ? &sourceRect : nullptr, &destRect);
}
