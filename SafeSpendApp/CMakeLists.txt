project(SafeSpendApp)

find_package(Qt6 REQUIRED COMPONENTS Widgets Charts)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(ICON_FILE "${CMAKE_CURRENT_SOURCE_DIR}/resources/app_icon.icns")

set(SOURCES
    src/main.cpp
    src/WelcomeWindow.cpp
    src/MainWindow.cpp
    src/AddTransactionDialog.cpp
    src/LoginDialog.cpp
    src/ThemeManager.cpp
)

add_executable(SafeSpendApp MACOSX_BUNDLE ${SOURCES} ${ICON_FILE})

set_target_properties(SafeSpendApp PROPERTIES MACOSX_BUNDLE_ICON_FILE app_icon.icns)

set_source_files_properties(${ICON_FILE} PROPERTIES MACOSX_BUNDLE_RESOURCE TRUE)

target_include_directories(SafeSpendApp PRIVATE ../CoreBudget/include)

target_link_libraries(SafeSpendApp PRIVATE Qt6::Widgets Qt6::Charts CoreBudget)