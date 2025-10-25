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
public:
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

    void joinChannel(const std::string& channel) {
        sendMessage("!join " + channel, "!server");
    }

    void leaveChannel(const std::string& channel) {
        sendMessage("!leave " + channel, "!server");
    }

    void login(const std::string& password) {
        sendMessage("!login " + jsonToString(Json::array({name, password})), "!server");
    }

    void registerUser(const std::string& password) {
        sendMessage("!register " + jsonToString(Json::array({name, password})), "!server");
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
            std::cerr << "An error has been occured!\nError source: " << json["src"] << "\nError text: " << json["msg"] << std::endl;
            return Message(0, "!server", name, "error: " + std::string(json["msg"]));
        }

        return Message(json["time"], json["src"], json["dst"], json["msg"]);
    }

    void storeServerResponse(const std::string& response) {
        server_responses.push_back(response);
        if (server_responses.size() > PULSAR_MESSAGE_LIMIT) server_responses.pop_front();
    }

    std::string waitForServerResponse(const std::string& expectedType, int32_t timeout_ms = PULSAR_TIMEOUT_MS) {
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
                    if (response.find(expectedType) != std::string::npos) {
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
};