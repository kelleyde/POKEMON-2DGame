#include "components.hpp"

#include <SDL3_image/SDL_image.h>
#include <iostream>
#include "InputHandler.hpp"

// COMPONENT - BASE CLASS IMPLEMENTATION
// Internal: Register this component with its owning GameObject.
// Called automatically when attaching a component via GameObject::addComponent().
void Component::setOwner(GameObject* go)
{
    owner = go;
}

// SPRITE COMPONENT - TEXTURE RENDERING IMPLEMENTATION
// Destructor: clean up SDL texture when component is destroyed.
SpriteComponent::~SpriteComponent()
{
    SDL_DestroyTexture(texture);
}

// Load a texture from disk and configure rendering parameters.
// Converts a CPU-side surface to a GPU texture for fast rendering.
bool SpriteComponent::loadSprite(SDL_Renderer* renderer, const char* path, float width, float height)
{
    // Cache the renderer for later render calls.
    ownerRenderer = renderer;
    
    // Set the default on-screen display size.
    destRect.w = width;
    destRect.h = height;

    // Load image file into a CPU-side surface first.
    SDL_Surface* surface = IMG_Load(path);
    if (!surface)
    {
        std::cerr << "IMG_Load failed for '" << path << "': " << SDL_GetError() << '\n';
        return false;
    }

    // Convert surface to a GPU texture for hardware-accelerated rendering.
    texture = SDL_CreateTextureFromSurface(renderer, surface);

    // Surface is no longer needed after upload to GPU.
    SDL_DestroySurface(surface);
    if (!texture)
    {
        std::cerr << "SDL_CreateTextureFromSurface failed: " << SDL_GetError() << '\n';
        return false;
    }

    return true;
}

// Update the on-screen position for this sprite.
void SpriteComponent::setPosition(float x, float y)
{
    // Update the destination rectangle's top-left corner.
    destRect.x = x;
    destRect.y = y;
}

// Define the source rectangle for sprite sheet cropping.
// Allows rendering a specific frame/tile from an atlas.
void SpriteComponent::setSourceRect(int x, int y, int w, int h)
{
    // Set the source region in the texture.
    sourceRect.x = x;
    sourceRect.y = y;
    sourceRect.w = w;
    sourceRect.h = h;
    
    // Enable cropped rendering mode (instead of rendering the entire texture).
    useSourceRect = true;
}

// Adjust the on-screen display size without changing position.
void SpriteComponent::setSize(float width, float height)
{
    destRect.w = width;
    destRect.h = height;
}

// Render this sprite at a custom destination rectangle.
// Useful for drawing at positions other than the configured position.
void SpriteComponent::drawAt(const SDL_FRect& destination)
{
    // Guard against uninitialized renderer or missing texture.
    if (!texture || !ownerRenderer)
    {
        return;
    }

    // Render using either the full texture or a cropped source region.
    SDL_RenderTexture(ownerRenderer, texture, useSourceRect ? &sourceRect : nullptr, &destination);
}

// Per-frame render hook called by GameObject.
void SpriteComponent::render(SDL_Renderer*)
{
    // Guard against uninitialized renderer or missing texture.
    if (!texture || !ownerRenderer)
    {
        return;
    }

    // Render the sprite using either the full texture or a specific atlas frame.
    SDL_RenderTexture(ownerRenderer, texture, useSourceRect ? &sourceRect : nullptr, &destRect);
}

// DIALOGUE BOX - STATE MANAGEMENT AND RENDERING

// Show the dialogue box with the given text message.
void showDialogueBox(DialogueBoxState& state, const std::string& text)
{
    state.visible = true;
    state.text = text;
}

// Hide the currently visible dialogue box.
void clearDialogueBox(DialogueBoxState& state)
{
    state.visible = false;
    state.text.clear();
}

// Check if dialogue is visible; if so, and A button is pressed, dismiss it.
// This provides the standard player interaction for closing dialogue.
void clearDialogueBoxOnA(DialogueBoxState& state, const InputHandler& input)
{
    if (state.visible && input.wasKeyPressed(SDLK_A))
    {
        clearDialogueBox(state);
    }
}

// Render the dialogue modal and message text.
// Draws a semi-transparent panel at the bottom of the screen with the
// dialogue text, border, and an optional button prompt.
void renderDialogueBox(SDL_Renderer* renderer, SDL_Texture* spriteSheet, int windowWidth, int windowHeight, const DialogueBoxState& state)
{
    // Do nothing if dialogue is not visible.
    if (!state.visible)
    {
        return;
    }

    // Define the dialogue panel rectangle (24px margins, 146px tall).
    const SDL_FRect box = {24.0f, static_cast<float>(windowHeight - 170), static_cast<float>(windowWidth - 48), 146.0f};
    
    // Draw the semi-transparent panel background.
    SDL_SetRenderDrawColor(renderer, 20, 20, 28, 230);
    SDL_RenderFillRect(renderer, &box);
    
    // Draw the white border around the panel.
    SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
    SDL_RenderRect(renderer, &box);

    // Draw the dialogue text (using SDL's debug text rendering).
    SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
    SDL_RenderDebugText(renderer, box.x + 14.0f, box.y + 20.0f, state.text.c_str());

    // Draw the "A" button prompt in the lower-right corner (if sprite sheet available).
    if (spriteSheet)
    {
        // Source region of the "A" tile in the sprite sheet.
        //const SDL_FRect aSrc = {64.0f, 192.0f, 32.0f, 32.0f};
        const SDL_FRect aSrc = {32.0f, 192.0f, 32.0f, 32.0f};


        // Destination near the lower-right of the dialogue box.
        const SDL_FRect aDst = {box.x + box.w - 56.0f, box.y + box.h - 48.0f, 32.0f, 32.0f};
        SDL_RenderTexture(renderer, spriteSheet, &aSrc, &aDst);
    }
}
