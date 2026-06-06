#pragma once
#include "ITransaction.hpp"
#include <vector>
#include <memory>
#include <string>

// ── Cel oszczędnościowy ───────────────────────────────────────────────────────
struct Goal {
    std::string name;
    double      targetAmount  = 0.0;
    double      currentAmount = 0.0;
};

class Wallet {
private:
    std::vector<std::unique_ptr<ITransaction>> transactions;
    std::vector<Goal>                          goals;

public:
    // ── Transakcje ───────────────────────────────────────────────────────────
    void addTransaction(std::unique_ptr<ITransaction> transaction);
    double calculateBalance() const;
    const std::vector<std::unique_ptr<ITransaction>>& getTransactions() const;
    void clear();

    // ── Cele oszczędnościowe ─────────────────────────────────────────────────
    void   addGoal(const Goal& goal);
    bool   fundGoal(size_t index, double amount); // zwraca false jeśli brak salda
    const std::vector<Goal>& getGoals() const;
    std::vector<Goal>&       getGoalsMutable();
};