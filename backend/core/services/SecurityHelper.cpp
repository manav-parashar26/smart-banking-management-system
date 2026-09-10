#include "SecurityHelper.h"
#include <sstream>
#include <iomanip>
#include <random>
#include <cstring>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#include <wincrypt.h>
#endif

namespace SecurityHelper {

// ============================================================================
// Internal Cryptographic Primitives: SHA-256 & HMAC-SHA-256
// ============================================================================
namespace {

class SHA256 {
private:
    uint32_t state[8];
    uint64_t count;
    uint8_t buffer[64];

    static inline uint32_t rotr(uint32_t x, uint32_t n) { return (x >> n) | (x << (32 - n)); }
    static inline uint32_t ch(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (~x & z); }
    static inline uint32_t maj(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (x & z) ^ (y & z); }
    static inline uint32_t sigma0(uint32_t x) { return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22); }
    static inline uint32_t sigma1(uint32_t x) { return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25); }
    static inline uint32_t gamma0(uint32_t x) { return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3); }
    static inline uint32_t gamma1(uint32_t x) { return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10); }

    static const uint32_t K[64];

    void transform(const uint8_t block[64]) {
        uint32_t w[64];
        for (int i = 0; i < 16; ++i) {
            w[i] = (static_cast<uint32_t>(block[i * 4]) << 24) |
                   (static_cast<uint32_t>(block[i * 4 + 1]) << 16) |
                   (static_cast<uint32_t>(block[i * 4 + 2]) << 8) |
                   (static_cast<uint32_t>(block[i * 4 + 3]));
        }
        for (int i = 16; i < 64; ++i) {
            w[i] = gamma1(w[i - 2]) + w[i - 7] + gamma0(w[i - 15]) + w[i - 16];
        }

        uint32_t a = state[0], b = state[1], c = state[2], d = state[3];
        uint32_t e = state[4], f = state[5], g = state[6], h = state[7];

        for (int i = 0; i < 64; ++i) {
            uint32_t t1 = h + sigma1(e) + ch(e, f, g) + K[i] + w[i];
            uint32_t t2 = sigma0(a) + maj(a, b, c);
            h = g;
            g = f;
            f = e;
            e = d + t1;
            d = c;
            c = b;
            b = a;
            a = t1 + t2;
        }

        state[0] += a; state[1] += b; state[2] += c; state[3] += d;
        state[4] += e; state[5] += f; state[6] += g; state[7] += h;
    }

public:
    SHA256() { reset(); }

    void reset() {
        state[0] = 0x6a09e667; state[1] = 0xbb67ae85; state[2] = 0x3c6ef372; state[3] = 0xa54ff53a;
        state[4] = 0x510e527f; state[5] = 0x9b05688c; state[6] = 0x1f83d9ab; state[7] = 0x5be0cd19;
        count = 0;
    }

    void update(const uint8_t* data, size_t len) {
        size_t bufferIdx = static_cast<size_t>((count >> 3) & 63);
        count += static_cast<uint64_t>(len) << 3;

        size_t partLen = 64 - bufferIdx;
        size_t i = 0;

        if (len >= partLen) {
            std::memcpy(&buffer[bufferIdx], data, partLen);
            transform(buffer);
            for (i = partLen; i + 63 < len; i += 64) {
                transform(&data[i]);
            }
            bufferIdx = 0;
        }
        if (i < len) {
            std::memcpy(&buffer[bufferIdx], &data[i], len - i);
        }
    }

    void final(uint8_t digest[32]) {
        uint8_t finalCount[8];
        for (int i = 0; i < 8; ++i) {
            finalCount[i] = static_cast<uint8_t>((count >> ((7 - i) * 8)) & 0xff);
        }

        uint8_t pad = 0x80;
        update(&pad, 1);

        uint8_t zero = 0;
        while (((count >> 3) & 63) != 56) {
            update(&zero, 1);
        }
        update(finalCount, 8);

        for (int i = 0; i < 8; ++i) {
            digest[i * 4]     = static_cast<uint8_t>((state[i] >> 24) & 0xff);
            digest[i * 4 + 1] = static_cast<uint8_t>((state[i] >> 16) & 0xff);
            digest[i * 4 + 2] = static_cast<uint8_t>((state[i] >> 8) & 0xff);
            digest[i * 4 + 3] = static_cast<uint8_t>(state[i] & 0xff);
        }
    }

    static std::vector<uint8_t> hash(const uint8_t* data, size_t len) {
        SHA256 sha;
        sha.update(data, len);
        std::vector<uint8_t> out(32);
        sha.final(out.data());
        return out;
    }
};

const uint32_t SHA256::K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

// HMAC-SHA256 (RFC 2104)
std::vector<uint8_t> hmacSHA256(const uint8_t* key, size_t keyLen, const uint8_t* msg, size_t msgLen) {
    uint8_t k0[64] = {0};
    if (keyLen > 64) {
        auto h = SHA256::hash(key, keyLen);
        std::memcpy(k0, h.data(), 32);
    } else {
        std::memcpy(k0, key, keyLen);
    }

    uint8_t iPad[64], oPad[64];
    for (int i = 0; i < 64; ++i) {
        iPad[i] = k0[i] ^ 0x36;
        oPad[i] = k0[i] ^ 0x5c;
    }

    SHA256 innerSha;
    innerSha.update(iPad, 64);
    innerSha.update(msg, msgLen);
    uint8_t innerHash[32];
    innerSha.final(innerHash);

    SHA256 outerSha;
    outerSha.update(oPad, 64);
    outerSha.update(innerHash, 32);
    std::vector<uint8_t> out(32);
    outerSha.final(out.data());
    return out;
}

