#include "MainWindow.hpp"
#include "AddTransactionDialog.hpp"
#include "LoginDialog.hpp"
#include "Income.hpp"
#include "Expense.hpp"
#include "DatabaseManager.hpp"
#include "DatabaseException.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSplitter>
#include <QWidget>
#include <QMessageBox>
#include <QHeaderView>
#include <QDate>
#include <QFileDialog>
#include <QTextStream>
#include <QFile>
#include <QMap>
#include <QStringList>
#include <QInputDialog>
#include <QScrollArea>
#include <QFrame>
#include <cmath>
#include <algorithm>

#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>

// ═══════════════════════════════════════════════════════════════════════════════
//  Stałe
// ═══════════════════════════════════════════════════════════════════════════════
static constexpr double MONTHLY_BUDGET_LIMIT = 5000.0;

// Limity kopertowe dla kategorii (case-insensitive matching)
static const QMap<QString, double> ENVELOPE_LIMITS = {
    {"Jedzenie",   800.0},
    {"Rata",      1500.0},
    {"Transport",  400.0},
    {"Rozrywka",   300.0},
};

// ═══════════════════════════════════════════════════════════════════════════════
//  Pomocnicze
// ═══════════════════════════════════════════════════════════════════════════════
QPushButton* MainWindow::makeButton(const QString& text, const QString& color) {
    QPushButton* btn = new QPushButton(text, this);
    btn->setFixedHeight(42);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setStyleSheet(QString(R"(
        QPushButton {
            background-color: %1;
            color: white;
            border: none;
            border-radius: 8px;
            font-size: 13px;
            font-weight: bold;
            padding: 0 16px;
        }
        QPushButton:hover   { background-color: %1; filter: brightness(1.2); }
        QPushButton:pressed { background-color: %1; opacity: 0.75; }
    )").arg(color));
    return btn;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Konstruktor
// ═══════════════════════════════════════════════════════════════════════════════
MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("SafeSpend");
    resize(1000, 700);

    // ── Centralny widget + layout ────────────────────────────────────────────
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* rootLayout = new QVBoxLayout(centralWidget);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // ── QTabWidget ───────────────────────────────────────────────────────────
    tabWidget = new QTabWidget(this);
    tabWidget->setStyleSheet(R"(
        QTabWidget::pane {
            border: none;
            background-color: #111827;
        }
        QTabBar::tab {
            background: #1e293b;
            color: #94a3b8;
            padding: 10px 28px;
            font-size: 13px;
            font-weight: bold;
            border: none;
            min-width: 120px;
        }
        QTabBar::tab:selected {
            background: #0d6efd;
            color: white;
        }
        QTabBar::tab:hover:!selected {
            background: #273549;
            color: #cbd5e1;
        }
    )");
    rootLayout->addWidget(tabWidget);
    setCentralWidget(centralWidget);

    // ╔═══════════════════════════════════════════════════════════════════════╗
    // ║  ZAKŁADKA 1 — Historia transakcji                                     ║
    // ╚═══════════════════════════════════════════════════════════════════════╝
    QWidget*     tab1    = new QWidget(this);
    QVBoxLayout* layout1 = new QVBoxLayout(tab1);
    layout1->setContentsMargins(16, 16, 16, 16);
    layout1->setSpacing(10);

    // ── Pasek wyszukiwania ───────────────────────────────────────────────────
    searchBar = new QLineEdit(this);
    searchBar->setPlaceholderText("🔍  Szukaj transakcji (data, kategoria, kwota)...");
    searchBar->setFixedHeight(40);
    searchBar->setStyleSheet(R"(
        QLineEdit {
            background-color: #1e293b;
            color: #f0f0f0;
            border: 1px solid #334155;
            border-radius: 8px;
            padding: 0 14px;
            font-size: 13px;
        }
        QLineEdit:focus { border: 1px solid #3d8bfd; }
    )");
    layout1->addWidget(searchBar);

    // ── Etykieta salda ────────────────────────────────────────────────────────
    balanceLabel = new QLabel("Obecne saldo: 0.00 PLN", this);
    QFont font = balanceLabel->font();
    font.setPointSize(22);
    font.setBold(true);
    balanceLabel->setFont(font);
    balanceLabel->setStyleSheet("color: #38bdf8; padding: 4px 2px;");
    layout1->addWidget(balanceLabel);

    // ── Tabela historii (6 kolumn: +Konto) ───────────────────────────────────
    historyTable = new QTableWidget(0, 6, this);
    historyTable->setHorizontalHeaderLabels(
        {"Data", "Kategoria", "Typ", "Kwota (PLN)", "Konto", "Cykliczna"});
    historyTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    historyTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    historyTable->setAlternatingRowColors(true);
    historyTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    historyTable->setStyleSheet(R"(
        QTableWidget {
            background-color: #1e293b;
            alternate-background-color: #243044;
            color: #e2e8f0;
            gridline-color: #334155;
            border: none;
            border-radius: 6px;
        }
        QTableWidget::item:selected {
            background-color: #1a4f9f;
        }
        QHeaderView::section {
            background-color: #0f172a;
            color: #94a3b8;
            font-weight: bold;
            padding: 6px;
            border: none;
            border-bottom: 1px solid #334155;
        }
    )");
    layout1->addWidget(historyTable);

    // ── Pasek budżetu miesięcznego ────────────────────────────────────────────
    QLabel* budgetLabel = new QLabel(
        QString("Budżet miesięczny (limit: %1 PLN):").arg(MONTHLY_BUDGET_LIMIT, 0, 'f', 0), this);
    budgetLabel->setStyleSheet("color: #94a3b8; font-size: 12px;");
    layout1->addWidget(budgetLabel);

    budgetBar = new QProgressBar(this);
    budgetBar->setRange(0, 100);
    budgetBar->setValue(0);
    budgetBar->setFixedHeight(22);
    budgetBar->setTextVisible(true);
    budgetBar->setFormat("0 / 5000 PLN  (0%)");
    budgetBar->setStyleSheet(R"(
        QProgressBar {
            background-color: #1e293b;
            border: 1px solid #334155;
            border-radius: 6px;
            text-align: center;
            color: white;
            font-size: 12px;
        }
        QProgressBar::chunk {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #1a6fdb, stop:1 #0d6efd);
            border-radius: 6px;
        }
    )");
    layout1->addWidget(budgetBar);

    // ── Przyciski akcji ───────────────────────────────────────────────────────
    QHBoxLayout* buttonRow = new QHBoxLayout();
    buttonRow->setSpacing(10);

    addTransactionButton = makeButton("➕  Dodaj transakcję",        "#0d6efd");
    saveButton           = makeButton("💾  Zapisz zaszyfrowane dane", "#0f7a4e");
    exportCsvButton      = makeButton("📄  Eksportuj do CSV",         "#7c3aed");

    buttonRow->addWidget(addTransactionButton);
    buttonRow->addWidget(saveButton);
    buttonRow->addWidget(exportCsvButton);
    layout1->addLayout(buttonRow);

    tabWidget->addTab(tab1, "📋  Historia");

    // ╔═══════════════════════════════════════════════════════════════════════╗
    // ║  ZAKŁADKA 2 — Analiza (wykres kołowy + słupkowy + koperty)            ║
    // ╚═══════════════════════════════════════════════════════════════════════╝
    QWidget*     tab2    = new QWidget(this);
    QVBoxLayout* layout2 = new QVBoxLayout(tab2);
    layout2->setContentsMargins(12, 12, 12, 12);
    layout2->setSpacing(12);

    // ── Nagłówek ─────────────────────────────────────────────────────────────
    QLabel* analysisTitle = new QLabel("📊  Analiza finansów", tab2);
    analysisTitle->setStyleSheet(
        "color: #e2e8f0; font-size: 16px; font-weight: bold; padding: 4px;");
    layout2->addWidget(analysisTitle);

    // ── Górny podział: wykres kołowy | wykres słupkowy ───────────────────────
    QSplitter* chartSplitter = new QSplitter(Qt::Horizontal, tab2);
    chartSplitter->setStyleSheet("QSplitter::handle { background: #334155; width: 2px; }");

    // Wykres kołowy
    pieSeries = new QPieSeries(this);
    QChart* pieChart = new QChart();
    pieChart->addSeries(pieSeries);
    pieChart->setTitle("Wydatki wg kategorii");
    pieChart->setTitleFont(QFont("Arial", 12, QFont::Bold));
    pieChart->setBackgroundBrush(QBrush(QColor("#111827")));
    pieChart->setTitleBrush(QBrush(QColor("#e2e8f0")));
    pieChart->legend()->setLabelColor(QColor("#94a3b8"));
    pieChart->legend()->setAlignment(Qt::AlignBottom);
    pieChart->setAnimationOptions(QChart::SeriesAnimations);

    chartView = new QChartView(pieChart, this);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setStyleSheet("background: #111827; border: none;");
    chartView->setMinimumHeight(280);

    // Wykres słupkowy
    QChart* barChart = new QChart();
    barChart->setTitle("Przychody vs Wydatki (miesiące)");
    barChart->setTitleFont(QFont("Arial", 12, QFont::Bold));
    barChart->setBackgroundBrush(QBrush(QColor("#111827")));
    barChart->setTitleBrush(QBrush(QColor("#e2e8f0")));
    barChart->legend()->setLabelColor(QColor("#94a3b8"));
    barChart->legend()->setAlignment(Qt::AlignBottom);
    barChart->setAnimationOptions(QChart::SeriesAnimations);

    barChartView = new QChartView(barChart, this);
    barChartView->setRenderHint(QPainter::Antialiasing);
    barChartView->setStyleSheet("background: #111827; border: none;");
    barChartView->setMinimumHeight(280);

    chartSplitter->addWidget(chartView);
    chartSplitter->addWidget(barChartView);
    chartSplitter->setSizes({500, 500});
    layout2->addWidget(chartSplitter, 2);

    // ── Separator ────────────────────────────────────────────────────────────
    QFrame* separator = new QFrame(tab2);
    separator->setFrameShape(QFrame::HLine);
    separator->setStyleSheet("color: #334155; margin: 4px 0;");
    layout2->addWidget(separator);

    // ── Budżetowanie kopertowe ────────────────────────────────────────────────
    QLabel* envelopeTitle = new QLabel("🗂  Budżetowanie kopertowe (bieżący miesiąc)", tab2);
    envelopeTitle->setStyleSheet(
        "color: #94a3b8; font-size: 13px; font-weight: bold; padding: 2px;");
    layout2->addWidget(envelopeTitle);

    envelopeWidget = new QWidget(tab2);
    envelopeLayout = new QVBoxLayout(envelopeWidget);
    envelopeLayout->setContentsMargins(4, 4, 4, 4);
    envelopeLayout->setSpacing(6);
    envelopeWidget->setStyleSheet("background: transparent;");
    layout2->addWidget(envelopeWidget, 1);

    tabWidget->addTab(tab2, "📊  Analiza");

    // ╔═══════════════════════════════════════════════════════════════════════╗
    // ║  ZAKŁADKA 3 — Cele oszczędnościowe                                    ║
    // ╚═══════════════════════════════════════════════════════════════════════╝
    QWidget*     tab3    = new QWidget(this);
    QVBoxLayout* layout3 = new QVBoxLayout(tab3);
    layout3->setContentsMargins(16, 16, 16, 16);
    layout3->setSpacing(12);

    QLabel* goalsTitle = new QLabel("🎯  Cele Oszczędnościowe", tab3);
    goalsTitle->setStyleSheet(
        "color: #e2e8f0; font-size: 16px; font-weight: bold; padding: 4px;");
    layout3->addWidget(goalsTitle);

    // Przyciski
    QHBoxLayout* goalsBtnRow = new QHBoxLayout();
    goalsBtnRow->setSpacing(10);
    QPushButton* newGoalBtn  = makeButton("🎯  Nowy cel",   "#0f7a4e");
    QPushButton* fundGoalBtn = makeButton("💰  Zasil cel",  "#0d6efd");
    QPushButton* saveGoalBtn = makeButton("💾  Zapisz",     "#7c3aed");
    newGoalBtn->setParent(tab3);
    fundGoalBtn->setParent(tab3);
    saveGoalBtn->setParent(tab3);
    goalsBtnRow->addWidget(newGoalBtn);
    goalsBtnRow->addWidget(fundGoalBtn);
    goalsBtnRow->addWidget(saveGoalBtn);
    goalsBtnRow->addStretch();
    layout3->addLayout(goalsBtnRow);

    // Scroll area z celami
    goalsScrollArea = new QScrollArea(tab3);
    goalsScrollArea->setWidgetResizable(true);
    goalsScrollArea->setStyleSheet(R"(
        QScrollArea {
            background: #111827;
            border: none;
        }
        QScrollBar:vertical {
            background: #1e293b;
            width: 8px;
            border-radius: 4px;
        }
        QScrollBar::handle:vertical {
            background: #334155;
            border-radius: 4px;
        }
    )");

    goalsContainer = new QWidget();
    goalsContainer->setStyleSheet("background: #111827;");
    goalsLayout = new QVBoxLayout(goalsContainer);
    goalsLayout->setContentsMargins(8, 8, 8, 8);
    goalsLayout->setSpacing(10);
    goalsLayout->addStretch();

    goalsScrollArea->setWidget(goalsContainer);
    layout3->addWidget(goalsScrollArea);

    tabWidget->addTab(tab3, "🎯  Cele");

    // ═════════════════════════════════════════════════════════════════════════
    //  Połączenia sygnałów
    // ═════════════════════════════════════════════════════════════════════════

    connect(searchBar, &QLineEdit::textChanged, this, &MainWindow::applySearch);

    // Dodawanie transakcji
    connect(addTransactionButton, &QPushButton::clicked, this, [this]() {
        AddTransactionDialog dialog(this);
        if (dialog.exec() == QDialog::Accepted) {
            double  amount      = dialog.getAmount();
            QString category    = dialog.getCategory();
            QString type        = dialog.getTransactionType();
            bool    recurring   = dialog.getIsRecurring();
            QString accountName = dialog.getAccountName();
            QString today       = QDate::currentDate().toString("yyyy-MM-dd");

            if (type == "Przychod") {
                wallet.addTransaction(
                    std::make_unique<Income>(amount, category.toStdString(),
                                             today.toStdString(), recurring,
                                             accountName.toStdString()));
            } else {
                wallet.addTransaction(
                    std::make_unique<Expense>(amount, category.toStdString(),
                                              today.toStdString(), recurring,
                                              accountName.toStdString()));
            }
            refreshTable();
            updateBudgetBar();
            updateChart();
            updateBarChart();
            updateEnvelopeBudgets();
            updateBalanceDisplay();
        }
    });

    // Zapis zaszyfrowany
    connect(saveButton, &QPushButton::clicked, this, [this]() {
        LoginDialog loginDialog(this);
        if (loginDialog.exec() == QDialog::Accepted) {
            DatabaseManager dbManager;
            try {
                dbManager.saveWallet(wallet, "finanse_baza.bin",
                                     loginDialog.getPassword().toStdString());
                QMessageBox::information(this, "Sukces",
                    "✔  Dane zostały bezpiecznie zaszyfrowane i zapisane!");
            } catch (const DatabaseException& e) {
                QMessageBox::critical(this, "Błąd",
                    QString("Błąd zapisu: %1").arg(e.what()));
            }
        }
    });

    connect(exportCsvButton, &QPushButton::clicked, this, &MainWindow::exportToCSV);

    // Nowy cel
    connect(newGoalBtn, &QPushButton::clicked, this, [this]() {
        bool ok;
        QString name = QInputDialog::getText(this, "Nowy cel oszczędnościowy",
                                             "Nazwa celu:", QLineEdit::Normal, "", &ok);
        if (!ok || name.trimmed().isEmpty()) return;

        double target = QInputDialog::getDouble(this, "Kwota docelowa",
                                                "Docelowa kwota (PLN):",
                                                0.0, 0.0, 1000000.0, 2, &ok);
        if (!ok || target <= 0.0) return;

        Goal g;
        g.name          = name.trimmed().toStdString();
        g.targetAmount  = target;
        g.currentAmount = 0.0;
        wallet.addGoal(g);
        refreshGoalsTab();
    });

    // Zasil cel
    connect(fundGoalBtn, &QPushButton::clicked, this, [this]() {
        const auto& goals = wallet.getGoals();
        if (goals.empty()) {
            QMessageBox::information(this, "Brak celów", "Najpierw dodaj cel oszczędnościowy.");
            return;
        }

        QStringList items;
        for (const auto& g : goals)
            items << QString::fromStdString(g.name)
                         + QString(" (%1/%2 PLN)")
                               .arg(g.currentAmount, 0, 'f', 2)
                               .arg(g.targetAmount, 0, 'f', 2);

        bool ok;
        QString chosen = QInputDialog::getItem(this, "Zasil cel",
                                               "Wybierz cel:", items, 0, false, &ok);
        if (!ok) return;

        int idx = items.indexOf(chosen);
        if (idx < 0) return;

        double amount = QInputDialog::getDouble(this, "Kwota",
                                                "Kwota do wpłacenia (PLN):",
                                                0.0, 0.01, 1000000.0, 2, &ok);
        if (!ok || amount <= 0.0) return;

        double balance = wallet.calculateBalance();
        if (amount > balance) {
            QMessageBox::warning(this, "Brak środków",
                QString("Saldo (%1 PLN) jest za niskie!").arg(balance, 0, 'f', 2));
            return;
        }

        // Dodaj wydatek (odejmuje z salda)
        QString goalExpCat = "Cel: " + QString::fromStdString(goals[idx].name);
        QString today = QDate::currentDate().toString("yyyy-MM-dd");
        wallet.addTransaction(
            std::make_unique<Expense>(amount, goalExpCat.toStdString(),
                                      today.toStdString(), false, "Oszczędności"));

        // Zasil cel
        wallet.fundGoal(static_cast<size_t>(idx), amount);

        refreshTable();
        updateBudgetBar();
        updateChart();
        updateBarChart();
        updateEnvelopeBudgets();
        updateBalanceDisplay();
        refreshGoalsTab();
    });

    // Zapisz (z zakładki celów)
    connect(saveGoalBtn, &QPushButton::clicked, this, [this]() {
        LoginDialog loginDialog(this);
        if (loginDialog.exec() == QDialog::Accepted) {
            DatabaseManager dbManager;
            try {
                dbManager.saveWallet(wallet, "finanse_baza.bin",
                                     loginDialog.getPassword().toStdString());
                QMessageBox::information(this, "Sukces",
                    "✔  Dane zostały bezpiecznie zaszyfrowane i zapisane!");
            } catch (const DatabaseException& e) {
                QMessageBox::critical(this, "Błąd",
                    QString("Błąd zapisu: %1").arg(e.what()));
            }
        }
    });

    // Inicjalizacja wizualizacji
    updateBalanceDisplay();
    refreshTable();
    updateBudgetBar();
    updateChart();
    updateBarChart();
    updateEnvelopeBudgets();
    refreshGoalsTab();
}

// ═══════════════════════════════════════════════════════════════════════════════
//  loadWalletData — wywoływane z WelcomeWindow po zalogowaniu
// ═══════════════════════════════════════════════════════════════════════════════
void MainWindow::loadWalletData(Wallet&& loadedWallet) {
    wallet = std::move(loadedWallet);
    checkRecurringTransactions();
    refreshTable();
    updateBalanceDisplay();
    updateBudgetBar();
    updateChart();
    updateBarChart();
    updateEnvelopeBudgets();
    refreshGoalsTab();
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Krok 1 — Aktualizacja paska budżetu miesięcznego
// ═══════════════════════════════════════════════════════════════════════════════
void MainWindow::updateBudgetBar() {
    QDate today     = QDate::currentDate();
    int   thisYear  = today.year();
    int   thisMonth = today.month();

    double monthlyExpenses = 0.0;
    for (const auto& t : wallet.getTransactions()) {
        if (dynamic_cast<Expense*>(t.get())) {
            QDate txDate = QDate::fromString(
                QString::fromStdString(t->getDate()), "yyyy-MM-dd");
            if (txDate.year() == thisYear && txDate.month() == thisMonth)
                monthlyExpenses += t->getAmount();
        }
    }

    double ratio   = monthlyExpenses / MONTHLY_BUDGET_LIMIT;
    int    percent = static_cast<int>(std::min(ratio * 100.0, 100.0));

    budgetBar->setValue(percent);
    budgetBar->setFormat(
        QString("%1 / %2 PLN  (%3%)")
            .arg(monthlyExpenses, 0, 'f', 0)
            .arg(MONTHLY_BUDGET_LIMIT, 0, 'f', 0)
            .arg(percent));

    if (percent >= 90) {
        budgetBar->setStyleSheet(R"(
            QProgressBar {
                background-color: #1e293b;
                border: 1px solid #7f1d1d;
                border-radius: 6px;
                text-align: center;
                color: white;
                font-size: 12px;
            }
            QProgressBar::chunk {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #b91c1c, stop:1 #ef4444);
                border-radius: 6px;
            }
        )");
    } else if (percent >= 70) {
        budgetBar->setStyleSheet(R"(
            QProgressBar {
                background-color: #1e293b;
                border: 1px solid #78350f;
                border-radius: 6px;
                text-align: center;
                color: white;
                font-size: 12px;
            }
            QProgressBar::chunk {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #b45309, stop:1 #f59e0b);
                border-radius: 6px;
            }
        )");
    } else {
        budgetBar->setStyleSheet(R"(
            QProgressBar {
                background-color: #1e293b;
                border: 1px solid #334155;
                border-radius: 6px;
                text-align: center;
                color: white;
                font-size: 12px;
            }
            QProgressBar::chunk {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #1a6fdb, stop:1 #0d6efd);
                border-radius: 6px;
            }
        )");
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Krok 3 — Wykres kołowy
// ═══════════════════════════════════════════════════════════════════════════════
void MainWindow::updateChart() {
    pieSeries->clear();

    QMap<QString, double> categoryTotals;
    for (const auto& t : wallet.getTransactions()) {
        if (dynamic_cast<Expense*>(t.get())) {
            QString cat = QString::fromStdString(t->getCategory());
            categoryTotals[cat] += t->getAmount();
        }
    }

    if (categoryTotals.isEmpty()) {
        auto* slice = pieSeries->append("Brak wydatków", 1.0);
        slice->setColor(QColor("#334155"));
        slice->setLabelColor(QColor("#94a3b8"));
        return;
    }

    static const QStringList palette = {
        "#0d6efd", "#06d6a0", "#ffd166", "#ef476f", "#118ab2",
        "#8338ec", "#fb5607", "#3a86ff", "#38bdf8", "#a3e635"
    };
    int colorIdx = 0;

    for (auto it = categoryTotals.constBegin(); it != categoryTotals.constEnd(); ++it) {
        auto* slice = pieSeries->append(
            QString("%1 (%2 PLN)").arg(it.key()).arg(it.value(), 0, 'f', 2),
            it.value());
        slice->setColor(QColor(palette[colorIdx % palette.size()]));
        slice->setLabelColor(QColor("#e2e8f0"));
        slice->setLabelVisible(it.value() / pieSeries->sum() > 0.05);
        ++colorIdx;
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Krok 3 — Wykres słupkowy trendów (miesięczny)
// ═══════════════════════════════════════════════════════════════════════════════
void MainWindow::updateBarChart() {
    QChart* chart = barChartView->chart();
    chart->removeAllSeries();
    // Usuń stare osie
    const auto oldAxes = chart->axes();
    for (auto* axis : oldAxes)
        chart->removeAxis(axis);

    // Zbierz dane per miesiąc
    QMap<QString, double> monthlyIncome;
    QMap<QString, double> monthlyExpense;

    for (const auto& t : wallet.getTransactions()) {
        QDate txDate = QDate::fromString(
            QString::fromStdString(t->getDate()), "yyyy-MM-dd");
        QString monthKey = txDate.toString("yyyy-MM");

        if (dynamic_cast<Income*>(t.get())) {
            monthlyIncome[monthKey]  += t->getAmount();
        } else if (dynamic_cast<Expense*>(t.get())) {
            monthlyExpense[monthKey] += t->getAmount();
        }
    }

    // Unikalne miesiące (posortowane)
    QStringList months;
    for (const auto& k : monthlyIncome.keys())
        if (!months.contains(k)) months << k;
    for (const auto& k : monthlyExpense.keys())
        if (!months.contains(k)) months << k;
    std::sort(months.begin(), months.end());

    if (months.isEmpty()) {
        // Placeholder
        QBarSet* placeholder = new QBarSet("Brak danych");
        placeholder->setColor(QColor("#334155"));
        *placeholder << 0;
        QBarSeries* series = new QBarSeries();
        series->append(placeholder);
        chart->addSeries(series);
        return;
    }

    QBarSet* incomeSet  = new QBarSet("Przychody");
    QBarSet* expenseSet = new QBarSet("Wydatki");
    incomeSet->setColor(QColor("#22c55e"));
    expenseSet->setColor(QColor("#ef4444"));
    incomeSet->setLabelColor(QColor("#e2e8f0"));
    expenseSet->setLabelColor(QColor("#e2e8f0"));

    QStringList displayMonths; // sformatowane etykiety osi X
    for (const QString& m : months) {
        incomeSet->append(monthlyIncome.value(m, 0.0));
        expenseSet->append(monthlyExpense.value(m, 0.0));
        // Formatowanie: "2024-05" → "Maj 2024"
        QDate d = QDate::fromString(m + "-01", "yyyy-MM-dd");
        displayMonths << d.toString("MMM yy");
    }

    QBarSeries* barSeries = new QBarSeries();
    barSeries->append(incomeSet);
    barSeries->append(expenseSet);
    barSeries->setLabelsVisible(false);
    chart->addSeries(barSeries);

    QBarCategoryAxis* axisX = new QBarCategoryAxis();
    axisX->append(displayMonths);
    axisX->setLabelsColor(QColor("#94a3b8"));
    axisX->setGridLineColor(QColor("#1e293b"));
    chart->addAxis(axisX, Qt::AlignBottom);
    barSeries->attachAxis(axisX);

    QValueAxis* axisY = new QValueAxis();
    axisY->setLabelFormat("%.0f");
    axisY->setLabelsColor(QColor("#94a3b8"));
    axisY->setGridLineColor(QColor("#1e293b"));
    axisY->setLinePenColor(QColor("#334155"));
    chart->addAxis(axisY, Qt::AlignLeft);
    barSeries->attachAxis(axisY);

    chart->legend()->setLabelColor(QColor("#94a3b8"));
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Krok 5 — Budżetowanie kopertowe
// ═══════════════════════════════════════════════════════════════════════════════
void MainWindow::updateEnvelopeBudgets() {
    // Wyczyść poprzednie widgety
    while (QLayoutItem* item = envelopeLayout->takeAt(0)) {
        if (QWidget* w = item->widget()) w->deleteLater();
        delete item;
    }

    QDate today     = QDate::currentDate();
    int   thisYear  = today.year();
    int   thisMonth = today.month();

    // Sumuj wydatki bieżącego miesiąca per kategoria
    QMap<QString, double> spent;
    for (const auto& t : wallet.getTransactions()) {
        if (dynamic_cast<Expense*>(t.get())) {
            QDate txDate = QDate::fromString(
                QString::fromStdString(t->getDate()), "yyyy-MM-dd");
            if (txDate.year() == thisYear && txDate.month() == thisMonth) {
                QString cat = QString::fromStdString(t->getCategory());
                // Case-insensitive matching z limitami
                for (auto it = ENVELOPE_LIMITS.constBegin(); it != ENVELOPE_LIMITS.constEnd(); ++it) {
                    if (cat.compare(it.key(), Qt::CaseInsensitive) == 0) {
                        spent[it.key()] += t->getAmount();
                        break;
                    }
                }
            }
        }
    }

    static const QMap<QString, QString> categoryIcons = {
        {"Jedzenie",  "🍕"},
        {"Rata",      "🏠"},
        {"Transport", "🚗"},
        {"Rozrywka",  "🎮"},
    };

    QGridLayout* grid = new QGridLayout();
    grid->setSpacing(8);
    int col = 0;

    for (auto it = ENVELOPE_LIMITS.constBegin(); it != ENVELOPE_LIMITS.constEnd(); ++it) {
        const QString& cat    = it.key();
        double         limit  = it.value();
        double         spentV = spent.value(cat, 0.0);
        int            pct    = static_cast<int>(std::min(spentV / limit * 100.0, 100.0));

        QWidget*     card       = new QWidget();
        QVBoxLayout* cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(12, 10, 12, 10);
        cardLayout->setSpacing(4);
        card->setStyleSheet(R"(
            background: #1e293b;
            border: 1px solid #334155;
            border-radius: 8px;
        )");

        QString icon = categoryIcons.value(cat, "📂");
        QLabel* nameLabel = new QLabel(
            QString("%1  %2").arg(icon).arg(cat));
        nameLabel->setStyleSheet("color: #e2e8f0; font-weight: bold; font-size: 13px;");

        QLabel* amtLabel = new QLabel(
            QString("%1 / %2 PLN").arg(spentV, 0, 'f', 0).arg(limit, 0, 'f', 0));
        amtLabel->setStyleSheet("color: #94a3b8; font-size: 12px;");

        QProgressBar* bar = new QProgressBar();
        bar->setRange(0, 100);
        bar->setValue(pct);
        bar->setFixedHeight(14);
        bar->setTextVisible(false);

        QString barColor = (pct >= 90) ? "#ef4444" : (pct >= 70) ? "#f59e0b" : "#22c55e";
        bar->setStyleSheet(QString(R"(
            QProgressBar {
                background-color: #0f172a;
                border: none;
                border-radius: 4px;
            }
            QProgressBar::chunk {
                background-color: %1;
                border-radius: 4px;
            }
        )").arg(barColor));

        cardLayout->addWidget(nameLabel);
        cardLayout->addWidget(amtLabel);
        cardLayout->addWidget(bar);

        grid->addWidget(card, 0, col++);
    }

    QWidget* gridWrapper = new QWidget();
    gridWrapper->setLayout(grid);
    gridWrapper->setStyleSheet("background: transparent;");
    envelopeLayout->addWidget(gridWrapper);
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Krok 4 — Zakładka celów oszczędnościowych
// ═══════════════════════════════════════════════════════════════════════════════
void MainWindow::refreshGoalsTab() {
    // Wyczyść stare widgety (oprócz stretch)
    while (QLayoutItem* item = goalsLayout->takeAt(0)) {
        if (QWidget* w = item->widget()) w->deleteLater();
        delete item;
    }

    const auto& goals = wallet.getGoals();

    if (goals.empty()) {
        QLabel* emptyLabel = new QLabel("Brak celów oszczędnościowych.\nKliknij '🎯 Nowy cel', aby dodać pierwszy.");
        emptyLabel->setStyleSheet("color: #475569; font-size: 14px; padding: 20px;");
        emptyLabel->setAlignment(Qt::AlignCenter);
        goalsLayout->addWidget(emptyLabel);
        goalsLayout->addStretch();
        return;
    }

    for (size_t i = 0; i < goals.size(); ++i) {
        const Goal& g = goals[i];
        double pct_d  = (g.targetAmount > 0.0)
                            ? std::min(g.currentAmount / g.targetAmount * 100.0, 100.0)
                            : 0.0;
        int pct       = static_cast<int>(pct_d);

        QWidget*     card       = new QWidget();
        QVBoxLayout* cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(16, 12, 16, 12);
        cardLayout->setSpacing(6);
        card->setStyleSheet(R"(
            background: #1e293b;
            border: 1px solid #334155;
            border-radius: 10px;
        )");

        // Nagłówek karty
        QHBoxLayout* headerRow = new QHBoxLayout();
        QLabel* nameLabel = new QLabel(
            QString("🎯  %1").arg(QString::fromStdString(g.name)));
        nameLabel->setStyleSheet(
            "color: #e2e8f0; font-size: 14px; font-weight: bold;");

        QLabel* pctLabel = new QLabel(QString("%1%").arg(pct));
        pctLabel->setStyleSheet(
            (pct >= 100) ? "color: #22c55e; font-size: 14px; font-weight: bold;"
                         : "color: #38bdf8; font-size: 14px; font-weight: bold;");

        headerRow->addWidget(nameLabel);
        headerRow->addStretch();
        headerRow->addWidget(pctLabel);
        cardLayout->addLayout(headerRow);

        // Kwota
        QLabel* amtLabel = new QLabel(
            QString("%1 / %2 PLN zebrano")
                .arg(g.currentAmount, 0, 'f', 2)
                .arg(g.targetAmount,  0, 'f', 2));
        amtLabel->setStyleSheet("color: #64748b; font-size: 12px;");
        cardLayout->addWidget(amtLabel);

        // Pasek postępu
        QProgressBar* bar = new QProgressBar();
        bar->setRange(0, 100);
        bar->setValue(pct);
        bar->setFixedHeight(16);
        bar->setTextVisible(false);

        QString barColor = (pct >= 100) ? "#22c55e" : "#0d6efd";
        bar->setStyleSheet(QString(R"(
            QProgressBar {
                background-color: #0f172a;
                border: none;
                border-radius: 6px;
            }
            QProgressBar::chunk {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 %1, stop:1 %2);
                border-radius: 6px;
            }
        )").arg(barColor).arg(pct >= 100 ? "#06d6a0" : "#38bdf8"));
        cardLayout->addWidget(bar);

        if (pct >= 100) {
            QLabel* doneLabel = new QLabel("✅  Cel osiągnięty!");
            doneLabel->setStyleSheet("color: #22c55e; font-size: 12px; font-weight: bold;");
            cardLayout->addWidget(doneLabel);
        }

        goalsLayout->addWidget(card);
    }

    goalsLayout->addStretch();
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Wyszukiwarka
// ═══════════════════════════════════════════════════════════════════════════════
void MainWindow::applySearch(const QString& text) {
    QString query = text.trimmed().toLower();
    double  visibleBalance = 0.0;

    const auto& transactions = wallet.getTransactions();

    for (int i = 0; i < historyTable->rowCount(); ++i) {
        bool matches = query.isEmpty();
        if (!matches) {
            for (int col = 0; col < historyTable->columnCount(); ++col) {
                QTableWidgetItem* item = historyTable->item(i, col);
                if (item && item->text().toLower().contains(query)) {
                    matches = true;
                    break;
                }
            }
        }
        historyTable->setRowHidden(i, !matches);

        if (matches && i < static_cast<int>(transactions.size())) {
            const auto& t = transactions[i];
            if (dynamic_cast<Income*>(t.get()))
                visibleBalance += t->getAmount();
            else
                visibleBalance -= t->getAmount();
        }
    }

    if (query.isEmpty()) {
        updateBalanceDisplay();
    } else {
        balanceLabel->setText(
            QString("Saldo (wyniki: %1 PLN)").arg(visibleBalance, 0, 'f', 2));
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Eksport CSV
// ═══════════════════════════════════════════════════════════════════════════════
void MainWindow::exportToCSV() {
    QString path = QFileDialog::getSaveFileName(
        this, "Eksportuj transakcje do CSV", "transakcje.csv",
        "Pliki CSV (*.csv);;Wszystkie pliki (*)");

    if (path.isEmpty()) return;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Błąd", "Nie można otworzyć pliku do zapisu.");
        return;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << "Data,Kategoria,Typ,Kwota (PLN),Konto,Cykliczna\n";

    for (const auto& t : wallet.getTransactions()) {
        QString type = dynamic_cast<Income*>(t.get()) ? "Przychod" : "Wydatek";
        QString rec  = t->isRecurring() ? "Tak" : "Nie";
        out << QString::fromStdString(t->getDate())        << ","
            << QString::fromStdString(t->getCategory())    << ","
            << type                                        << ","
            << QString::number(t->getAmount(), 'f', 2)     << ","
            << QString::fromStdString(t->getAccountName()) << ","
            << rec                                         << "\n";
    }

    file.close();
    QMessageBox::information(this, "Sukces",
        QString("✔  Wyeksportowano %1 transakcji do:\n%2")
            .arg(wallet.getTransactions().size()).arg(path));
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Cykliczne transakcje
// ═══════════════════════════════════════════════════════════════════════════════
void MainWindow::checkRecurringTransactions() {
    QDate today = QDate::currentDate();
    QStringList notifications;

    const auto& txList     = wallet.getTransactions();
    size_t      origCount  = txList.size();

    for (size_t i = 0; i < origCount; ++i) {
        const auto& t = txList[i];
        if (!t->isRecurring()) continue;

        QDate txDate = QDate::fromString(
            QString::fromStdString(t->getDate()), "yyyy-MM-dd");

        if (txDate.year() < today.year() ||
            (txDate.year() == today.year() && txDate.month() < today.month()))
        {
            QString cat     = QString::fromStdString(t->getCategory());
            double  amount  = t->getAmount();
            std::string acc = t->getAccountName();
            QString todayStr = today.toString("yyyy-MM-dd");

            if (dynamic_cast<Income*>(t.get())) {
                wallet.addTransaction(
                    std::make_unique<Income>(amount, cat.toStdString(),
                                             todayStr.toStdString(), true, acc));
            } else {
                wallet.addTransaction(
                    std::make_unique<Expense>(amount, cat.toStdString(),
                                              todayStr.toStdString(), true, acc));
            }

            notifications << QString("• %1: %2 PLN (%3)")
                .arg(cat).arg(amount, 0, 'f', 2)
                .arg(dynamic_cast<Income*>(t.get()) ? "Przychód" : "Wydatek");
        }
    }

    if (!notifications.isEmpty()) {
        QMessageBox::information(this, "Transakcje cykliczne",
            QString("Automatycznie dodano %1 cykliczną/e transakcję/e na dzisiaj:\n\n%2")
                .arg(notifications.size())
                .arg(notifications.join("\n")));
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Aktualizacja etykiety salda
// ═══════════════════════════════════════════════════════════════════════════════
void MainWindow::updateBalanceDisplay() {
    double balance = wallet.calculateBalance();
    balanceLabel->setText(
        QString("Saldo: %1 PLN").arg(balance, 0, 'f', 2));
    balanceLabel->setStyleSheet(
        balance >= 0
            ? "color: #38bdf8; padding: 4px 2px; font-weight: bold;"
            : "color: #ef4444; padding: 4px 2px; font-weight: bold;");
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Odświeżenie tabeli (6 kolumn)
// ═══════════════════════════════════════════════════════════════════════════════
void MainWindow::refreshTable() {
    historyTable->setRowCount(0);
    const auto& transactions = wallet.getTransactions();

    for (int i = 0; i < static_cast<int>(transactions.size()); ++i) {
        const auto& t = transactions[i];
        historyTable->insertRow(i);

        // Kol 0: Data
        historyTable->setItem(i, 0,
            new QTableWidgetItem(QString::fromStdString(t->getDate())));
        // Kol 1: Kategoria
        historyTable->setItem(i, 1,
            new QTableWidgetItem(QString::fromStdString(t->getCategory())));
        // Kol 2: Typ
        bool    isIncome = (dynamic_cast<Income*>(t.get()) != nullptr);
        QString typeStr  = isIncome ? "Przychód" : "Wydatek";
        auto*   typeItem = new QTableWidgetItem(typeStr);
        typeItem->setForeground(isIncome ? QColor("#22c55e") : QColor("#ef4444"));
        historyTable->setItem(i, 2, typeItem);
        // Kol 3: Kwota
        historyTable->setItem(i, 3,
            new QTableWidgetItem(QString::number(t->getAmount(), 'f', 2)));
        // Kol 4: Konto
        historyTable->setItem(i, 4,
            new QTableWidgetItem(QString::fromStdString(t->getAccountName())));
        // Kol 5: Cykliczna
        QString recStr  = t->isRecurring() ? "♻  Tak" : "—";
        auto*   recItem = new QTableWidgetItem(recStr);
        recItem->setForeground(t->isRecurring() ? QColor("#a3e635") : QColor("#475569"));
        historyTable->setItem(i, 5, recItem);
    }

    updateBalanceDisplay();

    if (!searchBar->text().isEmpty())
        applySearch(searchBar->text());
}