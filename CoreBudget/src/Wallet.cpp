#include "Wallet.hpp"
#include "Income.hpp"
#include "Expense.hpp"

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
}

// ── Cele oszczędnościowe ──────────────────────────────────────────────────────

void Wallet::addGoal(const Goal& goal) {
    goals.push_back(goal);
}

bool Wallet::fundGoal(size_t index, double amount) {
    if (index >= goals.size()) return false;
    if (calculateBalance() < amount) return false;

    // Odejmij z salda poprzez dodanie wydatku "Cel: <nazwa>"
    // (logika odejmowania jest w addTransaction — tu zwracamy tylko sukces,
    //  MainWindow sam doda transakcję Expense i wywoła addGoal z nową kwotą)
    goals[index].currentAmount += amount;
    return true;
}

const std::vector<Goal>& Wallet::getGoals() const {
    return goals;
}

std::vector<Goal>& Wallet::getGoalsMutable() {
    return goals;
}