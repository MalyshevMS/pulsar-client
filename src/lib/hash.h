#pragma once

#include <string>
#include <sstream>
#include <iomanip>

uint32_t fnv1a(const std::string& str) {
    uint32_t hash = 0x811C9DC5; // FNV offset basis
    for (char c : str) {
        hash ^= static_cast<uint8_t>(c);
        hash *= 0x01000193; // FNV prime
    }
    return hash;
}

std::string hasher(const std::string& input) {
    uint32_t h1 = fnv1a(PULSAR_SALT + input);
    uint32_t h2 = fnv1a(input + PULSAR_SALT);
    
    std::string combined = std::to_string(h1) + std::to_string(h2);
    uint32_t finalHash = fnv1a(combined);

    std::stringstream ss;
    ss << std::hex << finalHash;
    return ss.str();
}

std::string hash(const std::string& unhashed) {
    std::string current = unhashed;
    for (int i = 0; i < PULSAR_HASH_ITERATIONS; ++i) {
        current = hasher(current);
    }
    return current;
}
