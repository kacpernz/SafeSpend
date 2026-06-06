#include "Income.hpp"

Income::Income(double amount, const std::string& category, const std::string& date,
               bool recurring)
    : amount(amount), category(category), date(date), recurring(recurring) {}

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