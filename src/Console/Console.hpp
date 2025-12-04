#pragma once

#include <iostream>
#include <string>
#include "console_defines"
#include "Fastfetch.hpp"
#include "../defines"
#include "../API/PulsarAPI.hpp"

class Console {
private:
    PulsarAPI& api;
    std::string& dest;
    std::string& name;

    static bool checkLength(const std::string& str, size_t min) {
        return !(str.size() >= min);
    }
public:
    Console(PulsarAPI& api_ref, std::string& dest_ref, std::string& name_ref)
     : api(api_ref), dest(dest_ref), name(name_ref) {}

    static void clear() {
        #ifdef _WIN32
            std::system("cls");
        #else
            std::system("clear");
        #endif
    }

    static void setTitle(const std::string& title) {
        #ifdef _WIN32
            std::string command = "title " + title;
            std::system(command.c_str());
        #else
            std::cout << "\033]0;" << title << "\007";
        #endif
    }

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

    int run(const std::string& command) {
        if (command == "!exit") {
            api.sendUnread();
            std::cout << "Отключение..." << std::endl;
            return PULSAR_EXIT_CODE_DISCONNECT;
        }
        else if (command == "!clear") {
            clear();
        }
        else if (command.substr(0, 5) == "!dest") {
            if (checkLength(command, 7)) return PULSAR_EXIT_CODE_INVALID_ARGS;
            if (!api.isChannelMember(command.substr(6))) return PULSAR_EXIT_CODE_NOT_A_MEMBER;
            dest = command.substr(6);
        }
        else if (command.substr(0, 5) == "!join") {
            if (checkLength(command, 7)) return PULSAR_EXIT_CODE_INVALID_ARGS;
            if (!api.joinChannel(command.substr(6))) return PULSAR_EXIT_CODE_FAILURE;
            dest = command.substr(6);
        }
        else if (command.substr(0, 6) == "!leave") {
            if (checkLength(command, 8)) return PULSAR_EXIT_CODE_INVALID_ARGS;
            if (!api.leaveChannel(command.substr(7))) return PULSAR_EXIT_CODE_FAILURE;
            dest = ":all";
        }
        else if (command.substr(0, 5) == "!chat") {
            if (checkLength(command, 7)) return PULSAR_EXIT_CODE_INVALID_ARGS;
            auto arg = command.substr(6);
            if (!api.isChannelMember(arg)) return PULSAR_EXIT_CODE_NOT_A_MEMBER;
            std::cout << '\n' << api.getChat(arg).to_stream().rdbuf() << std::endl;
        }
        else if (command.substr(0, 7) == "!create") {
            if (checkLength(command, 9)) return PULSAR_EXIT_CODE_INVALID_ARGS;
            auto arg = command.substr(8);
            if (!api.createChannel(arg)) return PULSAR_EXIT_CODE_FAILURE;
            dest = arg;
        }
        else if (command.substr(0, 8) == "!contact") {
            if (checkLength(command, 15)) return PULSAR_EXIT_CODE_INVALID_ARGS;
            auto com = command.substr(9, 3);
            auto arg = command.substr(13);
            if (com == "add") {
                auto name = arg.substr(0, arg.find(' '));
                auto cont = arg.substr(arg.find(' ') + 1);
                api.createContact(name, cont);
            } else if (com == "rem") {
                api.removeContact(arg);
            }
        }
        else if (command.substr(0, 8) == "!profile") {
            if (checkLength(command, 10)) return PULSAR_EXIT_CODE_INVALID_ARGS;
            auto arg = command.substr(9);
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
        }
        else if (command == "!unread") {
            displayUnreadMessages();
        }
        else if (command.substr(0, 5) == "!read") {
            if (checkLength(command, 7)) return PULSAR_EXIT_CODE_INVALID_ARGS;
            auto rest = command.substr(6);
            auto chat = rest.substr(0, rest.find(' '));
            if (chat == "all") {
                api.readAll(dest);
                std::cout << "Все сообщения в чате " << dest << " помечены как прочитанные." << std::endl;
                return PULSAR_EXIT_CODE_SUCCESS;
            }

            auto id_str = rest.substr(rest.find(' ') + 1);
            size_t id = 0;
            try {
                id = std::stoull(id_str);
            } catch (...) {
                std::cout << "Неправильный ID сообщения: " << id_str << std::endl;
                return PULSAR_EXIT_CODE_INVALID_ARGS;
            }
            api.read(chat, id);
            std::cout << "Сообщение с ID " << id << " в чате " << chat << " помечено как прочитанные." << std::endl;
        }
        else if (command == "!fastfetch") {
            const std::vector<std::string> info = {
                "Pulsar Client " + std::string(PULSAR_VERSION),
                "Пользователь: " + name,
                "Сервер: " + api.getSocket().getRemoteAddress()->toString() + ":" + std::to_string(api.getSocket().getRemotePort())
            };
            fastfetch(info);
        }
        else if (command == "!help") {
            fullHelp();
        }
        else if (command.substr(0, 5) == "!help") {
            if (checkLength(command, 7)) help("!help");
            else help(command.substr(6));
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
        else {
            if (command[0] == '!')
            return PULSAR_EXIT_CODE_INVALID_COMMAND;
        }
        return PULSAR_EXIT_CODE_SUCCESS;
    }

    void fullHelp() {
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
    }

    void help(const std::string& command) {
        if (command == "!help") {
            fullHelp();
        } else if (command == "!exit") {
            std::cout << "!exit - Отключиться от сервера и выйти из клиента." << std::endl;
        } else if (command == "!dest") {
            std::cout << "!dest <channel/user> - Сменить назначение для отправляемых сообщений." << std::endl;
        } else if (command == "!join") {
            std::cout << "!join <channel> - Присоединиться к указанному каналу." << std::endl;
        } else if (command == "!leave") {
            std::cout << "!leave <channel> - Покинуть указанный канал." << std::endl;
        } else if (command == "!create") {
            std::cout << "!create <channel> - Создать новый канал с указанным именем." << std::endl;
        } else if (command == "!chat") {
            std::cout << "!chat <channel/user> - Просмотреть историю чата с указанным каналом или пользователем." << std::endl;
        } else if (command == "!profile") {
            std::cout << "!profile <username>|edit - Просмотреть профиль пользователя или изменить свой собственный." << std::endl;
        } else if (command == "!contact") {
            std::cout << "!contact <add/rem> <username> <contact> - Добавить или удалить контакт для указанного пользователя." << std::endl;
        } else if (command == "!unread") {
            std::cout << "!unread - Просмотреть все непрочитанные сообщения." << std::endl;
        } else if (command == "!read") {
            std::cout << "!read <chat> <id>|all - Пометить сообщение по ID в указанном чате как прочитанное, или все сообщения." << std::endl;
        } else if (command == "!fastfetch") {
            std::cout << "!fastfetch - Вывести информацию о клиенте." << std::endl;
        } else {
            std::cout << "Нет справки по команде: " << command << std::endl;
        }
    }
};