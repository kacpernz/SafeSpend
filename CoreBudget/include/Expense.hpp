#pragma once
#include "ITransaction.hpp"
#include <string>

class Expense : public ITransaction{
    friend class DatabaseManager;

private:
    double amount;
    std::string category;
    std::string date;
    bool recurring;
    std::string accountName;

public:
    Expense(double amount, const std::string &category, const std::string &date,
            bool recurring = false, const std::string &accountName = "Gotówka");

    double getAmount() const override;
    std::string getCategory() const override;
    std::string getDate() const override;
    bool isRecurring() const override;
    std::string getAccountName() const override;
};