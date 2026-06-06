#include "DatabaseManager.hpp"
#include "DatabaseException.hpp"
#include "EncryptionService.hpp"
#include "Income.hpp"
#include "Expense.hpp"
#include <fstream>
#include <vector>
#include <cstring>

void DatabaseManager::saveToBinaryFile(const std::vector<std::unique_ptr<ITransaction>>& transactions,
                                       const std::string& filename,
                                       const std::string& password)
{
    std::vector<char> buffer;

    // Pomocnicza lambda: pakuje string (rozmiar + znaki)
    auto appendString = [&buffer](const std::string& str) {
        size_t length = str.size();
        const char* lengthPtr = reinterpret_cast<const char*>(&length);
        buffer.insert(buffer.end(), lengthPtr, lengthPtr + sizeof(size_t));
        if (length > 0)
            buffer.insert(buffer.end(), str.data(), str.data() + length);
    };

    for (const auto& t : transactions) {
        if (auto income = dynamic_cast<Income*>(t.get())) {
            buffer.push_back('I');
            const char* amountPtr = reinterpret_cast<const char*>(&income->amount);
            buffer.insert(buffer.end(), amountPtr, amountPtr + sizeof(double));
            appendString(income->category);
            appendString(income->date);
            // Flaga cykliczna — 1 bajt
            buffer.push_back(static_cast<char>(income->recurring ? 1 : 0));
        }
        else if (auto expense = dynamic_cast<Expense*>(t.get())) {
            buffer.push_back('E');
            const char* amountPtr = reinterpret_cast<const char*>(&expense->amount);
            buffer.insert(buffer.end(), amountPtr, amountPtr + sizeof(double));
            appendString(expense->category);
            appendString(expense->date);
            // Flaga cykliczna — 1 bajt
            buffer.push_back(static_cast<char>(expense->recurring ? 1 : 0));
        }
    }

    EncryptionService encryptor(password);
    encryptor.encryptDecrypt(buffer.data(), buffer.size());

    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open())
        throw DatabaseException("Nie mozna otworzyc pliku do zapisu: " + filename);

    file.write(buffer.data(), buffer.size());
    file.close();
}

std::vector<std::unique_ptr<ITransaction>>
DatabaseManager::loadFromBinaryFile(const std::string& filename, const std::string& password)
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

    std::vector<std::unique_ptr<ITransaction>> loadedTransactions;
    size_t index = 0;

    // Lambda: czyta string (rozmiar + znaki)
    auto readString = [&buffer, &index]() -> std::string {
        if (index + sizeof(size_t) > buffer.size())
            throw DatabaseException("Bledne haslo lub plik uszkodzony (rozmiar tekstu).");
        size_t length;
        std::memcpy(&length, &buffer[index], sizeof(size_t));
        index += sizeof(size_t);

        if (index + length > buffer.size())
            throw DatabaseException("Bledne haslo lub plik uszkodzony (zawartosc tekstu).");
        std::string str(&buffer[index], length);
        index += length;
        return str;
    };

    while (index < static_cast<size_t>(size)) {
        if (index >= buffer.size())
            throw DatabaseException("Plik jest uszkodzony lub podano bledne haslo!");

        char type = buffer[index++];

        if (index + sizeof(double) > buffer.size())
            throw DatabaseException("Plik jest uszkodzony lub podano bledne haslo!");

        double amount;
        std::memcpy(&amount, &buffer[index], sizeof(double));
        index += sizeof(double);

        std::string category = readString();
        std::string date     = readString();

        // Flaga cykliczna — 1 bajt
        if (index >= buffer.size())
            throw DatabaseException("Plik jest uszkodzony lub podano bledne haslo (brak flagi cyklicznej)!");
        bool recurring = (static_cast<uint8_t>(buffer[index++]) != 0);

        if (type == 'I') {
            loadedTransactions.push_back(std::make_unique<Income>(amount, category, date, recurring));
        } else if (type == 'E') {
            loadedTransactions.push_back(std::make_unique<Expense>(amount, category, date, recurring));
        } else {
            throw DatabaseException("Nieznany typ transakcji. Prawdopodobnie bledne haslo!");
        }
    }

    file.close();
    return loadedTransactions;
}