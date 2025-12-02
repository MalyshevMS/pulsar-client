#pragma once

#include <SFML/Network.hpp>
#include <iostream>
#include <thread>
#include <list>
#include <atomic>
#include <string>
#include <mutex>
#include <condition_variable>
#include <chrono>

#include "../lib/jsonlib.h"
#include "../lib/hash.h"
#include "../Other/Datetime.hpp"
#include "../Other/Chat.hpp"
#include "../Network/Database.hpp"
#include "../Network/Checker.hpp"

class PulsarAPI {
private:
    sf::TcpSocket& socket;
    std::string name;
    Database db;
    std::list<std::string> server_responses;
    std::atomic_bool connected = true;
    std::mutex responses_mutex;
    std::condition_variable responses_cv;

    std::string waitForServerResponse(const std::string& expectedType, const std::string& additional = "", int32_t timeout_ms = PULSAR_TIMEOUT_MS) {
        auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms);

        std::unique_lock<std::mutex> lock(responses_mutex);
        while (std::chrono::steady_clock::now() < deadline) {
            for (auto it = server_responses.begin(); it != server_responses.end(); ++it) {
                const std::string response = *it;
                try {
                    auto json = Json::parse(response);
                    if (json.contains("type") && json["type"] == expectedType) {
                        std::string out = response;
                        server_responses.erase(it);
                        return out;
                    }
                } catch (...) {
                    bool starts = false;
                    if (response.size() >= expectedType.size() && response.substr(0, expectedType.size()) == expectedType) starts = true;
                    else if (response.size() >= expectedType.size() + 1 && (response[0] == '+' || response[0] == '-') && response.substr(1, expectedType.size()) == expectedType) starts = true;

                    if (starts && (additional.empty() || response.find(additional) != std::string::npos)) {
                        std::string out = response;
                        server_responses.erase(it);
                        return out;
                    }
                }
                #ifdef PULSAR_DEBUG
                    std::cerr << "[DEBUG]: Type not matched: " << response << std::endl;
                #endif
            }

            responses_cv.wait_for(lock, std::chrono::milliseconds(50));
        }

        std::cout << "Вышло время ожидания для запроса: " << expectedType << ", " << additional << std::endl;
        return "";
    }
