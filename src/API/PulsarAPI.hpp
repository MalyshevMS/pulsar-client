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
#include "../Network/Database.hpp"

class PulsarAPI {
private:
    sf::TcpSocket& socket;
    std::string name;
    std::list<std::string> server_responses;

    std::string waitForServerResponse(const std::string& expectedType, const std::string& additional = "", int32_t timeout_ms = PULSAR_TIMEOUT_MS) {
        int32_t wait_time = 0;
        while (wait_time < timeout_ms) {
            if (!server_responses.empty()) {
                std::string response = *(--server_responses.end());
                try {
                    auto json = Json::parse(response);
                    if (json.contains("type") && json["type"] == expectedType) {
                        return response;
                    }
                } catch(const std::exception& e) {
                    if (response.find(expectedType) != std::string::npos && response.find(additional) != std::string::npos) {
                        return response;
                    }
                }
                #ifdef PULSAR_DEBUG
                    std::cerr << "Type not matched: " << response << std::endl;
                #endif
            }
            sf::sleep(sf::milliseconds(1));
            wait_time++;
        }
        std::cout << "Timeout waiting for server response" << std::endl;
        return "";
    }
public:
    enum LoginResult {
        Success,
        Fail_Username,
        Fail_Password,
        Fail_Unknown
    };

    PulsarAPI(sf::TcpSocket& sock, const std::string& username) : socket(sock), name(username) {}
    
    std::string getName() const { return name; }
    sf::TcpSocket& getSocket() const { return socket; }

    void disconnect() {
        socket.disconnect();
        std::cout << "Press any key to continue...";
        std::cin.get();
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
            {"type", "message"},
            {"time", Datetime::now().toTime()},
            {"src", name},
            {"dst", dest},
            {"msg", message}
        });

        std::string msg = jsonToString(json);
        sf::Packet packet;
        packet << msg;
        
        if (socket.send(packet) != sf::Socket::Status::Done) {
            std::cout << "Error sending message" << std::endl;
            disconnect();
        }
    }

    bool joinChannel(const std::string& channel) {
        auto response = request("join", channel, channel);
        if (response.find("+join") != std::string::npos && response.find(channel) != std::string::npos) {
            std::cout << "Joined channel " << channel << std::endl;
            return true;
        } else {
            std::cout << "Failed to join channel " << channel << std::endl;
            return false;
        }
    }

    bool leaveChannel(const std::string& channel) {
        auto response = request("leave", channel, channel);
        if (response.find("+leave") != std::string::npos && response.find(channel) != std::string::npos) {
            std::cout << "Left channel " << channel << std::endl;
            return true;
        } else {
            std::cout << "Failed to leave channel " << channel << std::endl;
            return false;
        }
    }

    LoginResult login(const std::string& password) {
        auto response = request("login", jsonToString(Json::array({name, password})));

        if (response.find("login success") != std::string::npos) {
            return LoginResult::Success;
        } else if (response.find("login fail_username") != std::string::npos) {
            std::cout << "Login failed: Incorrect username" << std::endl;
            return LoginResult::Fail_Username;
        } else if (response.find("login fail_password") != std::string::npos) {
            std::cout << "Login failed: Incorrect password" << std::endl;
            return LoginResult::Fail_Password;
        } else {
            std::cout << "Login failed: Unknown error" << std::endl;
            return LoginResult::Fail_Unknown;
        }
    }

    LoginResult registerUser(const std::string& password) {
        auto response = request("register", jsonToString(Json::array({name, password})));

        if (response.find("register success") != std::string::npos) {
            return LoginResult::Success;
        } else {
            std::cout << "Login failed: Unknown error" << std::endl;
            return LoginResult::Fail_Unknown;
        }
    }

    Chat getChat(const std::string& channel) {
        auto response = request("chat", channel, channel);
        auto json = Json::parse(response.substr(5, std::string::npos));
        std::vector<std::string> vec = json["chat"];
        return Chat(json["name"], vec);
    }

    Database requestDb() {
        Database db(name);
        auto response = request("db user", name);
        auto json = Json::parse(response.substr(8, std::string::npos));
        db.init(json);
        return db;
    }

    Message receiveLastMessage() {
        sf::Packet packet;
        if (socket.receive(packet) != sf::Socket::Status::Done) {
            throw std::runtime_error("Error receiving message");
        }

        std::string msg;
        if (!(packet >> msg)) {
            throw std::runtime_error("Error extracting message from packet");
        }

        auto json = Json::parse(msg);
        if (json["type"] == "error") {
            std::cerr << "\nAn error has been occured!\nError source: " << json["src"] << "\nError text: " << json["msg"] << std::endl;
            return Message(0, "!server", name, "error: " + std::string(json["msg"]));
        }

        return Message(json["time"], json["src"], json["dst"], json["msg"]);
    }

    void storeServerResponse(const std::string& response) {
        server_responses.push_back(response);
        if (server_responses.size() > PULSAR_MESSAGE_LIMIT) server_responses.pop_front();
    }

    std::string request(const std::string& request_type, const std::string& args, const std::string& additional = "") {
        sendMessage('!' + request_type + ' ' + args, "!server");
        return waitForServerResponse(request_type, additional);
    }
};