#include "Expense.hpp"

Expense::Expense(double amount, const std::string& category, const std::string& date,
                 bool recurring, const std::string& accountName)
    : amount(amount), category(category), date(date), recurring(recurring),
      accountName(accountName) {}

double Expense::getAmount() const {
    return amount;
}

std::string Expense::getCategory() const {
    return category;
}

std::string Expense::getDate() const {
    return date;
}

bool Expense::isRecurring() const {
    return recurring;
}

std::string Expense::getAccountName() const {
    return accountName;
}