#include <string>
#include <bitset>
#include <iomanip>
#include <sstream>

#define PULSAR_SALT "57afbe95a4be3a9d"

std::string hash(const std::string& unhashed) {
    std::hash<std::string> hasher;
    size_t hash1 = hasher(PULSAR_SALT + unhashed);
    size_t hash2 = hasher(unhashed + PULSAR_SALT);
    
    std::string combined = std::to_string(hash1) + std::to_string(hash2);
    size_t finalHash = hasher(combined);
    
    std::stringstream ss;
    ss << std::hex << finalHash;
    return ss.str();
}