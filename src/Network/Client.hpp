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
#include "../Other/Fastfetch.hpp"
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

    void displayUnreadMessages() {
        auto msgs = api.getUnread();
        if (msgs.size() == 0) {
            std::cout << "У вас нет непрочитанных сообщений." << std::endl;
            return;
        }
        std::cout << "У вас есть " << msgs.size() << " непрочитанных сообщений: " << std::endl;
        for (auto i : msgs) {
            std::cout << i << std::endl;
        }
    }
public:
    Client(const std::string& name, const std::string& password_unhashed, const std::string& ip, unsigned short p)
     : name(name), serverIP(ip), port(p), connected(false), api(socket, name), window("Pulsar", 1280, 720) {
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
        displayUnreadMessages();
        std::cout << "------------------------" << std::endl;
        
        std::string message;
        dest = ":all";
        while (connected) {
            std::cout << "[" << name << "](" << dest << ") > " << std::flush;
            std::getline(std::cin, message);
            
            if (!connected) break;
            
            if (message == "!exit") {
                api.sendUnread();
                std::cout << "Отключение..." << std::endl;
                disconnect();
                break;
            }
            else if (message.substr(0, 5) == "!dest") {
                if (!api.isChannelMember(message.substr(6))) continue;
                dest = message.substr(6);
                continue;
            }
            else if (message.substr(0, 5) == "!join") {
                api.joinChannel(message.substr(6));
                dest = message.substr(6);
                continue;
            }
            else if (message.substr(0, 6) == "!leave") {
                api.leaveChannel(message.substr(7));
                dest = ":all";
                continue;
            }
            else if (message.substr(0, 5) == "!chat") {
                auto arg = message.substr(6);
                if (!api.isChannelMember(arg)) continue;
                std::cout << '\n' << api.getChat(arg).to_stream().rdbuf() << std::endl;
                continue;
            }
            else if (message.substr(0, 7) == "!create") {
                auto arg = message.substr(8);
                api.createChannel(arg);
                dest = arg;
                continue;
            }
            else if (message.substr(0, 8) == "!contact") {
                auto com = message.substr(9, 3);
                auto arg = message.substr(13);
                if (com == "add") {
                    if (arg.find(' ') == std::string::npos) {
                        std::cout << "Использование: !contact add <username> <contact>" << std::endl;
                        continue;
                    }
                    auto name = arg.substr(0, arg.find(' '));
                    auto cont = arg.substr(arg.find(' ') + 1);
                    api.createContact(name, cont);
                } else if (com == "rem") {
                    api.removeContact(arg);
                }
                continue;
            }
            else if (message.substr(0, 8) == "!profile") {
                auto arg = message.substr(9);
                if (arg == "edit") {
                    std::string description;
                    std::string email;
                    std::string realName;
                    std::string birthday_str;

                    std::cout << "Введите новое описание: ";
                    std::getline(std::cin, description);
                    std::cout << "Введите новый email: ";
                    std::getline(std::cin, email);
                    std::cout << "Введите новое имя (настоящее): ";
                    std::getline(std::cin, realName);
                    std::cout << "Введите новый день рождения (одно число): ";
                    std::getline(std::cin, birthday_str);
                    auto birthday = std::stoll(birthday_str);

                    api.updateProfile(Profile(description, email, realName, Datetime(birthday)));
                } else {
                    std::cout << "Если вы ожидаете слишком долго, значит профиля не существует." << std::endl;
                    auto profile = api.getProfile(arg);
                    std::cout << "Профиль пользователя " << arg << ":" \
                    << "\n\tОписание: " << profile.description() \
                    << "\n\tEmail: " << profile.email() \
                    << "\n\tИмя: " << profile.realName() \
                    << "\n\tДень рождения: " << profile.birthday().toTime() \
                    << std::endl;
                }
                continue;
            }
            else if (message == "!unread") {
                displayUnreadMessages();
                continue;
            }
            else if (message.substr(0, 5) == "!read") {
                if (message.find(' ') == std::string::npos) {
                    std::cout << "Использование: !read <chat> <id>" << std::endl;
                    continue;
                }
                auto rest = message.substr(6);
                auto chat = rest.substr(0, rest.find(' '));
                if (chat == "all") {
                    api.readAll(dest);
                    std::cout << "Все сообщения в чате " << dest << " помечены как прочитанные." << std::endl;
                    continue;
                }

                auto id_str = rest.substr(rest.find(' ') + 1);
                size_t id = 0;
                try {
                    id = std::stoull(id_str);
                } catch (...) {
                    std::cout << "Неправильный ID сообщения: " << id_str << std::endl;
                    continue;
                }
                api.read(chat, id);
                std::cout << "Сообщение с ID " << id << " в чате " << chat << " помечено как прочитанные." << std::endl;
                continue;
            }
            else if (message == "!fastfetch") {
                const std::vector<std::string> info = {
                    "Pulsar Client " + std::string(PULSAR_VERSION),
                    "Пользователь: " + name,
                    "Сервер: " + serverIP + ":" + std::to_string(port)
                };
                fastfetch(info);
                continue;
            }
            #ifdef PULSAR_DEBUG
            else if (message == "!l") {
                std::cout << "\n[DEBUG]: " << api.getLastResponse() << std::endl;
                continue;
            }
            else if (message == "!cldb") {
                std::cout << api.getDbString() << std::endl;
                continue;
            }
            #endif
            else if (message == "!help") {
                std::cout << "\tДоступные команды:\n"
                             "!exit                                         - Отключиться и выйти\n"
                             "!dest <channel/user>                          - Сменить назначение сообщений\n"
                             "!join <channel>                               - Присоединиться к каналу\n"
                             "!leave <channel>                              - Покинуть канал\n"
                             "!create <channel>                             - Создать канал\n"
                             "!chat <channel/user>                          - Посмотреть историю чата\n"
                             "!profile <username>|edit                      - Посмотреть или изменить профиль\n"
                             "!contact <add/rem> <username> <contact>       - Добавить или удалить контакт\n"
                             "!unread                                       - Посмотреть непрочитанные сообщения\n"
                             "!read <chat> <id>|all                         - Прочитать сообщение по ID из чата\n"
                             "!fastfetch                                    - Вывести информацию о клиенте\n"
                             "!help                                         - Вывести это окно\n"
                             "\tDebug-команды (доступны только в Debug-сборках):\n"
                             "!l                                            - Последний ответ от сервера\n"
                             "!cldb                                         - Клиентская база данных\n"
                << std::flush;
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