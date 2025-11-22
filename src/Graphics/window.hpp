#pragma once

#include "WindowBase.hpp"

class Window : public WindowBase {
private:

public:
    Window(const std::string& title, unsigned int width, unsigned int height)
     : WindowBase(title, width, height) {}

    virtual void proceedEvent(const sf::Event& event) override {
    
    }

    virtual void draw() override {
    
    }
};