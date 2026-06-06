#include "EncryptionService.hpp"

EncryptionService::EncryptionService(const std::string& key) : key(key) {}

void EncryptionService::encryptDecrypt(char* data, size_t size) {
    if (key.empty()) return;
    
    for (size_t i = 0; i < size; ++i) {
        data[i] ^= key[i % key.length()];
    }
}