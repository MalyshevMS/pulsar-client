#pragma voice
#pragma once
#include <string>
#include <SFML/Graphics.hpp>
#include "../Other/Datetime.hpp"
#include <iostream>
class MessageBox
{
private:
    std::string text = " ";
    int size;
    float x;
    float y;
    sf::Vector2f position;
    Datetime msgTime;
    float MsgBoxWidth;
    float MsgBoxHeight;

public:
    MessageBox(float pos_x, float pox_y, int size, std::string text, Datetime msgTime)
    {
        this->x = pos_x;
        this->y = pox_y;
        this->size = size;
        this->text = text;
        this->msgTime = msgTime;
    }
    void DrawMessageBox(sf::RenderWindow &window)
    {
        auto win_size = window.getSize();
        sf::Font font;
        if (!font.openFromFile("res/arial.ttf"))
        {
            std::cout << "Ошибка загрузки шрифта" << std::endl;
        }
        sf::RectangleShape MsgBox(sf::Vector2f();
        MsgBox.setPosition({})
    }
};