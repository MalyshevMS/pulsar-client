#include <string>
#include <bitset>
#include <iomanip>
#include <sstream>

std::string hasher(const std::string& input) {
    std::hash<std::string> hasher;
    size_t hash1 = hasher(PULSAR_SALT + input);
    size_t hash2 = hasher(input + PULSAR_SALT);
    
    std::string combined = std::to_string(hash1) + std::to_string(hash2);
    size_t finalHash = hasher(combined);
    
    std::stringstream ss;
    ss << std::hex << finalHash;
    return ss.str();
}

std::string hash(const std::string& unhashed) {
    std::string unhashed_copy = unhashed;
    for (int i = 0; i < PULSAR_HASH_ITERATIONS; ++i) {
        unhashed_copy = hasher(unhashed_copy);
    }
    return unhashed_copy;
}