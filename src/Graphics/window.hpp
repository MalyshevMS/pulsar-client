#pragma once

#include "WindowBase.hpp"
#include "Chatlist.hpp"

class Window : public WindowBase
{
private:
    Chatlist chatlist = Chatlist({"Alice", "Bob", "Charlie Kirk", "Diana", "Eve", "Frank", "Grace", "Heidi", "Ivan", "Klyde", "Kevin"});

public:
    Window(const std::string &title, unsigned int width, unsigned int height)
        : WindowBase(title, width, height) {}

    virtual void proceedEvent(const sf::Event &event) override
    {
    }

    virtual void draw() override
    {
        chatlist.draw(*win);
    }
};