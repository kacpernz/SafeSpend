#include "DatabaseManager.hpp"
#include "DatabaseException.hpp"
#include "EncryptionService.hpp"
#include "Income.hpp"
#include "Expense.hpp"
#include <fstream>
#include <vector>
#include <cstring>

// ═══════════════════════════════════════════════════════════════════════════════
//  Format pliku binarnego (v3):
//
//  [SEKCJA TRANSAKCJI]
//    Liczba transakcji : size_t
//    Dla każdej:
//      typ             : char ('I' = przychód, 'E' = wydatek)
//      amount          : double
//      category        : size_t (długość) + chars
//      date            : size_t (długość) + chars
//      recurring       : uint8_t
//      accountName     : size_t (długość) + chars
//
//  [SEKCJA CELÓW]
//    Liczba celów      : size_t
//    Dla każdego celu:
//      name            : size_t (długość) + chars
//      targetAmount    : double
//      currentAmount   : double
//
//  [SEKCJA LIMITÓW KATEGORII]  ← NOWE (v3)
//    Liczba limitów    : size_t
//    Dla każdego limitu:
//      categoryName    : size_t (długość) + chars
//      limitValue      : double
// ═══════════════════════════════════════════════════════════════════════════════

void DatabaseManager::saveWallet(const Wallet& wallet,
                                 const std::string& filename,
                                 const std::string& password)
{
    std::vector<char> buffer;

    // Lambda: pakuje string (rozmiar + znaki)
    auto appendString = [&buffer](const std::string& str) {
        size_t length = str.size();
        const char* lengthPtr = reinterpret_cast<const char*>(&length);
        buffer.insert(buffer.end(), lengthPtr, lengthPtr + sizeof(size_t));
        if (length > 0)
            buffer.insert(buffer.end(), str.data(), str.data() + length);
    };

    // Lambda: pakuje double
    auto appendDouble = [&buffer](double val) {
        const char* ptr = reinterpret_cast<const char*>(&val);
        buffer.insert(buffer.end(), ptr, ptr + sizeof(double));
    };

    // Lambda: pakuje size_t
    auto appendSizeT = [&buffer](size_t val) {
        const char* ptr = reinterpret_cast<const char*>(&val);
        buffer.insert(buffer.end(), ptr, ptr + sizeof(size_t));
    };

    // ── SEKCJA TRANSAKCJI ────────────────────────────────────────────────────
    const auto& transactions = wallet.getTransactions();
    appendSizeT(transactions.size());

    for (const auto& t : transactions) {
        if (auto income = dynamic_cast<Income*>(t.get())) {
            buffer.push_back('I');
            appendDouble(income->amount);
            appendString(income->category);
            appendString(income->date);
            buffer.push_back(static_cast<char>(income->recurring ? 1 : 0));
            appendString(income->accountName);
        }
        else if (auto expense = dynamic_cast<Expense*>(t.get())) {
            buffer.push_back('E');
            appendDouble(expense->amount);
            appendString(expense->category);
            appendString(expense->date);
            buffer.push_back(static_cast<char>(expense->recurring ? 1 : 0));
            appendString(expense->accountName);
        }
    }

    // ── SEKCJA CELÓW ─────────────────────────────────────────────────────────
    const auto& goals = wallet.getGoals();
    appendSizeT(goals.size());

    for (const auto& g : goals) {
        appendString(g.name);
        appendDouble(g.targetAmount);
        appendDouble(g.currentAmount);
    }

    // ── SEKCJA LIMITÓW KATEGORII ──────────────────────────────────────────────
    const auto& limits = wallet.getCategoryLimits();
    appendSizeT(limits.size());

    for (const auto& kv : limits) {
        appendString(kv.first);
        appendDouble(kv.second);
    }

    // ── Szyfrowanie i zapis ──────────────────────────────────────────────────
    EncryptionService encryptor(password);
    encryptor.encryptDecrypt(buffer.data(), buffer.size());

    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open())
        throw DatabaseException("Nie mozna otworzyc pliku do zapisu: " + filename);

    file.write(buffer.data(), buffer.size());
    file.close();
}

