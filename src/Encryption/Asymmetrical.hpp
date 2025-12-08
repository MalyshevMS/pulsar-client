#pragma once

#include "static"
#include "Algorithms.hpp"

namespace PulsarCrypto {
    namespace Asymmetrical {
        struct RSAkey {
            ubyte n, s;
            RSAkey(ubyte n, ubyte s) : n(n), s(s) {}
        };

        ubyte enc(ubyte b, RSAkey& key) {
            return pow_mod(b, key.s, key.n);
        }

        ubyte dec(ubyte b, RSAkey& key) {
            return pow_mod(b, key.s, key.n);
        }

        bytes encrypt(const bytes& msg, RSAkey pub) {
            bytes res;
            for (ubyte c : msg) res.push_back(enc(c, pub));
            return res;
        }

        bytes decrypt(const bytes& msg, RSAkey priv) {
            bytes res;
            for (ubyte c : msg) res.push_back(dec(c, priv));
            return res;
        }

        class RSAGenerator {
        private:
            ubyte p, q; // простые числа
            ubyte n; // модуль
            ubyte phi; // функция Эйлера
            ubyte e; // открытый ключ
            ubyte d; // закрытый ключ
        public:
            RSAGenerator() {

            }

            RSAGenerator(ubyte prime1, ubyte prime2, ubyte public_exp = 3) : p(prime1), q(prime2), e(public_exp) {
                ubyte n = p * q;
                ubyte phi = (p - 1) * (q - 1);

                while (gcd(e, phi) != 1) e += 2;

                d = inv_mod(e, phi);
            }

            RSAkey getPublic() {
                return {n, e};
            }

            RSAkey getPrivate() {
                return {n, d};
            }
        };
    };
};