#pragma once
#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QLineEdit>
#include <QProgressBar>
#include <QTabWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QVector>
#include <QShortcut>
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QPieSeries>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <vector>
#include <memory>
#include <string>
#include "Wallet.hpp"
#include "ITransaction.hpp"

class MainWindow : public QMainWindow {
    Q_OBJECT

private:
    // ── Dane ─────────────────────────────────────────────────────────────────
    Wallet      wallet;
    std::string m_savePassword;   // hasło do auto-zapisu (ustawiane przy logowaniu)

    // ── Zakładki ──────────────────────────────────────────────────────────────
    QTabWidget*    tabWidget;

    // ── Zakładka 1: Historia ─────────────────────────────────────────────────
    QLineEdit*     searchBar;
    QLabel*        balanceLabel;
    QTableWidget*  historyTable;
    QProgressBar*  budgetBar;
    QPushButton*   addTransactionButton;
    QPushButton*   exportCsvButton;

    // ── Zakładka 2: Analiza ──────────────────────────────────────────────────
    QChart*        pieChart;
    QChart*        barChart;
    QChartView*    chartView;
    QPieSeries*    pieSeries;
    QChartView*    barChartView;
    QWidget*       envelopeWidget;
    QVBoxLayout*   envelopeLayout;

    // ── Zakładka 3: Cele oszczędnościowe ─────────────────────────────────────
    QScrollArea*   goalsScrollArea;
    QWidget*       goalsContainer;
    QVBoxLayout*   goalsLayout;

    // ── Zakładka 4: Konta (subkonta) ─────────────────────────────────────────
    QWidget*       accountsWidget;
    QVBoxLayout*   accountsLayout;

    // ── Przycisk motywu (toolbar) ─────────────────────────────────────────────
    QPushButton*   themeToggleBtn;

    // ── Metody wewnętrzne ─────────────────────────────────────────────────────
    void updateChart();
    void updateBarChart();
    void updateBudgetBar();
    void updateEnvelopeBudgets();
    void refreshAccountsTab();
    void openLimitsDialog();
    void applySearch(const QString& text);
    void exportToCSV();
    void checkRecurringTransactions();
    void refreshGoalsTab();
    QPushButton* makeButton(const QString& text, const QString& color);
    void refreshAll();
    void autoSave();              // Cel 1: cichy zapis w tle

private slots:
    void onThemeChanged(bool isDark);

public:
    MainWindow(QWidget* parent = nullptr);
    void loadWalletData(Wallet&& loadedWallet, const std::string& password);
    void updateBalanceDisplay();
    void refreshTable();
};