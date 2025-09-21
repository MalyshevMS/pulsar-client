#include "Network/Client.hpp"

#define PULSAR_PORT 4171

int main() {
    std::string serverIP;
    std::cout << "Enter server IP (127.0.0.1 for localhost): ";
    std::getline(std::cin, serverIP);
    
    if (serverIP.empty()) {
        serverIP = "127.0.0.1";
    }
    
    Client client(serverIP, PULSAR_PORT);
    client.run();
    
    std::cout << "Client terminated." << std::endl;
    return 0;
}