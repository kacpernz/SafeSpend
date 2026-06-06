project(SafeSpendApp)

find_package(Qt6 REQUIRED COMPONENTS Widgets Charts)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(SOURCES
    src/main.cpp
    src/WelcomeWindow.cpp
    src/MainWindow.cpp
    src/AddTransactionDialog.cpp
    src/LoginDialog.cpp
)

add_executable(SafeSpendApp ${SOURCES})

target_include_directories(SafeSpendApp PRIVATE ../CoreBudget/include)

target_link_libraries(SafeSpendApp PRIVATE Qt6::Widgets Qt6::Charts CoreBudget)