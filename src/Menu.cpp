#include "Menu.h"

MainMenu::MainMenu(const SDL_Rect& rect, SDL_Renderer* renderer, TTF_Font* font)
    : rect_(rect)
    , background_(rect, SDL_Color({ 119, 181, 254, 200 }))
    , renderer_(renderer)
{
    const std::vector<ButtonData> buttonDatas{
        { ButtonId::Resume, "Resume" },
        { ButtonId::Options, "Options" },
        { ButtonId::Quit, "Exit" },
    };

    SDL_Color textColor = { 0, 0, 0, 255 };
    SDL_Color textBackgroundColor = { 255, 255, 255, 255 };

    // Setup each renderable Button
    buttons_.reserve(3);
    SDL_Rect r = { rect.x, rect.y, 100, 100 }; // will be overwritten

    for (const auto& data : buttonDatas)
    {
        buttons_.emplace_back(
            r,
            static_cast<int>(data.id),
            data.name,
            textColor,
            textBackgroundColor,
            renderer,
            font);
    }

    // Figure out the button positions
    computePositions();
}

void
MainMenu::setRect(const SDL_Rect& rect)
{
    rect_ = rect;
    background_.setRect(rect);
    computePositions();
}

void
MainMenu::render()
{
    // Render background
    background_.render(renderer_);

    // Render each option
    for (auto& button : buttons_)
        button.render();
}

void
MainMenu::handleEvent(SDL_Event& e, bool& shouldResume, bool& shouldQuit)
{
    for (auto& button : buttons_)
    {
        button.handleEvent(e);
        if (button.wasClicked())
        {
            SDL_LogInfo(
                SDL_LOG_CATEGORY_APPLICATION,
                "Button '%s' was clicked",
                button.text().c_str());

            switch (static_cast<ButtonId>(button.id()))
            {
                case ButtonId::Resume:
                    shouldResume = true;
                    break;

                case ButtonId::Quit:
                    shouldQuit = true;
                    break;

                default:
                    break;
            }
        }
    }
}

void
MainMenu::computePositions()
{
    int optionWidth = 200;
    int optionHeight = 80;

    int margin = 20;

    // Figure out top left x,y for the group of all options
    int totalHeight = (optionHeight + margin + margin) * buttons_.size();
    int totalWidth = optionWidth + margin + margin; // One column

    int topX = rect_.x + (0.5 * rect_.w) - (0.5 * totalWidth);
    int topY = rect_.y + (0.5 * rect_.h) - (0.5 * totalHeight);

    // Track the top left x, y point of each button
    // This corresponds to the bottom left x, y point of the button prior
    int lastTopX = topX;
    int lastTopY = topY;

    for (auto& button : buttons_)
    {
        int x = lastTopX + margin;
        int y = lastTopY + margin;

        button.setRect({ x, y, optionWidth, optionHeight });

        lastTopX = lastTopX; // No update here, just one column
        lastTopY = y + optionHeight + margin;
    }
}
