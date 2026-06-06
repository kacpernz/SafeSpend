#pragma once
#include "ITransaction.hpp"

class Income : public ITransaction {
    friend class DatabaseManager;

private:
    double      amount;
    std::string category;
    std::string date;
    bool        recurring;

public:
    Income(double amount, const std::string& category, const std::string& date,
           bool recurring = false);

    double      getAmount()   const override;
    std::string getCategory() const override;
    std::string getDate()     const override;
    bool        isRecurring() const override;
};