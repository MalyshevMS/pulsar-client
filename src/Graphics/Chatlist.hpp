#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <iostream>
#include "graphic_constants"

class Chatlist
{
private:
    std::vector<std::string> chats;
    unsigned int max_visible_chats = 10;
    sf::Color chat_background_color = sf::Color(50, 50, 50);
    sf::Color bacground_color = sf::Color(30, 30, 100);

public:
    Chatlist(const std::vector<std::string> &chats)
        : chats(chats)
    {
    }

    void addChat(const std::string &chat_name)
    {
        chats.push_back(chat_name);
    }

    void setMaxVisibleChats(unsigned int max_chats)
    {
        max_visible_chats = max_chats;
    }

    unsigned int getMaxVisibleChats() const
    {
        return max_visible_chats;
    }
    
    void setBackgroundColor(const sf::Color &color)
    {
        chat_background_color = color;
    }

    sf::Color getBackgroundColor() const
    {
        return chat_background_color;
    }

    void draw(sf::RenderWindow &window)
    {
        auto size = window.getSize();

        sf::Font font;
        if (!font.openFromFile("res/arial.ttf"))
        {
            std::cout << "Ошибка загрузки шрифта" << std::endl;
        }

        sf::RectangleShape background(sf::Vector2f(size.x / 5.f + 20, size.y));
        background.setPosition({0.0f, GR_PULSAR_BAR_HEIGHT});
        background.setFillColor(bacground_color);
        window.draw(background);

        for (int i = 0; i < std::min((size_t)chats.size(), (size_t)max_visible_chats); i++)
        {
            auto chat = chats[i];
            auto max_chats = max_visible_chats + 1;

            sf::RectangleShape chat_bg(sf::Vector2f(size.x / 5.f, size.y / max_chats - 5));
            chat_bg.setPosition({GR_CHATS_OFFSET_X, GR_CHATS_OFFSET_Y + i * size.y / max_chats});
            chat_bg.setFillColor(chat_background_color);
            window.draw(chat_bg);

            sf::Text chat_text(font);
            chat_text.setString(chat);
            chat_text.setCharacterSize(24);
            chat_text.setFillColor(sf::Color::White);
            float textX = GR_CHATS_OFFSET_X * 2.f;
            float textY = GR_CHATS_OFFSET_Y + i * size.y / max_chats + (size.y / max_chats - 5.f - chat_text.getCharacterSize()) / 2.f;

            chat_text.setPosition({textX, textY});
            window.draw(chat_text);
        }
    }
};