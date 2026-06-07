project(CoreBudget)

set(SOURCES
    src/Income.cpp
    src/Expense.cpp
    src/Transfer.cpp
    src/DatabaseManager.cpp
    src/EncryptionService.cpp
    src/Wallet.cpp
)

add_library(CoreBudget SHARED ${SOURCES})

target_include_directories(CoreBudget PUBLIC include)