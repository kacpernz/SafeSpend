#include "MainWindow.hpp"
#include "AddTransactionDialog.hpp"
#include "LoginDialog.hpp"
#include "Income.hpp"
#include "Expense.hpp"
#include "DatabaseManager.hpp"
#include "DatabaseException.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QMessageBox>
#include <QHeaderView>
#include <QDate>
#include <QFileDialog>
#include <QTextStream>
#include <QFile>
#include <QMap>
#include <QStringList>

#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>

// ═══════════════════════════════════════════════════════════════════════════════
//  Stałe
// ═══════════════════════════════════════════════════════════════════════════════
static constexpr double MONTHLY_BUDGET_LIMIT = 5000.0;

// ═══════════════════════════════════════════════════════════════════════════════
//  Konstruktor
// ═══════════════════════════════════════════════════════════════════════════════
MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("SafeSpend");
    resize(820, 620);

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

    // ── Tabela historii ───────────────────────────────────────────────────────
    historyTable = new QTableWidget(0, 5, this);
    historyTable->setHorizontalHeaderLabels({"Data", "Kategoria", "Typ", "Kwota (PLN)", "Cykliczna"});
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
    budgetBar->setFormat("%p%  (%v/100)");
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

    auto makeBtn = [this](const QString& text, const QString& color) -> QPushButton* {
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
            QPushButton:hover   { background-color: %1; opacity: 0.85; }
            QPushButton:pressed { background-color: %1; opacity: 0.7;  }
        )").arg(color));
        return btn;
    };

    addTransactionButton = makeBtn("➕  Dodaj transakcję",        "#0d6efd");
    saveButton           = makeBtn("💾  Zapisz zaszyfrowane dane", "#0f7a4e");
    exportCsvButton      = makeBtn("📄  Eksportuj do CSV",         "#7c3aed");

    buttonRow->addWidget(addTransactionButton);
    buttonRow->addWidget(saveButton);
    buttonRow->addWidget(exportCsvButton);
    layout1->addLayout(buttonRow);

    tabWidget->addTab(tab1, "📋  Historia");

    // ╔═══════════════════════════════════════════════════════════════════════╗
    // ║  ZAKŁADKA 2 — Analiza (wykres kołowy)                                 ║
    // ╚═══════════════════════════════════════════════════════════════════════╝
    QWidget*     tab2    = new QWidget(this);
    QVBoxLayout* layout2 = new QVBoxLayout(tab2);
    layout2->setContentsMargins(12, 12, 12, 12);

    pieSeries = new QPieSeries(this);

    QChart* chart = new QChart();
    chart->addSeries(pieSeries);
    chart->setTitle("Wydatki według kategorii");
    chart->setTitleFont(QFont("Arial", 14, QFont::Bold));
    chart->setBackgroundBrush(QBrush(QColor("#111827")));
    chart->setTitleBrush(QBrush(QColor("#e2e8f0")));
    chart->legend()->setLabelColor(QColor("#94a3b8"));
    chart->legend()->setAlignment(Qt::AlignBottom);
    chart->setAnimationOptions(QChart::SeriesAnimations);

    chartView = new QChartView(chart, this);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setStyleSheet("background: #111827; border: none;");
    layout2->addWidget(chartView);

    tabWidget->addTab(tab2, "📊  Analiza");

    // ═════════════════════════════════════════════════════════════════════════
    //  Połączenia sygnałów
    // ═════════════════════════════════════════════════════════════════════════

    // Cel 1 – wyszukiwarka
    connect(searchBar, &QLineEdit::textChanged, this, &MainWindow::applySearch);

    // Dodawanie transakcji
    connect(addTransactionButton, &QPushButton::clicked, this, [this]() {
        AddTransactionDialog dialog(this);
        if (dialog.exec() == QDialog::Accepted) {
            double  amount    = dialog.getAmount();
            QString category  = dialog.getCategory();
            QString type      = dialog.getTransactionType();
            bool    recurring = dialog.getIsRecurring();
            QString today     = QDate::currentDate().toString("yyyy-MM-dd");

            if (type == "Przychod") {
                wallet.addTransaction(
                    std::make_unique<Income>(amount, category.toStdString(),
                                             today.toStdString(), recurring));
            } else {
                wallet.addTransaction(
                    std::make_unique<Expense>(amount, category.toStdString(),
                                              today.toStdString(), recurring));
            }
            refreshTable();
            updateBudgetBar();
            updateChart();
        }
    });

    // Zapis zaszyfrowany
    connect(saveButton, &QPushButton::clicked, this, [this]() {
        LoginDialog loginDialog(this);
        if (loginDialog.exec() == QDialog::Accepted) {
            DatabaseManager dbManager;
            try {
                dbManager.saveToBinaryFile(wallet.getTransactions(),
                                           "finanse_baza.bin",
                                           loginDialog.getPassword().toStdString());
                QMessageBox::information(this, "Sukces",
                    "✔  Dane zostały bezpiecznie zaszyfrowane i zapisane!");
            } catch (const DatabaseException& e) {
                QMessageBox::critical(this, "Błąd",
                    QString("Błąd zapisu: %1").arg(e.what()));
            }
        }
    });

    // Cel 2 – eksport CSV
    connect(exportCsvButton, &QPushButton::clicked, this, &MainWindow::exportToCSV);

    // Inicjalizacja wizualizacji
    updateBalanceDisplay();
    refreshTable();
    updateBudgetBar();
    updateChart();
}

