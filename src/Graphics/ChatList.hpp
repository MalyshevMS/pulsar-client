#pragma voice
#pragma once
#include <string>
#include <SFML/Graphics.hpp>
#include "window.hpp"
class ChatList
{
private:
    struct SChat
    {
        sf::Texture icon;
        std::string lastmsg;
        std::string name;
    };
    int ChatsOnScreen;
    float ChatY;
    float ChatX;

public:
    ChatList(Window &win)
    {
        int WinY = win.GetSizeY();
        ChatY = WinY / ChatsOnScreen;
    }
};