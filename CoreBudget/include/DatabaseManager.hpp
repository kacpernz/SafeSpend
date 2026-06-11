#pragma once
#include "Wallet.hpp"
#include <string>

class DatabaseManager {
public:
    // Zapisuje cały portfel (transakcje + cele) do zaszyfrowanego pliku binarnego
    void saveWallet(const Wallet& wallet, const std::string& filename, const std::string& password);

    // Wczytuje cały portfel (transakcje + cele) z zaszyfrowanego pliku binarnego
    void loadWallet(Wallet& wallet, const std::string& filename, const std::string& password);
};