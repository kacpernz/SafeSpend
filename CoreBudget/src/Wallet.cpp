#include "Wallet.hpp"
#include "Income.hpp"
#include "Expense.hpp"

void Wallet::addTransaction(std::unique_ptr<ITransaction> transaction) {
    transactions.push_back(std::move(transaction));
}

double Wallet::calculateBalance() const {
    double balance = 0.0;
    
    for (const auto& t : transactions) {
        if (dynamic_cast<Income*>(t.get())) {
            balance += t->getAmount();
        } else if (dynamic_cast<Expense*>(t.get())) {
            balance -= t->getAmount();
        }
    }
    
    return balance;
}

const std::vector<std::unique_ptr<ITransaction>>& Wallet::getTransactions() const {
    return transactions;
}

void Wallet::clear() {
    transactions.clear();
}