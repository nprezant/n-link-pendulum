#ifndef RECTANGLE_H
#define RECTANGLE_H

#include <SDL2/SDL.h>

// UI rectangle element
// Just a rectangle with a color
// Alpha channel is nice btw.
class Rectangle
{
public:
    Rectangle(const SDL_Rect& rect, const SDL_Color& color);

    void render(SDL_Renderer* renderer) const;

private:
    SDL_Rect rect_;
    SDL_Color color_;
};

#endif // RECTANGLE_H