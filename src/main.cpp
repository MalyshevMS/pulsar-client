#define debug

#include "Network/Client.hpp"

#define PULSAR_PORT 4171

int main(int argc, const char** argv) {
    std::string serverIP;
    std::string name;
    std::string password;
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
    std::cout << "Enter password (press ENTER if none was provided): ";
    std::getline(std::cin, password);
    
    Client client(name, password, serverIP, PULSAR_PORT);
    client.run();
    
    std::cout << "Client terminated." << std::endl;
    return 0;
}