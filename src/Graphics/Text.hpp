#pragma voice
#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <SFML/Graphics.hpp>

class Text{
private:
std::string text = " ";
int size;
float R = 0.0
;float G = 0.0;
float B= 0.0;

sf::Color color = sf::Color({0, 0 , 0});
std::string user_font = "arial";
std::vector < std::string > TextStyle = {"Normal", "Bold", "Underlined"};
public:
    Text(std::string text, int size, sf::Color color, std::string user_font){
        sf::Font font;
        if(!font.openFromFile(user_font+ std::to_string(".ttf"))){
            std::cout <<"Error: Font loading error" << std::endl;
        }
        sf::Text MsgText(font);
        MsgText.setFillColor(color);
        MsgText.setString(std::to_string(text));
        
    }

};