// PBKDF2-HMAC-SHA256 (RFC 2898)
std::vector<uint8_t> pbkdf2HMACSHA256(const std::string& password, const std::vector<uint8_t>& salt, int iterations, size_t keyLen = 32) {
    std::vector<uint8_t> derivedKey;
    derivedKey.reserve(keyLen);

    uint32_t blockIndex = 1;
    while (derivedKey.size() < keyLen) {
        std::vector<uint8_t> saltWithBlock = salt;
        saltWithBlock.push_back(static_cast<uint8_t>((blockIndex >> 24) & 0xff));
        saltWithBlock.push_back(static_cast<uint8_t>((blockIndex >> 16) & 0xff));
        saltWithBlock.push_back(static_cast<uint8_t>((blockIndex >> 8) & 0xff));
        saltWithBlock.push_back(static_cast<uint8_t>(blockIndex & 0xff));

        auto u = hmacSHA256(reinterpret_cast<const uint8_t*>(password.data()), password.length(), saltWithBlock.data(), saltWithBlock.size());
        std::vector<uint8_t> f = u;

        for (int i = 1; i < iterations; ++i) {
            u = hmacSHA256(reinterpret_cast<const uint8_t*>(password.data()), password.length(), u.data(), u.size());
            for (size_t j = 0; j < f.size(); ++j) {
                f[j] ^= u[j];
            }
        }

        for (size_t j = 0; j < f.size() && derivedKey.size() < keyLen; ++j) {
            derivedKey.push_back(f[j]);
        }
        blockIndex++;
    }

    return derivedKey;
}

} // anonymous namespace

// ============================================================================
// Public Security API Implementations
// ============================================================================

std::vector<uint8_t> generateSecureRandomBytes(size_t numBytes) {
    std::vector<uint8_t> bytes(numBytes);
    bool generated = false;

#ifdef _WIN32
    HCRYPTPROV hProv = 0;
    if (CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT)) {
        if (CryptGenRandom(hProv, static_cast<DWORD>(numBytes), bytes.data())) {
            generated = true;
        }
        CryptReleaseContext(hProv, 0);
    }
#endif

    if (!generated) {
        std::random_device rd;
        for (size_t i = 0; i < numBytes; ++i) {
            bytes[i] = static_cast<uint8_t>(rd() & 0xff);
        }
    }

    return bytes;
}

std::string toHex(const uint8_t* data, size_t len) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (size_t i = 0; i < len; ++i) {
        oss << std::setw(2) << static_cast<int>(data[i]);
    }
    return oss.str();
}

std::string toHex(const std::vector<uint8_t>& data) {
    return toHex(data.data(), data.size());
}

std::vector<uint8_t> fromHex(const std::string& hex) {
    std::vector<uint8_t> bytes;
    bytes.reserve(hex.length() / 2);
    for (size_t i = 0; i + 1 < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        try {
            uint8_t byte = static_cast<uint8_t>(std::stoul(byteString, nullptr, 16));
            bytes.push_back(byte);
        } catch (...) {
            return {};
        }
    }
    return bytes;
}

bool constantTimeEquals(const std::string& a, const std::string& b) {
    if (a.length() != b.length()) return false;
    volatile uint8_t diff = 0;
    for (size_t i = 0; i < a.length(); ++i) {
        diff |= static_cast<uint8_t>(a[i] ^ b[i]);
    }
    return diff == 0;
}

bool isHashed(const std::string& str) {
    return str.rfind("$pbkdf2-sha256$", 0) == 0;
}

std::string hashPin(const std::string& pin) {
    const int ITERATIONS = 10000;
    auto salt = generateSecureRandomBytes(16);
    auto derivedKey = pbkdf2HMACSHA256(pin, salt, ITERATIONS, 32);

    std::ostringstream oss;
    oss << "$pbkdf2-sha256$" << ITERATIONS << "$" << toHex(salt) << "$" << toHex(derivedKey);
    return oss.str();
}

bool verifyPin(const std::string& candidatePin, const std::string& storedPinOrHash) {
    // 1. If stored in cryptographic PBKDF2 format: $pbkdf2-sha256$iterations$salt$hash
    if (isHashed(storedPinOrHash)) {
        // Parse components
        size_t p1 = storedPinOrHash.find('$', 1);
        if (p1 == std::string::npos) return false;
        size_t p2 = storedPinOrHash.find('$', p1 + 1);
        if (p2 == std::string::npos) return false;
        size_t p3 = storedPinOrHash.find('$', p2 + 1);
        if (p3 == std::string::npos) return false;

        std::string iterStr = storedPinOrHash.substr(p1 + 1, p2 - p1 - 1);
        std::string saltHex = storedPinOrHash.substr(p2 + 1, p3 - p2 - 1);
        std::string expectedHashHex = storedPinOrHash.substr(p3 + 1);

        int iterations = 10000;
        try {
            iterations = std::stoi(iterStr);
        } catch (...) {
            return false;
        }

        auto saltBytes = fromHex(saltHex);
        if (saltBytes.empty() || expectedHashHex.empty()) return false;

        auto candidateDerivedKey = pbkdf2HMACSHA256(candidatePin, saltBytes, iterations, 32);
        std::string candidateHashHex = toHex(candidateDerivedKey);

        return constantTimeEquals(candidateHashHex, expectedHashHex);
    }

    // 2. Legacy Migration Support: If stored as plaintext PIN, compare in constant-time
    return constantTimeEquals(candidatePin, storedPinOrHash);
}

} // namespace SecurityHelper
