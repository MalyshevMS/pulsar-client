#pragma once

#include "static"

namespace PulsarCrypto {
    static inline bytes random_bytes(size_t n) {
        static std::random_device rd;
        static std::mt19937_64 gen(rd());
        static std::uniform_int_distribution<ubyte> dist;

        bytes out(n);
        size_t i = 0;

        while (i + 8 <= n) {
            ubyte v = dist(gen);

            for (int k = 0; k < 8; k++)
                out[i++] = ubyte((v >> (k * 8)) & 0xFF);
        }
        
        while (i < n)
            out[i++] = ubyte(dist(gen) & 0xFF);

        return out;
    }

    static inline ubyte random_ubyte() {
        static std::random_device rd;
        static std::mt19937_64 gen(rd());
        static std::uniform_int_distribution<ubyte> dist;

        return ubyte(dist(gen) & 0xFF);
    }
};