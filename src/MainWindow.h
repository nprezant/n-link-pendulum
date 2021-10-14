#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <SDL2/SDL.h>

#include "PendulumProvider.h"
#include "Texture.h"

class MainWindow
{
public:
    MainWindow(int w, int h);

    // Deallocates memory
    // Frees media and shuts down SDL
    ~MainWindow();

    // Starts up SDL and creates window
    bool init();

    // Loads media
    bool loadMedia();

    // Main event loop
    void runLoop();

private:
    // The window we'll be rendering to
    SDL_Window* window_;

    // The window renderer
    SDL_Renderer* renderer_;

    // Scene textures
    Texture dotTexture_;
    Texture fpsTextTexture_;

    // Fonts
    TTF_Font* fpsFont_;

    // The pendulum
    PendulumProvider pendulumProvider_;

    // Screen dimensions
    int w_;
    int h_;
};

#endif // MAINWINDOW_H