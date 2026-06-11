#pragma once
#include "ITransaction.hpp"
#include <vector>
#include <map>
#include <memory>
#include <string>

// === Cel oszczędnościowy ===
struct Goal {
    std::string name;
    double targetAmount = 0.0;
    double currentAmount = 0.0;
};

class Wallet {
private:
    std::vector<std::unique_ptr<ITransaction>> transactions;
    std::vector<Goal> goals;
    std::map<std::string, double> categoryLimits;
    double monthlyBudgetLimit = 5000.0;

public:
    // === Transakcje ===
    void addTransaction(std::unique_ptr<ITransaction> transaction);
    double calculateBalance() const;
    const std::vector<std::unique_ptr<ITransaction>> &getTransactions() const;
    void clear();

    // === Subkonta ===
    // Zwraca mapę: accountName -> saldo (Income dodaje, Expense odejmuje)
    std::map<std::string, double> getAccountBalances() const;

    // === Cele oszczędnościowe ===
    void addGoal(const Goal &goal);
    bool fundGoal(size_t index, double amount);
    const std::vector<Goal> &getGoals() const;
    std::vector<Goal> &getGoalsMutable();

    // === Limity kopertowe ===
    void setCategoryLimit(const std::string &category, double limit);
    void removeCategoryLimit(const std::string &category);
    const std::map<std::string, double> &getCategoryLimits() const;

    // === Globalny limit budżetu miesięcznego ===
    void setMonthlyBudgetLimit(double limit);
    double getMonthlyBudgetLimit() const;
};