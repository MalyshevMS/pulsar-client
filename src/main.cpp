#define debug

#include "Network/Client.hpp"

#define PULSAR_PORT 4171

int main(int argc, const char** argv) {
    std::string serverIP;
    std::string name;
    #ifndef debug
        std::cout << "Enter server IP (127.0.0.1 for localhost): ";
        std::getline(std::cin, serverIP);

        if (serverIP.empty()) {
            serverIP = "127.0.0.1";
        }
    #else
        serverIP = "127.0.0.1";
    #endif

    if (argc <= 1) {
        std::cout << "Enter username: ";
        std::getline(std::cin, name);
    } else {
        name = "@" + std::string(argv[1]);
    }
    
    Client client(name, serverIP, PULSAR_PORT);
    client.run();
    
    std::cout << "Client terminated." << std::endl;
    return 0;
}