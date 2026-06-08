#pragma once
#include "ITransaction.hpp"
#include <string>

// ── Transfer wewnętrzny ───────────────────────────────────────────────────────
// Przenosi środki między subkontami. Nie zmienia głównego salda portfela.
class Transfer : public ITransaction {
    friend class DatabaseManager;

private:
    double      amount;
    std::string fromAccount;
    std::string toAccount;
    std::string date;

public:
    Transfer(double amount,
             const std::string& fromAccount,
             const std::string& toAccount,
             const std::string& date);

    double      getAmount()      const override;
    std::string getCategory()    const override;
    std::string getDate()        const override;
    bool        isRecurring()    const override;
    std::string getAccountName() const override;

    // Dodatkowe akcesory specyficzne dla Transfer
    std::string getFromAccount() const;
    std::string getToAccount()   const;
};
