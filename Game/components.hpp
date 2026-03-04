#pragma once

#include <SDL3/SDL.h>
#include <string>

class GameObject;
class InputHandler;

// COMPONENT - BASE CLASS FOR GAME OBJECT BEHAVIORS


class Component {
public:
    // Virtual destructor for safe deletion through base pointer.
    virtual ~Component() = default;
    
    // Optional per-frame behavior hook. Override to implement logic.
    // Args:
    //   deltaTime: elapsed seconds since last frame
    virtual void update(float deltaTime) {}
    
    // Optional per-frame rendering hook. Override to implement drawing.
    // Args:
    //   renderer: SDL renderer for draw calls
    virtual void render(SDL_Renderer* renderer) {}

private:
    // Internal: Called by GameObject when component is attached to owner.
    void setOwner(GameObject* go);

    // GameObject is allowed to call setOwner().
    friend class GameObject;

    // Back-pointer to owner GameObject (not owning memory).
    GameObject* owner = nullptr;
};

// SPRITE COMPONENT - TEXTURE RENDERING


// - Texture loading from disk (PNG, JPG, etc. via SDL_image)
// - Sprite sheet region selection (source rect)
// - On-screen positioning and sizing
// - Hardware-accelerated rendering through SDL renderer

class SpriteComponent : public Component {
public:
    // Destructor releases owned texture memory.
    ~SpriteComponent() override;

    // Load a texture from file and configure display dimensions.
    // Args:
    //   renderer: SDL renderer for texture creation
    //   path: file path to image (e.g., "sprite.png")
    //   width, height: desired on-screen display size in pixels
    // Returns: true on success, false on file I/O or rendering failure

    bool loadSprite(SDL_Renderer* renderer, const char* path, float width = 64.0f, float height = 64.0f);
    
    // Set the top-left position for drawing this sprite.
    // Args:
    //   x, y: world/screen coordinates
    void setPosition(float x, float y);
   
    // Define the source rectangle for sprite sheet cropping.
    // When set, only this region of the texture is rendered (used for atlases).
    // Args:
    //   x, y, w, h: pixel coordinates in the source texture
    void setSourceRect(int x, int y, int w, int h);

    // Change the on-screen display size without affecting position.
    // Args:
    //   width, height: new display dimensions in pixels
    void setSize(float width, float height);

    // Draw this sprite at a custom destination rectangle.
    // Useful for rendering at positions other than the configured position.
    // Args:
    //   destination: target screen rectangle
    void drawAt(const SDL_FRect& destination);

    // Per-frame render hook (called automatically by GameObject).
    void render(SDL_Renderer* renderer) override;

private:
    SDL_Renderer* ownerRenderer = nullptr;  // Renderer for draw calls (not owning)
    SDL_Texture* texture = nullptr;         // Loaded texture (owned by this component)
    
    // Source rectangle configuration.
    bool useSourceRect = false;             // Use source rect or render whole texture?
    SDL_FRect sourceRect{0.0f, 0.0f, 0.0f, 0.0f};  // Sprite sheet region to draw

    // Destination position and size on screen.
    SDL_FRect destRect{0.0f, 0.0f, 64.0f, 64.0f};
};

// DIALOGUE BOX - MODAL DIALOGUE STATE AND HELPERS
struct DialogueBoxState {
    bool visible = false;           // Is the dialogue box currently shown?
    std::string text{};             // Message text to display
};

// Show dialogue popover and block normal input flow until dismissed.
// Args:
//   state: dialogue state to modify
//   text: message to display
void showDialogueBox(DialogueBoxState& state, const std::string& text);

// Hide the currently visible dialogue box.
void clearDialogueBox(DialogueBoxState& state);

// Check if dialogue is visible and A is pressed; if so, clear it.
// Allows player to dismiss dialogue by pressing the A button.
void clearDialogueBoxOnA(DialogueBoxState& state, const InputHandler& input);

// Render the dialogue panel, message text, and prompt indicator.
// Args:
//   renderer: SDL renderer for draw calls
//   spriteSheet: sprite texture for the A-prompt tile (can be nullptr to skip)
//   windowWidth, windowHeight: screen dimensions for positioning
//   state: dialogue state and text to render
void renderDialogueBox(SDL_Renderer* renderer, SDL_Texture* spriteSheet, int windowWidth, int windowHeight, const DialogueBoxState& state);
