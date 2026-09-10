#pragma once

#include <string>
#include <vector>
#include <cstdint>

// ============================================================================
// CRYPTOGRAPHIC SECURITY & PASSWORD/PIN HASHING HELPER
// Standard: PBKDF2-HMAC-SHA256 (NIST SP 800-132 / RFC 2898)
// Features: 128-bit CSPRNG Salt, 10,000 Iterations, Constant-Time Verification
// ============================================================================

namespace SecurityHelper {

    // Generate cryptographically secure random bytes (CSPRNG)
    std::vector<uint8_t> generateSecureRandomBytes(size_t numBytes = 16);

    // Hex encoding and decoding utilities
    std::string toHex(const std::vector<uint8_t>& data);
    std::string toHex(const uint8_t* data, size_t len);
    std::vector<uint8_t> fromHex(const std::string& hex);

    // Cryptographic hash a PIN with a unique salt:
    // Format: "$pbkdf2-sha256$10000$<32-hex-salt>$<64-hex-hash>"
    std::string hashPin(const std::string& pin);

    // Verify candidate PIN against a stored hash (or legacy plaintext during migration)
    bool verifyPin(const std::string& candidatePin, const std::string& storedPinOrHash);

    // Check if a stored string is already in PBKDF2 hash format
    bool isHashed(const std::string& str);

    // Constant-time string equality comparison to resist side-channel timing attacks
    bool constantTimeEquals(const std::string& a, const std::string& b);

} // namespace SecurityHelper
