# SafeSpend

![C++](https://img.shields.io/badge/c++-%2300599C.svg?style=flat-square&logo=c%2B%2B&logoColor=white)
![Qt](https://img.shields.io/badge/Qt-%23217346.svg?style=flat-square&logo=Qt&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-%23064F8C.svg?style=flat-square&logo=cmake&logoColor=white)
![macOS](https://img.shields.io/badge/macOS-000000?style=flat-square&logo=apple&logoColor=white)

SafeSpend is a native macOS desktop application for personal finance and budget management. Built with C++ and Qt6, it provides tools for expense tracking, transaction categorization, and savings planning within a responsive graphical environment.

## Key Features

* **Envelope Budgeting:** Set specific spending limits with dynamic progress bars and track long-term savings goals.
* **Sub-accounts & Transfers:** Manage independent balances (e.g., Bank, Wallet) with zero-sum internal transfers.
* **Data Visualization:** Interactive pie and bar charts powered by QtCharts.
* **Modern UI/UX:** Clean interface with seamless, system-integrated Light and Dark mode switching.
* **Privacy-First:** 100% offline architecture. No cloud sync, no tracking.

## Architecture & Technologies

* **Modular Design:** Separated via CMake into `CoreBudget` (dynamic library/engine) and `SafeSpendApp` (Qt6 GUI).
* **Custom Binary Storage:** Fast, efficient binary serialization instead of standard text files.
* **Cryptography:** Local database secured with user-defined XOR encryption.

## Installation (Pre-built for macOS)

If you just want to use the application without building it from source:
1. Go to the [Releases](../../releases) tab.
2. Download the latest `SafeSpendApp.zip`.
3. Extract the archive and move `SafeSpendApp.app` to your `/Applications` folder.

## Building from Source

To build this project on your local machine, ensure you have CMake and Qt6 installed.

### Prerequisites
On macOS, you can install the required dependencies using [Homebrew](https://brew.sh/):

```bash
brew install cmake qt@6
```

### Build Steps

1. Clone the repository:
```bash
git clone https://github.com/kacpernz/SafeSpend.git
cd SafeSpend
```

2. Create a build directory and run CMake:
```bash
mkdir build
cd build
cmake ..
```

3. Compile the project:
```bash
make
```

4. The executable/bundle will be available in the `build` directory.
