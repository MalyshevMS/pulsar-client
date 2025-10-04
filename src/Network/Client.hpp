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
#include "Database.hpp"

#define PULSAR_MESSAGE_LIMIT 10
#define PULSAR_MAX_RESPONSE_TIME 5000
// #define PULSAR_DEBUG

class Client {
private:
    sf::TcpSocket socket;
    std::string serverIP;
    unsigned short port;
    std::atomic<bool> connected;
    std::atomic<bool> login_success = false;
    std::thread receiveThread;
    std::string name;
    std::string password;
    std::list<std::string> server_responses;
    Database db;
public:
    Client(const std::string& name, const std::string& password_unhashed, const std::string& ip, unsigned short p)
     : name(name), serverIP(ip), port(p), connected(false), db(name) {
        password = hash(password_unhashed);
    }
    
    bool connectToServer() {
        sf::Socket::Status status = socket.connect(*sf::IpAddress::resolve(serverIP), port, sf::seconds(5));
        if (status != sf::Socket::Status::Done) {
            std::cout << "Error: Could not connect to server " << serverIP << ":" << port << std::endl;
            std::cout << "Make sure server is running and accessible" << std::endl;
            return false;
        }
        connected = true;
        sendMessage("!login " + jsonToString(Json::array({name, password})), "!server");
        return true;
    }
    
    void disconnect() {
        connected = false;
        socket.disconnect();
    }

    void requestDb() {
        sendMessage("!db user " + name, "!server");
        std::string response;
        Json json;
        int32_t wait_time = 0;
        while (true) {
            response = *(--server_responses.end());
            try {
                json = Json::parse(response);
                if (json.contains("channels")) break;
            } catch(const std::exception& e){}
            sf::sleep(sf::milliseconds(1));
            wait_time++;

            if (wait_time > PULSAR_MAX_RESPONSE_TIME) {
                std::cerr << "Database request took too long; Disconecting..." << std::endl;
                disconnect();
                break;
            }
        }
        
        
    }
    
    void sendMessage(const std::string& message, const std::string& dest) {
        if (!connected) return;

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
    
    void receiveMessages() {
        while (connected) {
            sf::Packet packet;
            sf::Socket::Status status = socket.receive(packet);
            
            if (status == sf::Socket::Status::Done) {
                std::string message;
                if (packet >> message) {
                    Json json = Json::parse(message);

                    #ifdef PULSAR_DEBUG
                        std::cout << "\nReceived: " << message << std::endl;
                        std::cout << "Decoded: " << std::string(json["msg"]) << std::endl;
                    #endif                    

                    std::cout << "> " << std::flush;

                    if (json["src"] == "!server") {
                        if (json["msg"] == "login fail_username" || json["msg"] == "login fail_password") {
                            disconnect();
                        } else if (json["msg"] == "login success") {
                            login_success = true;
                        }

                        server_responses.push_back(json["msg"]);
                        if (server_responses.size() > PULSAR_MESSAGE_LIMIT) server_responses.pop_front();
                    }
                }
            } else if (status == sf::Socket::Status::Disconnected) {
                std::cout << "\nDisconnected from server" << std::endl;
                disconnect();
                break;
            } else if (status == sf::Socket::Status::Error) {
                std::cout << "\nNetwork error occurred" << std::endl;
                disconnect();
                break;
            }
            
            sf::sleep(sf::milliseconds(100));
        }
    }
    
    void run() {
        if (!connectToServer()) {
            return;
        }
        
        receiveThread = std::thread(&Client::receiveMessages, this);
        while (!login_success) sf::sleep(sf::milliseconds(1));

        std::cout << "Connected to server! Type your messages below." << std::endl;
        std::cout << "Type '!exit' to quit." << std::endl;
        std::cout << "------------------------" << std::endl;

        requestDb();
        
        std::string message;
        std::string dest = ":all";
        while (connected) {
            std::cout << "> " << std::flush;
            std::getline(std::cin, message);
            
            if (!connected) break;
            
            if (message == "!exit") {
                std::cout << "Disconnecting..." << std::endl;
                break;
            }
            else if (message == "!cldb") {
                std::cout << db.getString() << std::endl;
                continue;
            }
            else if (message.substr(0, 5) == "!dest") {
                dest = message.substr(6, std::string::npos);
                continue;
            }
            else if (message.substr(0, 5) == "!msgs") {
                for (auto& i : server_responses) std::cout << i << "; ";
                std::cout << std::endl;
                continue;
            }
            
            if (!message.empty()) {
                sendMessage(message, dest);
            }
        }
    }
};