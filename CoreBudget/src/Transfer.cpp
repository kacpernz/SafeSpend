#include "Transfer.hpp"

Transfer::Transfer(double amount,
                   const std::string& fromAccount,
                   const std::string& toAccount,
                   const std::string& date)
    : amount(amount)
    , fromAccount(fromAccount)
    , toAccount(toAccount)
    , date(date)
{}

double Transfer::getAmount() const {
    return amount;
}

std::string Transfer::getCategory() const {
    return "Transfer";
}

std::string Transfer::getDate() const {
    return date;
}

bool Transfer::isRecurring() const {
    return false;
}

std::string Transfer::getAccountName() const {
    return fromAccount;
}

std::string Transfer::getFromAccount() const {
    return fromAccount;
}

std::string Transfer::getToAccount() const {
    return toAccount;
}
