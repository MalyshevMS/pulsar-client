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
    Database db;
    PulsarAPI api;
public:
    Client(const std::string& name, const std::string& password_unhashed, const std::string& ip, unsigned short p)
     : name(name), serverIP(ip), port(p), connected(false), db(name), api(socket, name) {
        if (password_unhashed.size() > 0) password = hash(password_unhashed);
        else password = "";
    }
    
    bool connectToServer() {
        sf::Socket::Status status = socket.connect(*sf::IpAddress::resolve(serverIP), port, sf::milliseconds(PULSAR_TIMEOUT_MS));
        if (status != sf::Socket::Status::Done) {
            std::cout << "Error: Could not connect to server " << serverIP << ":" << port << std::endl;
            std::cout << "Make sure server is running and accessible" << std::endl;
            return false;
        }
        connected = true;
        return true;
    }
    
    void disconnect() {
        connected = false;
        api.disconnect();
        exit(0);
    }
    
    void receiveMessages() {
        while (connected) {
            auto msg = api.receiveLastMessage();

            if (msg.get_src() == "!server") {
                api.storeServerResponse(msg.get_msg());
            } else if ((db.is_channel_member(msg.get_dst()) || msg.get_dst() == name) && login_success) {
                std::cout << '\n' << "[time: " << msg.get_time() << " | from " << msg.get_src() << " to " << msg.get_dst() << "]: " << msg.get_msg() << std::endl;
                std::cout << "(to " << dest << ") > " << std::flush;
            }
            
            sf::sleep(sf::milliseconds(100));
        }
    }
    
    void run() {
        if (!connectToServer()) {
            return;
        }
        
        receiveThread = std::thread(&Client::receiveMessages, this);
        auto login_status = api.login(password);
        if (login_status == PulsarAPI::LoginResult::Fail_Username) {
            std::cout << "Register new user? (y/N): ";
            std::string ans;
            std::getline(std::cin, ans);
            if (ans[0] == (char)0 || ans[0] == 'n' || ans[0] == 'N') disconnect();
            if (api.registerUser(password) != PulsarAPI::LoginResult::Success) {
                std::cout << "Registration failed; Disconnecting..." << std::endl;
                disconnect();
            }
        } else if (login_status != PulsarAPI::LoginResult::Success) {
            std::cout << "Disconnecting..." << std::endl;
            disconnect();
        }
        login_success = true;

        std::cout << "Connected to server! Type your messages below." << std::endl;
        std::cout << "Type '!exit' to quit. Type '!help' for commands." << std::endl;
        std::cout << "You are logged in as '" << name << "'." << std::endl;
        std::cout << "------------------------" << std::endl;

        db = api.requestDb();
        
        std::string message;
        dest = ":all";
        while (connected) {
            std::cout << "[" << name << "](to " << dest << ") > " << std::flush;
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
                if (api.joinChannel(message.substr(6, std::string::npos))) 
                    db.join(message.substr(6, std::string::npos));
                dest = message.substr(6, std::string::npos);
                continue;
            }
            else if (message.substr(0, 6) == "!leave") {
                if (api.leaveChannel(message.substr(7, std::string::npos))) 
                    db.leave(message.substr(7, std::string::npos)); 
                dest = ":all";
                continue;
            }
            else if (message.substr(0, 5) == "!chat") {
                auto arg = message.substr(6, std::string::npos);
                if (!db.is_channel_member(arg)) continue;
                std::cout << '\n' << api.getChat(arg).to_stream().rdbuf() << " ; EOT" << std::endl;
                continue;
            }
            else if (message.substr(0, 7) == "!create") {
                auto arg = message.substr(8, std::string::npos);
                if (api.createChannel(arg)) 
                    if (api.joinChannel(arg)) 
                        db.join(arg);
                dest = arg;
                continue;
            }
            else if (message == "!testrequest") {
                std::cout << "Sending test request..." << std::endl;
                std::cout << api.request("test", name) << std::endl;
                continue;
            }
            else if (message == "!help") {
                std::cout << "Available commands:\n"
                             "!exit                         - Disconnect from server and exit client\n"
                             "!dest <channel/user>          - Set destination channel for messages\n"
                             "!join <channel>               - Join a channel\n"
                             "!chat <channel/user>          - Request chat history for a channel or user\n"
                             "!testrequest                  - Send test request to server\n"
                             "!help                         - Show this help message\n";
                continue;
            }
            
            if (!message.empty()) {
                api.sendMessage(message, dest);
            }
        }
    }
};