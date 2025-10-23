#include "defines"
#include "Network/Client.hpp"

#ifdef _WIN32
#include <windows.h>
#endif

int main(int argc, const char** argv) {
#ifdef _WIN32
    SetConsoleCP(65001); // Russian UTF-8 support
    SetConsoleOutputCP(65001);
#endif
    std::string serverIP;
    std::string name;
    std::string password;
    #ifndef PULSAR_IP_PRESET
        std::cout << "Enter server IP (default " << PULSAR_IP_PRESET << "): ";
        std::getline(std::cin, serverIP);

        if (serverIP.empty()) {
            serverIP = PULSAR_IP_PRESET;
        }
    #else
        serverIP = PULSAR_IP_PRESET;
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