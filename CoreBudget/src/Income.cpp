#include "Income.hpp"

Income::Income(double amount, const std::string& category, const std::string& date, bool recurring, const std::string& accountName)
    : amount(amount), category(category), date(date), recurring(recurring), accountName(accountName) {}

double Income::getAmount() const {
    return amount;
}

std::string Income::getCategory() const {
    return category;
}

std::string Income::getDate() const {
    return date;
}

bool Income::isRecurring() const {
    return recurring;
}

std::string Income::getAccountName() const {
    return accountName;
}