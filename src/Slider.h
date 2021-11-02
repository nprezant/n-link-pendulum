#ifndef PENDULUM_SLIDER_H
#define PENDULUM_SLIDER_H

#include "Icon.h"
#include "Rectangle.h"
#include "Text.h"
#include "Texture.h"
#include <SDL2/SDL.h>
#include <string>

class Slider
{
public:
    struct Range
    {
        double start;
        double stop;
        double stride;
        int count() { return (stop - start) / stride; }

        Range(double start, double stop, double stride)
            : start(start)
            , stop(stop)
            , stride(stride)
        {
        }
    };

    Slider(
        const SDL_Rect& rect,
        const std::string& label,
        const Range& range,
        const double initialValue,
        const SDL_Color& bgColor,
        SDL_Renderer* renderer,
        TTF_Font* font);

    SDL_Rect rect();
    void setRect(const SDL_Rect& rect);

    void render();
    void handleEvent(SDL_Event& e);

    double value();
    void setValue(double newValue);
    bool valueChanged();

    int computeThumbOffset(int barWidth);
    void computePositions();

private:
    std::string toString(double val);

    SDL_Rect rect_;
    Rectangle background_;
    Rectangle bar_;
    Icon thumb_;
    Text label_;
    Text displayValue_;
    Range range_;
    SDL_Renderer* renderer_;
    double value_;
    bool valueChanged_;
};

#endif // PENDULUM_SLIDER_H