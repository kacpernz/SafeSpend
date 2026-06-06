#pragma once
#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QLineEdit>
#include <QProgressBar>
#include <QTabWidget>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <vector>
#include <memory>
#include "Wallet.hpp"
#include "ITransaction.hpp"

class MainWindow : public QMainWindow {
    Q_OBJECT

private:
    // ── Dane ─────────────────────────────────────────────────────────────────
    Wallet wallet;

    // ── Zakładki ──────────────────────────────────────────────────────────────
    QTabWidget*    tabWidget;

    // ── Zakładka 1: Historia ─────────────────────────────────────────────────
    QLineEdit*     searchBar;
    QLabel*        balanceLabel;
    QTableWidget*  historyTable;
    QProgressBar*  budgetBar;
    QPushButton*   addTransactionButton;
    QPushButton*   saveButton;
    QPushButton*   exportCsvButton;

    // ── Zakładka 2: Analiza (wykres) ─────────────────────────────────────────
    QChartView*    chartView;
    QPieSeries*    pieSeries;

    // ── Metody wewnętrzne ─────────────────────────────────────────────────────
    void updateChart();
    void updateBudgetBar();
    void applySearch(const QString& text);
    void exportToCSV();
    void checkRecurringTransactions();

public:
    MainWindow(QWidget* parent = nullptr);
    void loadWalletData(std::vector<std::unique_ptr<ITransaction>> transactions);
    void updateBalanceDisplay();
    void refreshTable();
};