#pragma once
#include "ITransaction.hpp"
#include <vector>
#include <memory>

class Wallet {
private:
    std::vector<std::unique_ptr<ITransaction>> transactions;

public:
    void addTransaction(std::unique_ptr<ITransaction> transaction);
    double calculateBalance() const;
    const std::vector<std::unique_ptr<ITransaction>>& getTransactions() const;
    void clear();
};