void DatabaseManager::loadWallet(Wallet& wallet,
                                 const std::string& filename,
                                 const std::string& password)
{
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open())
        throw DatabaseException("Nie znaleziono pliku bazy danych: " + filename);

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    if (!file.read(buffer.data(), size))
        throw DatabaseException("Blad podczas odczytu pliku.");

    EncryptionService encryptor(password);
    encryptor.encryptDecrypt(buffer.data(), buffer.size());

    size_t index = 0;

    // Lambda: czyta string (rozmiar + znaki)
    auto readString = [&buffer, &index, size]() -> std::string {
        if (index + sizeof(size_t) > static_cast<size_t>(size))
            throw DatabaseException("Bledne haslo lub plik uszkodzony (rozmiar tekstu).");
        size_t length;
        std::memcpy(&length, &buffer[index], sizeof(size_t));
        index += sizeof(size_t);

        if (index + length > static_cast<size_t>(size))
            throw DatabaseException("Bledne haslo lub plik uszkodzony (zawartosc tekstu).");
        std::string str(&buffer[index], length);
        index += length;
        return str;
    };

    // Lambda: czyta double
    auto readDouble = [&buffer, &index, size]() -> double {
        if (index + sizeof(double) > static_cast<size_t>(size))
            throw DatabaseException("Plik uszkodzony (odczyt double).");
        double val;
        std::memcpy(&val, &buffer[index], sizeof(double));
        index += sizeof(double);
        return val;
    };

    // Lambda: czyta size_t
    auto readSizeT = [&buffer, &index, size]() -> size_t {
        if (index + sizeof(size_t) > static_cast<size_t>(size))
            throw DatabaseException("Plik uszkodzony (odczyt size_t).");
        size_t val;
        std::memcpy(&val, &buffer[index], sizeof(size_t));
        index += sizeof(size_t);
        return val;
    };

    wallet.clear();

    // ── SEKCJA TRANSAKCJI ────────────────────────────────────────────────────
    size_t txCount = readSizeT();

    for (size_t i = 0; i < txCount; ++i) {
        if (index >= static_cast<size_t>(size))
            throw DatabaseException("Plik jest uszkodzony lub podano bledne haslo!");

        char type      = buffer[index++];
        double amount  = readDouble();
        std::string cat    = readString();
        std::string date   = readString();

        if (index >= static_cast<size_t>(size))
            throw DatabaseException("Plik jest uszkodzony (brak flagi cyklicznej)!");
        bool recurring = (static_cast<uint8_t>(buffer[index++]) != 0);

        std::string accountName = readString();

        if (type == 'I') {
            wallet.addTransaction(
                std::make_unique<Income>(amount, cat, date, recurring, accountName));
        } else if (type == 'E') {
            wallet.addTransaction(
                std::make_unique<Expense>(amount, cat, date, recurring, accountName));
        } else {
            throw DatabaseException("Nieznany typ transakcji. Prawdopodobnie bledne haslo!");
        }
    }

    // ── SEKCJA CELÓW ─────────────────────────────────────────────────────────
    if (index < static_cast<size_t>(size)) {
        size_t goalCount = readSizeT();
        for (size_t i = 0; i < goalCount; ++i) {
            Goal g;
            g.name          = readString();
            g.targetAmount  = readDouble();
            g.currentAmount = readDouble();
            wallet.addGoal(g);
        }
    }

    // ── SEKCJA LIMITÓW KATEGORII ──────────────────────────────────────────────
    if (index < static_cast<size_t>(size)) {
        size_t limitCount = readSizeT();
        for (size_t i = 0; i < limitCount; ++i) {
            std::string catName = readString();
            double      lim     = readDouble();
            if (lim > 0.0)
                wallet.setCategoryLimit(catName, lim);
        }
    }

    file.close();
}