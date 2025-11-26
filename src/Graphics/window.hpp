#pragma once
#include <string>
#include <thread>
#include <SFML/Graphics.hpp>

#pragma GCC diagnostic ignored "-Wunused-result"

class Window
{
private:
    unsigned int width; // ГИПОТАЛАМУС
    unsigned int height;
    std::string title;
    sf::RenderWindow *win;
    bool fullscreen = false;
    bool running = false;
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
    std::thread winThread;

public:
    Window(const std::string &title, unsigned int width, unsigned int height)
        : title(title), width(width), height(height)
    {
    }

    ~Window()
    {
        stop();
    }

    void proceedEvent(const std::optional<sf::Event> event)
    {
        // TODO: обработка событий тут
    }

    void draw()
    {
        // TODO: отрисовка через win->draw(...)
    }

    void loop()
    {
        sf::RenderWindow window(sf::VideoMode({width, height}), title);
        win = &window;
        while (running && window.isOpen())
        {
            if (const std::optional event = window.pollEvent())
            {
                if (event->is<sf::Event::Closed>())
                {
                    stop();
                }
                proceedEvent(event);
            }

            window.clear();
            draw();
            window.display();
        }
    }
    int GetSizeY()
    {
        return height;
    }
    int GetSizeX()
    {
        return width;
    }
    void run()
    {
        running = true;
        winThread = std::thread(&Window::loop, this);
    }

    void stop()
    {
        running = false;
        if (win->isOpen())
            win->close();
    }

    bool isRunning() const
    {
        return running;
    }
};