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
    std::string dest;
    std::string name;
    std::string password;
    std::list<std::string> server_responses;
    Database db;
public:
    Client(const std::string& name, const std::string& password_unhashed, const std::string& ip, unsigned short p)
     : name(name), serverIP(ip), port(p), connected(false), db(name) {
        if (password_unhashed.size() > 0) password = hash(password_unhashed);
        else password = "";
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
        std::cout << "Press any key to continue...";
        std::cin.get();
        exit(0);
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

        db.init(json);
    }

    void parse_server(const std::string& resp) {
        if (resp == "login fail_password") {
            std::cerr << "Incorrect password; Disconnecting...";
            disconnect();
        } else if (resp == "login fail_username") {
            std::cerr << "Incorrect login; Register new user? (y/N): ";
            std::string ans;
            std::getline(std::cin, ans);
            if (ans[0] == (char)0 || ans[0] == 'n' || ans[0] == 'N') disconnect();
            sendMessage("!register " + jsonToString(Json::array({name, password})), "!server");
        } else if (resp == "login success") {
            login_success = true;
        }

        else if (resp.substr(0, 5) == "+join") {
            db.join(resp.substr(6, std::string::npos));
            std::cout << "Joined channel " << resp.substr(6, std::string::npos) << std::endl;
            std::cout << "(to " << dest << ") > " << std::flush;
        } else if (resp.substr(0, 5) == "-join") {
            std::cout << "Failed to join channel " << resp.substr(6, std::string::npos) << std::endl;
            std::cout << "(to " << dest << ") > " << std::flush;
        }

        else if (resp.substr(0, 4) == "chat") {
            std::vector<std::string> vec = Json::parse(resp.substr(5, std::string::npos));
            std::cout << '\n' << Chat(vec).to_stream().rdbuf() << " ; EOT" << std::endl;
            std::cout << "(to " << dest << ") > " << std::flush;
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
                    if (json["type"] == "error") {
                        std::cerr << "An error has been occured!\nError source: " << json["src"] << "\nError text: " << json["msg"] << std::endl;

                        std::cout << "(to " << dest << ") > " << std::flush;
                    }

                    if (json["src"] == "!server") {
                        parse_server(json["msg"]);

                        server_responses.push_back(json["msg"]);
                        if (server_responses.size() > PULSAR_MESSAGE_LIMIT) server_responses.pop_front();
                    } else if (db.is_channel_member(json["dst"]) || json["dst"] == name) {
                        #ifndef PULSAR_DEBUG
                            std::cout << "[From "  << std::string(json["src"]) << " to " << std::string(json["dst"]) << "]: " << std::string(json["msg"]) << std::endl;
                            std::cout << "(to " << dest << ") > " << std::flush;
                        #endif
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
        dest = ":all";
        while (connected) {
            std::cout << "(to " << dest << ") > " << std::flush;
            std::getline(std::cin, message);
            
            if (!connected) break;
            
            if (message == "!exit") {
                std::cout << "Disconnecting..." << std::endl;
                disconnect();
                break;
            }
            else if (message == "!cldb") {
                std::cout << db.getString() << std::endl;
                continue;
            }
            else if (message.substr(0, 5) == "!dest") {
                if (!db.is_channel_member(message.substr(6, std::string::npos))) continue;
                dest = message.substr(6, std::string::npos);
                continue;
            }
            else if (message.substr(0, 5) == "!join") {
                sendMessage("!join " + message.substr(6, std::string::npos), "!server");
                dest = message.substr(6, std::string::npos);
                continue;
            }
            else if (message.substr(0, 5) == "!msgs") {
                for (auto& i : server_responses) std::cout << i << " ; EOT\n";
                std::cout << std::flush;
                continue;
            }
            else if (message.substr(0, 2) == "!l") {
                std::cout << *(--server_responses.end()) << " ; EOT\n";
                continue;
            }
            else if (message.substr(0, 5) == "!chat") {
                auto arg = message.substr(6, std::string::npos);
                if (!db.is_channel_member(arg)) continue;
                sendMessage("!chat " + message.substr(6, std::string::npos), "!server");
                continue;
            }
            
            if (!message.empty()) {
                sendMessage(message, dest);
            }
        }
    }
};