public:
    enum LoginResult {
        Success,
        Fail_Username,
        Fail_Password,
        Fail_Unknown
    };

    PulsarAPI(sf::TcpSocket& sock, const std::string& username) : socket(sock), name(username), db(username) {
        if (!Checker::checkUsername(username)) PULSAR_THROW UsernameFailed(username);
    }
    
    std::string getName() const { return name; }
    sf::TcpSocket& getSocket() const { return socket; }

    void disconnect() {
        if (!connected) return;
        connected = false;
        socket.disconnect();
        std::cout << "Отключено от сервера." << std::endl;
    }

    void sendMessage(const std::string& message, const std::string& dest) {
        /*
            Message format (JSON):
            {
                "type": "message",
                "time": current time (integer, seconds since epoch),
                "src": "this client username",
                "dst": "destination (channel or user)"
                "msg": "your message text"
            }
        */

        auto json = Json({
            // sending without id; server will assign one
            {"type", "message"},
            {"time", Datetime::now().toTime()},
            {"src", name},
            {"dst", dest},
            {"msg", message}
        });

        std::string msg = jsonToString(json);
        
        if (socket.send(msg.data(), msg.size()) != sf::Socket::Status::Done) {
            std::cout << "Не удалось отправить сообщение" << std::endl;
            disconnect();
        }
    }

    bool joinChannel(const std::string& channel) {
        auto response = request("join", channel, channel);
        if (response.find("+join") != std::string::npos && response.find(channel) != std::string::npos) {
            std::cout << "Вы присоединились к каналу " << channel << std::endl;
            db.join(channel);
            return true;
        } else {
            std::cout << "Не удалось присоединиться к каналу " << channel << std::endl;
            return false;
        }
    }

    bool leaveChannel(const std::string& channel) {
        auto response = request("leave", channel, channel);
        if (response.find("+leave") != std::string::npos && response.find(channel) != std::string::npos) {
            std::cout << "Вы покинули канал " << channel << std::endl;
            db.leave(channel);
            return true;
        } else {
            std::cout << "Не удалось покинуть канал " << channel << std::endl;
            return false;
        }
    }

    bool createChannel(const std::string& channel) {
        if (!Checker::checkChannelName(channel)) PULSAR_THROW ChannelNameFailed(channel);

        auto response = request("create", channel, channel);
        if (response.find("+create") != std::string::npos && response.find(channel) != std::string::npos) {
            std::cout << "Создан канал " << channel << std::endl;
            joinChannel(channel);
            return true;
        } else {
            std::cout << "Не удалось создать канал " << channel << std::endl;
            return false;
        }
    }

    bool createContact(const std::string& username, const std::string& contact) {
        if (!Checker::checkUsername(username)) PULSAR_THROW UsernameFailed(username);

        auto response = request("db contact add", jsonToString(Json::array({username, contact})));
        if (response.find("+db contact add") != std::string::npos && response.find(username) != std::string::npos) {
            std::cout << "Создан контакт " << '"' << contact << '"' << std::endl;
            db.add_contact(username, contact);
            return true;
        } else {
            std::cout << "Не удалось создать контакт " << '"' << contact << '"' << std::endl;
            return false;
        }
    }

    bool removeContact(const std::string& contact) {
        if (!Checker::checkUsername(contact)) PULSAR_THROW UsernameFailed(contact);

        auto response = request("db contact rem", contact);
        if (response.find("+db contact rem") != std::string::npos && response.find(contact) != std::string::npos) {
            std::cout << "Удален контакт " << '"' << contact << '"' << std::endl;
            db.remove_contact(contact);
            return true;
        } else {
            std::cout << "Не удалось удалить контакт " << '"' << contact << '"' << std::endl;
            return false;
        }
    }

    void updateProfile(const Profile& profile) {
        db.update_profile(profile);

        auto response = request("profile", jsonToString(Json::array({"set", name, profile.toJson()})));
        if (response.find("profile set") != std::string::npos && response.find(name) != std::string::npos) {
            std::cout << "Профиль обновлен" << std::endl;
        }
    }

    Profile getProfile(const std::string& username) {
        auto response = request("profile", jsonToString(Json::array({"get", username})));
        if (response.find("profile get") != std::string::npos) {
            try {
                auto parsed = Json::parse(response.substr(12));
                return Profile::fromJson(parsed);
            } catch (const std::exception& e) {
                std::cerr << "Failed to parse profile JSON: " << e.what() << std::endl;
                return PULSAR_NO_PROFILE;
            } catch (...) {
                std::cerr << "Unknown error parsing profile JSON" << std::endl;
                return PULSAR_NO_PROFILE;
            }
        } else {
            return PULSAR_NO_PROFILE;
        }
    }

    LoginResult login(const std::string& password) {
        auto response = request("login", jsonToString(Json::array({name, password})));

        if (response.find("login success") != std::string::npos) {
            return LoginResult::Success;
        } else if (response.find("login fail_username") != std::string::npos) {
            std::cout << "Ошибка входа: неверное имя пользователя" << std::endl;
            return LoginResult::Fail_Username;
        } else if (response.find("login fail_password") != std::string::npos) {
            std::cout << "Ошибка входа: неверный пароль" << std::endl;
            return LoginResult::Fail_Password;
        } else {
            std::cout << "Ошибка входа: неизвестная ошибка" << std::endl;
            return LoginResult::Fail_Unknown;
        }
    }

    LoginResult registerUser(const std::string& password) {
        auto response = request("register", jsonToString(Json::array({name, password})));

        if (response.find("+register") != std::string::npos) {
            return LoginResult::Success;
        } else if (response.find("-register") != std::string::npos) {
            std::cout << "Ошибка регистрации: пользователь уже существует" << std::endl;
            return LoginResult::Fail_Username;
        } else {
            std::cout << "Ошибка регистрации: неизвестная ошибка" << std::endl;
            return LoginResult::Fail_Unknown;
        }
    }

    Chat getChat(const std::string& channel) {
        if (!Checker::checkChannelName(channel) && channel[0] != '@') PULSAR_THROW ChannelNameFailed(channel);
        auto response = request("chat", channel, channel);
        try {
            auto json = Json::parse(response.substr(5, std::string::npos));
            if (!json.contains("chat") || !json.contains("name")) {
                std::cerr << "Malformed chat response: " << response << std::endl;
                return Chat("", {});
            }
            std::vector<std::string> vec = json["chat"];
            return Chat(json["name"], vec);
        } catch (const std::exception& e) {
            std::cerr << "Failed to parse chat JSON: " << e.what() << " Response: " << response << std::endl;
            return Chat("", {});
        } catch (...) {
            std::cerr << "Unknown error parsing chat JSON. Response: " << response << std::endl;
            return Chat("", {});
        }
    }

    bool requestDb() {
        auto response = request("db user", name);
        try {
            auto json = Json::parse(response.substr(8, std::string::npos));
            db.init(json);
            return true;
        } catch (const std::exception& e) {
            std::cout << "Не удалось обработать запрос базы данных: " << e.what() << std::endl;
            return false;
        } catch (...) {
            std::cout << "Не удалось обработать запрос базы данных: неизвестная ошибка" << std::endl;
            return false;
        }
    }

    bool isChannelMember(const std::string& channel) {
        if (!Checker::checkChannelName(channel) && channel[0] != '@' && channel[0] != '!' && channel != "") PULSAR_THROW ChannelNameFailed(channel);
        return db.is_channel_member(channel);
    }

    std::string getDbString() {
        return db.getString();
    }

    std::string getLastResponse() {
        std::lock_guard<std::mutex> lock(responses_mutex);
        if (server_responses.empty()) return std::string();
        return server_responses.back();
    }

    Message receiveLastMessage() {
        if (!connected) return PULSAR_NO_MESSAGE;
        char buffer[PULSAR_PACKET_SIZE];
        std::size_t received;
        if (socket.receive(buffer, sizeof(buffer), received) != sf::Socket::Status::Done) {
            disconnect();
            return PULSAR_NO_MESSAGE;
        }

        std::string msg(buffer, received);
        try {
            Json json = Json::parse(msg);
            if (json.contains("type") && json["type"] == "error") {
                std::cerr << "\nAn error has been occured!\nError source: " << json["src"] << "\nError text: " << json["msg"] << std::endl;
                return Message(0, 0, "!server", name, "error: " + std::string(json["msg"]));
            }
            return db.contact(Message(json["id"], json["time"], json["src"], json["dst"], json["msg"]));
        } catch (const std::exception& e) {
            std::cerr << "Failed to parse incoming message JSON: " << e.what() << "\nRaw: " << msg << std::endl;
            return PULSAR_NO_MESSAGE;
        } catch (...) {
            std::cerr << "Unknown error parsing incoming message. Raw: " << msg << std::endl;
            return PULSAR_NO_MESSAGE;
        }
    }

    // bad working function
    Message getMsgById(const std::string& chat, size_t id) {
        auto response = request("msg", jsonToString(Json::array({chat, id})), std::to_string(id));
        if (response.find("msg") != std::string::npos) {
            return parse_line(response.substr(4), chat);
        }
        return PULSAR_NO_MESSAGE;
    }

    void storeUnread(const Message& msg) {
        db.store_unread(msg);
    }

    std::vector<Message> getUnread() {
        std::vector<Message> ret;
        auto msg = PULSAR_NO_MESSAGE;
        for (auto i : db.get_unread()) {
            ret.push_back(msg.from_json(i));
        }
        return ret;
    }

    void sendUnread() {
        auto unread = db.get_unread();
        if (unread.size() == 0) return;

        std::vector<Json> vec;
        for (auto i : unread) {
            Json payload;
            payload["id"] = i["id"];
            payload["dst"] = i["dst"];
            vec.push_back(payload);
        }

        request("unread", jsonToString(Json::array({vec})));
    }

    void requestUnread() {
        auto response = request("db user", name);
        try {
            auto parsed = Json::parse(response.substr(8, std::string::npos));
            if (!parsed.contains("unread")) return;
            std::vector<Json> vec = parsed["unread"];
            for (auto json : vec) {
                std::string chat_str = json["chat"];
                size_t id = json["id"];
                auto chat = getChat(chat_str);
                auto msg = chat.getByID(id);
                if (msg != PULSAR_NO_MESSAGE) db.store_unread(msg);
            }
        } catch (const std::exception& e) {
            std::cerr << "Failed to parse unread list JSON: " << e.what() << " Response: " << response << std::endl;
            return;
        } catch (...) {
            std::cerr << "Unknown error parsing unread list. Response: " << response << std::endl;
            return;
        }
    }

    void read(const std::string& chat, size_t id) {
        db.read(chat, id);
        request("read", jsonToString(Json::array({chat, id})));
    }

    void readAll(const std::string& chat) {
        auto unread = db.get_unread();
        auto parser = PULSAR_NO_MESSAGE;
        for (auto i : unread) {
            auto msg = parser.from_json(i);
            if (msg.get_dst() == chat) {
                read(chat, msg.get_id());
            }
        }
    }

    void storeServerResponse(const std::string& response) {
        std::lock_guard<std::mutex> lock(responses_mutex);
        server_responses.push_back(response);
        if (server_responses.size() > PULSAR_MESSAGE_LIMIT) server_responses.pop_front();
        responses_cv.notify_one();
    }

    std::string request(const std::string& request_type, const std::string& args, const std::string& additional = "") {
        sendMessage('!' + request_type + ' ' + args, "!server");
        return waitForServerResponse(request_type, additional);
    }
};