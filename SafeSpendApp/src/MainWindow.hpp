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
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
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

    // ── Zakładka 2: Analiza ──────────────────────────────────────────────────
    QChartView*    chartView;      // wykres kołowy
    QPieSeries*    pieSeries;
    QChartView*    barChartView;   // wykres słupkowy trendów
    QWidget*       envelopeWidget; // budżetowanie kopertowe
    QVBoxLayout*   envelopeLayout;

    // ── Zakładka 3: Cele oszczędnościowe ─────────────────────────────────────
    QScrollArea*   goalsScrollArea;
    QWidget*       goalsContainer;
    QVBoxLayout*   goalsLayout;

    // ── Metody wewnętrzne ─────────────────────────────────────────────────────
    void updateChart();
    void updateBarChart();
    void updateBudgetBar();
    void updateEnvelopeBudgets();
    void applySearch(const QString& text);
    void exportToCSV();
    void checkRecurringTransactions();
    void refreshGoalsTab();
    QPushButton* makeButton(const QString& text, const QString& color);

public:
    MainWindow(QWidget* parent = nullptr);
    void loadWalletData(Wallet&& loadedWallet);
    void updateBalanceDisplay();
    void refreshTable();
};