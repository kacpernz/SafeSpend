#pragma once
#include <string>

class EncryptionService {
private:
    std::string key;

public:
    explicit EncryptionService(const std::string& key);
    void encryptDecrypt(char* data, size_t size);
};