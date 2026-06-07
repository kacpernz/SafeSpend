#include "Wallet.hpp"
#include "Income.hpp"
#include "Expense.hpp"
#include "Transfer.hpp"


// ── Transakcje ────────────────────────────────────────────────────────────────

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
    goals.clear();
    categoryLimits.clear();
}

// ── Subkonta ──────────────────────────────────────────────────────────────────

std::map<std::string, double> Wallet::getAccountBalances() const {
    std::map<std::string, double> balances;
    for (const auto& t : transactions) {
        if (dynamic_cast<Income*>(t.get())) {
            balances[t->getAccountName()] += t->getAmount();
        } else if (dynamic_cast<Expense*>(t.get())) {
            balances[t->getAccountName()] -= t->getAmount();
        } else if (auto* tr = dynamic_cast<Transfer*>(t.get())) {
            // Transfer: odejmuje z konta źródłowego, dodaje do docelowego
            balances[tr->getFromAccount()] -= tr->getAmount();
            balances[tr->getToAccount()]   += tr->getAmount();
        }
    }
    return balances;
}

// ── Cele oszczędnościowe ──────────────────────────────────────────────────────

void Wallet::addGoal(const Goal& goal) {
    goals.push_back(goal);
}

bool Wallet::fundGoal(size_t index, double amount) {
    if (index >= goals.size()) return false;
    if (calculateBalance() < amount) return false;
    goals[index].currentAmount += amount;
    return true;
}

const std::vector<Goal>& Wallet::getGoals() const {
    return goals;
}

std::vector<Goal>& Wallet::getGoalsMutable() {
    return goals;
}

// ── Limity kopertowe ──────────────────────────────────────────────────────────

void Wallet::setCategoryLimit(const std::string& category, double limit) {
    if (limit <= 0.0)
        categoryLimits.erase(category);
    else
        categoryLimits[category] = limit;
}

void Wallet::removeCategoryLimit(const std::string& category) {
    categoryLimits.erase(category);
}

const std::map<std::string, double>& Wallet::getCategoryLimits() const {
    return categoryLimits;
}

// ── Globalny limit budżetu miesięcznego ──────────────────────────────────────

void Wallet::setMonthlyBudgetLimit(double limit) {
    if (limit > 0.0)
        monthlyBudgetLimit = limit;
}

double Wallet::getMonthlyBudgetLimit() const {
    return monthlyBudgetLimit;
}