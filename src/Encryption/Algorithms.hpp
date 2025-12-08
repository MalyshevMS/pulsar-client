#pragma once

#include "static"
#include "Random.hpp"

namespace PulsarCrypto {
    ubyte gcd(ubyte a, ubyte b) { // НОД
        while (b != 0) {
            ubyte t = b;
            b = a % b;
            a = t;
        }
        return a;
    }

    ubyte inv_mod(ubyte a, ubyte mod) { // Обратный модуль
        ubyte m0 = mod, x0 = 0, x1 = 1;
        ubyte t, q;
        if (mod == 1) return 0;
        while (a > 1) {
            q = a / mod;
            t = mod;
            mod = a % mod;
            a = t;
            t = x0;
            x0 = x1 - q * x0;
            x1 = t;
        }
        if (x1 < 0) x1 += m0;
        return x1;
    }

    ubyte pow_mod(ubyte base, ubyte exp, ubyte mod) { // (base ^ exp) % mod
        ubyte result = 1;
        base = base % mod;
        while (exp > 0) {
            if (exp % 2 == 1)
                result = (result * base) % mod;
            exp = exp >> 1;
            base = (base * base) % mod;
        }
        return result;
    }

    bool is_prime(ubyte n) { // Проверка на простоту
        if (n <= 1) return false;
        for (ubyte i = 2; i * i <= n; ++i)
            if (n % i == 0)
                return false;
        return true;
    }

    ubyte generate_prime(ubyte min, ubyte max) { // Генератор простых чисел
        while (true) {
            ubyte num = random_ubyte();
            if (is_prime(num)) return num;
        }
        return 0;
    }
};