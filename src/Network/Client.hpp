#pragma once

#include <SFML/Network.hpp>
#include <iostream>
#include <thread>
#include <list>
#include <atomic>
#include <string>

#include "../lib/jsonlib.h"
#include "../lib/hash.h"
#include "../Other/Datetime.hpp"
#include "../Other/Chat.hpp"
#include "../Console/Console.hpp"
#include "../Graphics/Window.hpp"
#include "../API/PulsarAPI.hpp"
#include "Database.hpp"

class Client {
private:
    sf::TcpSocket socket;
    std::string serverIP;
    unsigned short port;
    std::atomic<bool> connected;
    std::atomic<bool> login_success = false;
    std::thread receiveThread;
    std::string dest;
    std::string name;
    std::string password;
    PulsarAPI api;
    Window window;
    Console cmd;
public:
    Client(const std::string& name, const std::string& password_unhashed, const std::string& ip, unsigned short p)
     : name(name), serverIP(ip), port(p), connected(false), api(socket, name), window("Pulsar", 1280, 720), cmd(api, dest, this->name) {
        if (password_unhashed.size() > 0) password = hash(password_unhashed);
        else password = "";

        window.setOnClose([this]() {
            std::thread([this]() {
                this->disconnect();
            }).detach();
        });
    }
    
    bool connectToServer() {
        sf::Socket::Status status = socket.connect(*sf::IpAddress::resolve(serverIP), port, sf::milliseconds(PULSAR_TIMEOUT_MS));
        if (status != sf::Socket::Status::Done) {
            std::cout << "Невозможно подключиться к " << serverIP << ":" << port << std::endl;
            std::cout << "Удостоверьтесь, что сервер работает и доступен." << std::endl;
            return false;
        }
        connected = true;
        return true;
    }
    
    void disconnect() {
        connected = false;
        api.disconnect();
        try { window.stop(); } catch (...) {}
        if (receiveThread.joinable()) {
            receiveThread.join();
        }
    }

    ~Client() {
        connected = false;
        try {
            api.disconnect();
        } catch (...) {}
        try {
#ifdef PULSAR_GUI
            window.stop();
#endif
        } catch (...) {}
        if (receiveThread.joinable()) {
            try {
                receiveThread.join();
            } catch (...) {}
        }
    }
    
    void receiveMessages() {
        try {
            while (connected) {
                auto msg = api.receiveLastMessage();

                if (msg.get_src() == "!server") {
                    api.storeServerResponse(msg.get_msg());
                } else if (login_success) {
                    if ((msg.get_dst() == dest || msg.get_dst() == name)) {
                        std::cout << "\r\x1b[K" << msg << std::endl;
                        std::cout << "[" << name << "](" << dest << ") > " << std::flush;
                    } else {
                        api.storeUnread(msg);
                    }
                }

                sf::sleep(sf::milliseconds(100));
            }
        } catch (const std::exception& e) {
            std::cerr << "Exception in receiveMessages: " << e.what() << std::endl;
        } catch (...) {
            std::cerr << "Unknown exception in receiveMessages" << std::endl;
        }
    }
    
    void run() {
        if (!connectToServer()) {
            return;
        }

        #ifdef PULSAR_GUI
            window.run();
        #endif
        
        receiveThread = std::thread(&Client::receiveMessages, this);

        auto login_status = api.login(password);
        if (login_status == PulsarAPI::LoginResult::Fail_Username) {
            std::cout << "Зарегситрировать нового пользователя? (y/N): ";
            std::string ans;
            std::getline(std::cin, ans);
            if (ans[0] == (char)0 || ans[0] == 'n' || ans[0] == 'N') disconnect();
            if (api.registerUser(password) != PulsarAPI::LoginResult::Success) {
                std::cout << "Ошибка регистрации; Отключение..." << std::endl;
                disconnect();
            }
        } else if (login_status != PulsarAPI::LoginResult::Success) {
            std::cout << "Отключение..." << std::endl;
            disconnect();
        }
        login_success = true;

        std::cout << "Подключено к серверу! Наберите свои сообщения ниже." << std::endl;
        std::cout << "Наберите '!exit' чтобы выйти или '!help' чтобы посмотреть полный список команд." << std::endl;
        std::cout << "Вы вошли в Pulsar как '" << name << "'." << std::endl;
        
        api.requestDb();
        api.requestUnread();
        
        #ifndef PULSAR_GUI
        // todo: move this to GUI or in separate console function/window
        std::cout << "------------------------" << std::endl;
        cmd.displayUnreadMessages();
        std::cout << "------------------------" << std::endl;
        cmd.setTitle("Pulsar - " + name);
        
        std::string message;
        dest = ":all";
        while (connected) {
            std::cout << "[" << name << "](" << dest << ") > " << std::flush;
            std::getline(std::cin, message);
            
            if (!connected) break;
            if (message[0] == '!') {
                int code = cmd.run(message);
                switch (code) {
                    case PULSAR_EXIT_CODE_DISCONNECT:
                        disconnect();
                        break;
                    case PULSAR_EXIT_CODE_NOT_A_MEMBER:
                        std::cout << "Ошибка: Вы не являетесь участником этого канала." << std::endl;
                        break;
                    case PULSAR_EXIT_CODE_INVALID_ARGS:
                        std::cout << "Ошибка: Неверные аргументы команды." << std::endl;
                        break;
                    case PULSAR_EXIT_CODE_INVALID_COMMAND:
                        std::cout << "Ошибка: Неизвестная команда." << std::endl;
                        break;
                    case PULSAR_EXIT_CODE_FAILURE:
                        std::cout << "Ошибка: Не удалось выполнить команду." << std::endl;
                        break;
                    case PULSAR_EXIT_CODE_SUCCESS:
                    default:
                        break;
                }
                continue;
            }
            
            if (!message.empty()) {
                api.sendMessage(message, dest);
            }
        }
        #else
            while (window.isRunning()) {
                sf::sleep(sf::milliseconds(100));
            }
        #endif
    }
};