// ═══════════════════════════════════════════════════════════════════════════════
//  loadWalletData — wywoływane z WelcomeWindow po zalogowaniu
// ═══════════════════════════════════════════════════════════════════════════════
void MainWindow::loadWalletData(std::vector<std::unique_ptr<ITransaction>> transactions) {
    wallet.clear();
    for (auto& t : transactions)
        wallet.addTransaction(std::move(t));

    // Cel 5 – sprawdź cykliczne transakcje
    checkRecurringTransactions();

    refreshTable();
    updateBalanceDisplay();
    updateBudgetBar();
    updateChart();
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Cel 1 – Wyszukiwarka
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

        // Sumuj saldo tylko widocznych wierszy
        if (matches && i < static_cast<int>(transactions.size())) {
            const auto& t = transactions[i];
            if (dynamic_cast<Income*>(t.get()))
                visibleBalance += t->getAmount();
            else
                visibleBalance -= t->getAmount();
        }
    }

    // Aktualizuj etykietę salda dla widocznych wierszy
    if (query.isEmpty()) {
        updateBalanceDisplay(); // pełne saldo
    } else {
        balanceLabel->setText(
            QString("Saldo (wyniki: %1 PLN)").arg(visibleBalance, 0, 'f', 2));
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Cel 2 – Eksport CSV
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

    // Nagłówek
    out << "Data,Kategoria,Typ,Kwota (PLN),Cykliczna\n";

    for (const auto& t : wallet.getTransactions()) {
        QString type = dynamic_cast<Income*>(t.get()) ? "Przychod" : "Wydatek";
        QString rec  = t->isRecurring() ? "Tak" : "Nie";
        out << QString::fromStdString(t->getDate())     << ","
            << QString::fromStdString(t->getCategory()) << ","
            << type                                      << ","
            << QString::number(t->getAmount(), 'f', 2)  << ","
            << rec                                       << "\n";
    }

    file.close();
    QMessageBox::information(this, "Sukces",
        QString("✔  Wyeksportowano %1 transakcji do:\n%2")
            .arg(wallet.getTransactions().size()).arg(path));
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Cel 3 – Aktualizacja wykresu kołowego
// ═══════════════════════════════════════════════════════════════════════════════
void MainWindow::updateChart() {
    pieSeries->clear();

    // Zbierz sumę wydatków per kategoria
    QMap<QString, double> categoryTotals;
    for (const auto& t : wallet.getTransactions()) {
        if (dynamic_cast<Expense*>(t.get())) {
            QString cat = QString::fromStdString(t->getCategory());
            categoryTotals[cat] += t->getAmount();
        }
    }

    if (categoryTotals.isEmpty()) {
        // Placeholder gdy brak danych
        auto* slice = pieSeries->append("Brak wydatków", 1.0);
        slice->setColor(QColor("#334155"));
        slice->setLabelColor(QColor("#94a3b8"));
        return;
    }

    // Paleta kolorów
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
        slice->setLabelVisible(true);
        ++colorIdx;
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Cel 4 – Budżet miesięczny
// ═══════════════════════════════════════════════════════════════════════════════
void MainWindow::updateBudgetBar() {
    QDate today = QDate::currentDate();
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

    // Zmiana koloru na czerwony gdy >= 90%
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
//  Cel 5 – Cykliczne transakcje
// ═══════════════════════════════════════════════════════════════════════════════
void MainWindow::checkRecurringTransactions() {
    QDate today = QDate::currentDate();
    QStringList notifications;

    // Iterujemy po KOPII listy rozmiarów, bo będziemy dodawać nowe elementy
    const auto& txList = wallet.getTransactions();
    size_t originalCount = txList.size();

    for (size_t i = 0; i < originalCount; ++i) {
        const auto& t = txList[i];
        if (!t->isRecurring()) continue;

        QDate txDate = QDate::fromString(
            QString::fromStdString(t->getDate()), "yyyy-MM-dd");

        // Jeśli transakcja jest starsza niż miesiąc (inny rok LUB inny miesiąc)
        if (txDate.year() < today.year() ||
            (txDate.year() == today.year() && txDate.month() < today.month()))
        {
            QString cat    = QString::fromStdString(t->getCategory());
            double  amount = t->getAmount();
            QString today_str = today.toString("yyyy-MM-dd");

            if (dynamic_cast<Income*>(t.get())) {
                wallet.addTransaction(
                    std::make_unique<Income>(amount, cat.toStdString(),
                                             today_str.toStdString(), true));
            } else {
                wallet.addTransaction(
                    std::make_unique<Expense>(amount, cat.toStdString(),
                                              today_str.toStdString(), true));
            }

            notifications << QString("• %1: %2 PLN (%3)")
                .arg(cat)
                .arg(amount, 0, 'f', 2)
                .arg(dynamic_cast<Income*>(t.get()) ? "Przychód" : "Wydatek");
        }
    }

    if (!notifications.isEmpty()) {
        QMessageBox::information(
            this,
            "Transakcje cykliczne",
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
    QString text = QString("Saldo: %1 PLN").arg(balance, 0, 'f', 2);
    balanceLabel->setText(text);
    balanceLabel->setStyleSheet(
        balance >= 0
            ? "color: #38bdf8; padding: 4px 2px; font-weight: bold;"
            : "color: #ef4444; padding: 4px 2px; font-weight: bold;");
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Odświeżenie tabeli
// ═══════════════════════════════════════════════════════════════════════════════
void MainWindow::refreshTable() {
    historyTable->setRowCount(0);
    const auto& transactions = wallet.getTransactions();

    for (int i = 0; i < static_cast<int>(transactions.size()); ++i) {
        const auto& t = transactions[i];
        historyTable->insertRow(i);

        historyTable->setItem(i, 0,
            new QTableWidgetItem(QString::fromStdString(t->getDate())));
        historyTable->setItem(i, 1,
            new QTableWidgetItem(QString::fromStdString(t->getCategory())));

        bool isIncome = (dynamic_cast<Income*>(t.get()) != nullptr);
        QString typeStr = isIncome ? "Przychód" : "Wydatek";
        auto* typeItem = new QTableWidgetItem(typeStr);
        typeItem->setForeground(isIncome ? QColor("#22c55e") : QColor("#ef4444"));
        historyTable->setItem(i, 2, typeItem);

        historyTable->setItem(i, 3,
            new QTableWidgetItem(QString::number(t->getAmount(), 'f', 2)));

        QString recStr = t->isRecurring() ? "♻  Tak" : "—";
        auto* recItem = new QTableWidgetItem(recStr);
        recItem->setForeground(t->isRecurring() ? QColor("#a3e635") : QColor("#475569"));
        historyTable->setItem(i, 4, recItem);
    }

    updateBalanceDisplay();

    // Ponownie zastosuj aktywny filtr
    if (!searchBar->text().isEmpty())
        applySearch(searchBar->text());
}