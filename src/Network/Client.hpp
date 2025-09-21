#pragma once

#include <SFML/Network.hpp>
#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <string>

class Client {
private:
    sf::TcpSocket socket;
    std::string serverIP;
    unsigned short port;
    std::atomic<bool> connected;
    std::thread receiveThread;
    
public:
    Client(const std::string& ip, unsigned short p) : serverIP(ip), port(p), connected(false) {}
    
    ~Client() {
        disconnect();
    }
    
    bool connectToServer() {
        sf::Socket::Status status = socket.connect(*sf::IpAddress::resolve(serverIP), port, sf::seconds(5));
        if (status != sf::Socket::Status::Done) {
            std::cout << "Error: Could not connect to server " << serverIP << ":" << port << std::endl;
            std::cout << "Make sure server is running and accessible" << std::endl;
            return false;
        }
        connected = true;
        std::cout << "Connected to server! Type your messages below." << std::endl;
        std::cout << "Type '/exit' to quit." << std::endl;
        std::cout << "------------------------" << std::endl;
        return true;
    }
    
    void disconnect() {
        connected = false;
        socket.disconnect();
    }
    
    void sendMessage(const std::string& message) {
        if (!connected) return;
        
        sf::Packet packet;
        packet << message;
        
        if (socket.send(packet) != sf::Socket::Status::Done) {
            std::cout << "Error sending message" << std::endl;
            connected = false;
        }
    }
    
    void receiveMessages() {
        while (connected) {
            sf::Packet packet;
            sf::Socket::Status status = socket.receive(packet);
            
            if (status == sf::Socket::Status::Done) {
                std::string message;
                if (packet >> message) {
                    std::cout << "\nReceived: " << message << std::endl;
                    std::cout << "> " << std::flush;
                }
            } else if (status == sf::Socket::Status::Disconnected) {
                std::cout << "\nDisconnected from server" << std::endl;
                connected = false;
                break;
            } else if (status == sf::Socket::Status::Error) {
                std::cout << "\nNetwork error occurred" << std::endl;
                connected = false;
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
        
        std::string message;
        while (connected) {
            std::cout << "> " << std::flush;
            std::getline(std::cin, message);
            
            if (!connected) break;
            
            if (message == "/exit") {
                std::cout << "Disconnecting..." << std::endl;
                break;
            }
            
            if (!message.empty()) {
                sendMessage(message);
            }
        }
        
        disconnect();
    }
};