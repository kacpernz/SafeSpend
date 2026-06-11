#pragma once
#include <string>

class ITransaction {
public:
    virtual ~ITransaction() = default;
    virtual double getAmount() const = 0;
    virtual std::string getCategory() const = 0;
    virtual std::string getDate() const = 0;
    virtual bool isRecurring() const = 0;
    virtual std::string getAccountName() const = 0;
};