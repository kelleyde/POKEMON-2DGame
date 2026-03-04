#include "components.hpp"

#include <SDL3_image/SDL_image.h>
#include <iostream>
#include "InputHandler.hpp"


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

void showDialogueBox(DialogueBoxState& state, const std::string& text)
{
    state.visible = true;
    state.text = text;
}

void clearDialogueBox(DialogueBoxState& state)
{
    state.visible = false;
    state.text.clear();
}

void clearDialogueBoxOnA(DialogueBoxState& state, const InputHandler& input)
{
    if (state.visible && input.wasKeyPressed(SDLK_A))
    {
        clearDialogueBox(state);
    }
}

void renderDialogueBox(SDL_Renderer* renderer, SDL_Texture* spriteSheet, int windowWidth, int windowHeight, const DialogueBoxState& state)
{
    if (!state.visible)
    {
        return;
    }

    const SDL_FRect box = {24.0f, static_cast<float>(windowHeight - 170), static_cast<float>(windowWidth - 48), 146.0f};
    SDL_SetRenderDrawColor(renderer, 20, 20, 28, 230);
    SDL_RenderFillRect(renderer, &box);
    SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
    SDL_RenderRect(renderer, &box);

    // SDL3 debug text to display dialogue lines.
    SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
    SDL_RenderDebugText(renderer, box.x + 14.0f, box.y + 20.0f, state.text.c_str());

    // Draw "A" tile prompt from sprite sheet near the lower-right of the box.
    if (spriteSheet)
    {
        const SDL_FRect aSrc = {64.0f, 192.0f, 32.0f, 32.0f};
        const SDL_FRect aDst = {box.x + box.w - 56.0f, box.y + box.h - 48.0f, 32.0f, 32.0f};
        SDL_RenderTexture(renderer, spriteSheet, &aSrc, &aDst);
    }
}
