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
    std::list<std::string> server_responses;
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

    void requestDb() {
        api.sendMessage("!db user " + name, "!server");
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

            if (wait_time > PULSAR_TIMEOUT_MS) {
                std::cerr << "Database request took too long; Disconecting..." << std::endl;
                disconnect();
                break;
            }
        }

        db.init(json);
    }

    void parse_server(const std::string& resp) {
        // if (resp.substr(0, 5) == "+join") {
        //     db.join(resp.substr(6, std::string::npos));
        //     std::cout << "Joined channel " << resp.substr(6, std::string::npos) << std::endl;
        //     std::cout << "(to " << dest << ") > " << std::flush;
        // } else if (resp.substr(0, 5) == "-join") {
        //     std::cout << "Failed to join channel " << resp.substr(6, std::string::npos) << std::endl;
        //     std::cout << "(to " << dest << ") > " << std::flush;
        // }

        // if (resp.substr(0, 4) == "chat") {
        //     auto json = Json::parse(resp.substr(5, std::string::npos));
        //     std::vector<std::string> vec = json["chat"];
        //     std::cout << '\n' << Chat(json["name"], vec).to_stream().rdbuf() << " ; EOT" << std::endl;
        //     std::cout << "(to " << dest << ") > " << std::flush;
        // }
    }
    
    void receiveMessages() {
        while (connected) {
            auto msg = api.receiveLastMessage();

            if (msg.get_src() == "!server") {
                parse_server(msg.get_msg());

                api.storeServerResponse(msg.get_msg());
                server_responses.push_back(msg.get_msg());
                if (server_responses.size() > PULSAR_MESSAGE_LIMIT) server_responses.pop_front();
            } else if ((db.is_channel_member(msg.get_dst()) || msg.get_dst() == name) && login_success) {
                std::cout << '\n' << "[time: " << msg.get_time() << " | from: " << msg.get_src() << " to: " << msg.get_dst() << "]: " << msg.get_msg() << std::endl;
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
                std::cout << '\n' << api.getChat(arg).to_stream().rdbuf() << " ; EOT" << std::endl;
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