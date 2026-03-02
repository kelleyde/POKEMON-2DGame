#include "components.hpp"

#include <SDL3_image/SDL_image.h>
#include <iostream>


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


void SpriteComponent::setSize(float width, float height)
{
    destRect.w = width;
    destRect.h = height;
}

void SpriteComponent::drawAt(const SDL_FRect& destination)
{
    if (!texture || !ownerRenderer)
    {
        return;
    }

    SDL_RenderTexture(ownerRenderer, texture, useSourceRect ? &sourceRect : nullptr, &destination);
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
