#pragma once
#include "ITransaction.hpp"
#include <vector>
#include <memory>
#include <string>

class DatabaseManager {
public:
    void saveToBinaryFile(const std::vector<std::unique_ptr<ITransaction>>& transactions, const std::string& filename, const std::string& password);
    std::vector<std::unique_ptr<ITransaction>> loadFromBinaryFile(const std::string& filename, const std::string& password);
};