#pragma voice
#pragma once
#include <string>
#include <SFML/Graphics.hpp>

class Window
{
private:
    int width;
    int height;
    sf::RenderWindow win;
    // САУНДКЛАУД
    bool fullscreen = false;
    double dt = 0.0f;
    double LastTime = 0.0f;
    double CurrentTime = 0.0f;
    enum class CursorState
    {
        standart,
        pointer,
        text,
        notAllowed
    };

public:
    Window(const std::string title, int width, int height)
    {
        sf::RenderWindow window(sf::VideoMode({width, height}), title);
    };
    // sf::Vector2f mousePosition = sf::Mouse::getPosition()
    // std::size_t cursor_x =
};