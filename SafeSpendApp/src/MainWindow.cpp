#include "MainWindow.hpp"
#include "AddTransactionDialog.hpp"
#include "DatabaseException.hpp"
#include "DatabaseManager.hpp"
#include "Expense.hpp"
#include "Income.hpp"
#include "Transfer.hpp"
#include "ThemeManager.hpp"

#include <QDate>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFileDialog>
#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QMap>
#include <QMessageBox>
#include <QScrollArea>
#include <QShortcut>
#include <QSplitter>
#include <QStatusBar>
#include <QStringList>
#include <QTextStream>
#include <QVBoxLayout>
#include <QWidget>
#include <algorithm>
#include <cmath>

#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QtCharts/QValueAxis>

// (stała MONTHLY_BUDGET_LIMIT usunięta — wartość przechowywana w wallet.getMonthlyBudgetLimit())




// ═══════════════════════════════════════════════════════════════════════════════
//  Pomocnicze
// ═══════════════════════════════════════════════════════════════════════════════
QPushButton *MainWindow::makeButton(const QString &text, const QString &color) {
  QPushButton *btn = new QPushButton(text, this);
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
    )")
                         .arg(color));
  return btn;
}

void MainWindow::refreshAll() {
  refreshTable();
  updateBalanceDisplay();
  updateBudgetBar();
  updateChart();
  updateBarChart();
  updateEnvelopeBudgets();
  refreshGoalsTab();
  refreshAccountsTab();
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Auto-zapis (cichy, bez dialogu) — Cel 1
// ═══════════════════════════════════════════════════════════════════════════════
void MainWindow::autoSave() {
  if (m_savePassword.empty())
    return; // hasło jeszcze niedostępne — pomiń
  DatabaseManager dbManager;
  try {
    dbManager.saveWallet(wallet, "finanse_baza.bin", m_savePassword);
  } catch (const DatabaseException &e) {
    // Zapisujemy cicho; błąd wyświetlamy tylko jeśli użytkownik nacisnął Ctrl+S
    Q_UNUSED(e)
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Konstruktor
// ═══════════════════════════════════════════════════════════════════════════════
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  setWindowTitle("SafeSpend");
  resize(1100, 720);

  // ── Centralny widget + layout ────────────────────────────────────────────
  QWidget *centralWidget = new QWidget(this);
  QVBoxLayout *rootLayout = new QVBoxLayout(centralWidget);
  rootLayout->setContentsMargins(0, 0, 0, 0);
  rootLayout->setSpacing(0);

  // ── Toolbar z przyciskiem motywu (u góry) ────────────────────────────────
  QWidget *toolBar = new QWidget(this);
  toolBar->setFixedHeight(44);
  toolBar->setStyleSheet(
      "background: #0f172a; border-bottom: 1px solid #1e293b;");
  QHBoxLayout *toolBarLayout = new QHBoxLayout(toolBar);
  toolBarLayout->setContentsMargins(16, 0, 16, 0);
  toolBarLayout->setSpacing(8);

  QLabel *appLogoLabel = new QLabel("💰  SafeSpend", toolBar);
  appLogoLabel->setStyleSheet(
      "color: #38bdf8; font-size: 14px; font-weight: bold;");
  toolBarLayout->addWidget(appLogoLabel);
  toolBarLayout->addStretch();

  themeToggleBtn =
      new QPushButton(ThemeManager::instance()->toggleButtonLabel(), toolBar);
  themeToggleBtn->setFixedHeight(28);
  themeToggleBtn->setCursor(Qt::PointingHandCursor);
  themeToggleBtn->setStyleSheet(R"(
        QPushButton {
            background: rgba(255,255,255,0.07);
            color: #94a3b8;
            border: 1px solid rgba(255,255,255,0.15);
            border-radius: 8px;
            font-size: 12px;
            font-weight: bold;
            padding: 0 14px;
        }
        QPushButton:hover { background: rgba(255,255,255,0.13); color: white; }
        QPushButton:pressed { background: rgba(255,255,255,0.20); }
    )");
  toolBarLayout->addWidget(themeToggleBtn);
  rootLayout->addWidget(toolBar);

  // ── QTabWidget ───────────────────────────────────────────────────────────
  tabWidget = new QTabWidget(this);
  // Styl zakładek zarządzany przez ThemeManager (globalny QSS)
  rootLayout->addWidget(tabWidget);
  setCentralWidget(centralWidget);

  // ╔═══════════════════════════════════════════════════════════════════════╗
  // ║  ZAKŁADKA 1 — Historia transakcji                                     ║
  // ╚═══════════════════════════════════════════════════════════════════════╝
  QWidget *tab1 = new QWidget(this);
  QVBoxLayout *layout1 = new QVBoxLayout(tab1);
  layout1->setContentsMargins(16, 16, 16, 16);
  layout1->setSpacing(10);

  // ── Pasek wyszukiwania ───────────────────────────────────────────────────
  searchBar = new QLineEdit(this);
  searchBar->setPlaceholderText(
      "🔍  Szukaj transakcji (data, kategoria, kwota)...");
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
  // Etykieta ogólna; aktualny limit jest widoczny dynamicznie w formacie paska
  QLabel *budgetLabel = new QLabel("Budżet miesięczny:", this);
  budgetLabel->setStyleSheet("color: #94a3b8; font-size: 12px;");
  layout1->addWidget(budgetLabel);


  budgetBar = new QProgressBar(this);
  budgetBar->setRange(0, 100);
  budgetBar->setValue(0);
  budgetBar->setFixedHeight(22);
  budgetBar->setTextVisible(true);
  budgetBar->setFormat("0 / 5000 PLN  (0%)");  // zostanie nadpisane przez updateBudgetBar()
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
  QHBoxLayout *buttonRow = new QHBoxLayout();
  buttonRow->setSpacing(10);

  addTransactionButton = makeButton("➕  Dodaj transakcję", "#0d6efd");
  exportCsvButton = makeButton("📄  Eksportuj do CSV", "#7c3aed");

  buttonRow->addWidget(addTransactionButton);
  buttonRow->addWidget(exportCsvButton);
  layout1->addLayout(buttonRow);

  tabWidget->addTab(tab1, "📋  Historia");

  // ╔═══════════════════════════════════════════════════════════════════════╗
  // ║  ZAKŁADKA 2 — Analiza (wykres kołowy + słupkowy + koperty)            ║
  // ╚═══════════════════════════════════════════════════════════════════════╝
  QWidget *tab2 = new QWidget(this);
  QVBoxLayout *layout2 = new QVBoxLayout(tab2);
  layout2->setContentsMargins(12, 12, 12, 12);
  layout2->setSpacing(10);

  // ── Nagłówek + przycisk limitów ──────────────────────────────────────────
  QHBoxLayout *analysisHeader = new QHBoxLayout();
  QLabel *analysisTitle = new QLabel("📊  Analiza finansów", tab2);
  analysisTitle->setStyleSheet(
      "color: #e2e8f0; font-size: 16px; font-weight: bold; padding: 4px;");

  QPushButton *limitsBtn = new QPushButton("⚙  Zarządzaj limitami", tab2);
  limitsBtn->setFixedHeight(36);
  limitsBtn->setCursor(Qt::PointingHandCursor);
  limitsBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #334155;
            color: #e2e8f0;
            border: 1px solid #475569;
            border-radius: 7px;
            font-size: 12px;
            font-weight: bold;
            padding: 0 14px;
        }
        QPushButton:hover   { background-color: #475569; color: white; }
        QPushButton:pressed { background-color: #1e293b; }
    )");

  analysisHeader->addWidget(analysisTitle);
  analysisHeader->addStretch();
  analysisHeader->addWidget(limitsBtn);
  layout2->addLayout(analysisHeader);

  // ── Wykres kołowy | wykres słupkowy ──────────────────────────────────────
  QSplitter *chartSplitter = new QSplitter(Qt::Horizontal, tab2);
  chartSplitter->setStyleSheet(
      "QSplitter::handle { background: #334155; width: 2px; }");

  pieSeries = new QPieSeries(this);
  pieChart = new QChart();
  pieChart->addSeries(pieSeries);
  pieChart->setTitle("Wydatki wg kategorii");
  pieChart->setTitleFont(QFont("Arial", 12, QFont::Bold));
  pieChart->setTheme(QChart::ChartThemeDark);
  pieChart->setBackgroundBrush(QBrush(QColor("#111827")));
  pieChart->legend()->setLabelColor(QColor("#94a3b8"));
  pieChart->legend()->setAlignment(Qt::AlignBottom);
  pieChart->setAnimationOptions(QChart::SeriesAnimations);

  chartView = new QChartView(pieChart, this);
  chartView->setRenderHint(QPainter::Antialiasing);
  chartView->setStyleSheet("background: #111827; border: none;");
  chartView->setMinimumHeight(260);

  barChart = new QChart();
  barChart->setTitle("Przychody vs Wydatki (miesiące)");
  barChart->setTitleFont(QFont("Arial", 12, QFont::Bold));
  barChart->setTheme(QChart::ChartThemeDark);
  barChart->setBackgroundBrush(QBrush(QColor("#111827")));
  barChart->legend()->setLabelColor(QColor("#94a3b8"));
  barChart->legend()->setAlignment(Qt::AlignBottom);
  barChart->setAnimationOptions(QChart::SeriesAnimations);

  barChartView = new QChartView(barChart, this);
  barChartView->setRenderHint(QPainter::Antialiasing);
  barChartView->setStyleSheet("background: #111827; border: none;");
  barChartView->setMinimumHeight(260);

  chartSplitter->addWidget(chartView);
  chartSplitter->addWidget(barChartView);
  chartSplitter->setSizes({500, 500});
  layout2->addWidget(chartSplitter, 2);

  // ── Separator ────────────────────────────────────────────────────────────
  QFrame *separator = new QFrame(tab2);
  separator->setFrameShape(QFrame::HLine);
  separator->setStyleSheet("color: #334155; margin: 2px 0;");
  layout2->addWidget(separator);

  // ── Budżetowanie kopertowe ────────────────────────────────────────────────
  QLabel *envelopeTitle =
      new QLabel("🗂  Budżetowanie kopertowe (bieżący miesiąc)", tab2);
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
  QWidget *tab3 = new QWidget(this);
  QVBoxLayout *layout3 = new QVBoxLayout(tab3);
  layout3->setContentsMargins(16, 16, 16, 16);
  layout3->setSpacing(12);

  QLabel *goalsTitle = new QLabel("🎯  Cele Oszczędnościowe", tab3);
  goalsTitle->setStyleSheet(
      "color: #e2e8f0; font-size: 16px; font-weight: bold; padding: 4px;");
  layout3->addWidget(goalsTitle);

  QHBoxLayout *goalsBtnRow = new QHBoxLayout();
  goalsBtnRow->setSpacing(10);
  QPushButton *newGoalBtn = makeButton("🎯  Nowy cel", "#0f7a4e");
  QPushButton *fundGoalBtn = makeButton("💰  Zasil cel", "#0d6efd");
  newGoalBtn->setParent(tab3);
  fundGoalBtn->setParent(tab3);
  goalsBtnRow->addWidget(newGoalBtn);
  goalsBtnRow->addWidget(fundGoalBtn);
  goalsBtnRow->addStretch();
  layout3->addLayout(goalsBtnRow);

  goalsScrollArea = new QScrollArea(tab3);
  goalsScrollArea->setWidgetResizable(true);
  goalsScrollArea->setStyleSheet(R"(
        QScrollArea { background: #111827; border: none; }
        QScrollBar:vertical {
            background: #1e293b; width: 8px; border-radius: 4px;
        }
        QScrollBar::handle:vertical { background: #334155; border-radius: 4px; }
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

  // ╔═══════════════════════════════════════════════════════════════════════╗
  // ║  ZAKŁADKA 4 — Konta (subkonta)                                        ║
  // ╚═══════════════════════════════════════════════════════════════════════╝
  QWidget *tab4 = new QWidget(this);
  QVBoxLayout *layout4 = new QVBoxLayout(tab4);
  layout4->setContentsMargins(16, 16, 16, 16);
  layout4->setSpacing(12);

  QLabel *accountsTitle = new QLabel("🏦  Stan subkont", tab4);
  accountsTitle->setStyleSheet(
      "color: #e2e8f0; font-size: 16px; font-weight: bold; padding: 4px;");
  layout4->addWidget(accountsTitle);

  QLabel *accountsHint = new QLabel(
      "Poniżej widoczny jest aktualny stan środków na każdym koncie.", tab4);
  accountsHint->setStyleSheet(
      "color: #64748b; font-size: 12px; padding: 0 4px 8px 4px;");
  layout4->addWidget(accountsHint);

  QScrollArea *accountsScroll = new QScrollArea(tab4);
  accountsScroll->setWidgetResizable(true);
  accountsScroll->setStyleSheet(R"(
        QScrollArea { background: #111827; border: none; }
        QScrollBar:vertical {
            background: #1e293b; width: 8px; border-radius: 4px;
        }
        QScrollBar::handle:vertical { background: #334155; border-radius: 4px; }
    )");

  accountsWidget = new QWidget();
  accountsWidget->setStyleSheet("background: #111827;");
  accountsLayout = new QVBoxLayout(accountsWidget);
  accountsLayout->setContentsMargins(4, 4, 4, 4);
  accountsLayout->setSpacing(10);
  accountsLayout->addStretch();

  accountsScroll->setWidget(accountsWidget);
  layout4->addWidget(accountsScroll);

  tabWidget->addTab(tab4, "🏦  Konta");

  // ═════════════════════════════════════════════════════════════════════════
  connect(searchBar, &QLineEdit::textChanged, this, &MainWindow::applySearch);

  // Dodawanie transakcji (z obsługą Transfer)
  connect(addTransactionButton, &QPushButton::clicked, this, [this]() {
    AddTransactionDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
      double amount = dialog.getAmount();
      QString type  = dialog.getTransactionType();
      QString today = QDate::currentDate().toString("yyyy-MM-dd");

      if (type == "Transfer") {
        QString from = dialog.getFromAccount();
        QString to   = dialog.getToAccount();
        if (from == to) {
          QMessageBox::warning(this, "Transfer",
            "Konta źródłowe i docelowe muszą być różne!");
          return;
        }
        wallet.addTransaction(std::make_unique<Transfer>(
            amount, from.toStdString(), to.toStdString(), today.toStdString()));
      } else if (type == "Przychod") {
        wallet.addTransaction(std::make_unique<Income>(
            amount, dialog.getCategory().toStdString(),
            today.toStdString(), dialog.getIsRecurring(),
            dialog.getAccountName().toStdString()));
      } else {
        wallet.addTransaction(std::make_unique<Expense>(
            amount, dialog.getCategory().toStdString(),
            today.toStdString(), dialog.getIsRecurring(),
            dialog.getAccountName().toStdString()));
      }
      refreshAll();
      autoSave();
    }
  });

  // Eksport CSV
  connect(exportCsvButton, &QPushButton::clicked, this, &MainWindow::exportToCSV);

  // Zarządzaj limitami
  connect(limitsBtn, &QPushButton::clicked, this, &MainWindow::openLimitsDialog);

  // Nowy cel
  connect(newGoalBtn, &QPushButton::clicked, this, [this]() {
    bool ok;
    QString name =
        QInputDialog::getText(this, "Nowy cel oszczędnościowy",
                              "Nazwa celu:", QLineEdit::Normal, "", &ok);
    if (!ok || name.trimmed().isEmpty())
      return;

    double target = QInputDialog::getDouble(this, "Kwota docelowa",
                                            "Docelowa kwota (PLN):", 0.0, 0.0,
                                            1000000.0, 2, &ok);
    if (!ok || target <= 0.0)
      return;

    Goal g;
    g.name = name.trimmed().toStdString();
    g.targetAmount = target;
    g.currentAmount = 0.0;
    wallet.addGoal(g);
    refreshGoalsTab();
    autoSave();
  });

  // Zasil cel
  connect(fundGoalBtn, &QPushButton::clicked, this, [this]() {
    const auto &goals = wallet.getGoals();
    if (goals.empty()) {
      QMessageBox::information(this, "Brak celów",
                               "Najpierw dodaj cel oszczędnościowy.");
      return;
    }

    QStringList items;
    for (const auto &g : goals)
      items << QString::fromStdString(g.name) +
                   QString(" (%1/%2 PLN)")
                       .arg(g.currentAmount, 0, 'f', 2)
                       .arg(g.targetAmount, 0, 'f', 2);

    bool ok;
    QString chosen = QInputDialog::getItem(
        this, "Zasil cel", "Wybierz cel:", items, 0, false, &ok);
    if (!ok)
      return;

    int idx = items.indexOf(chosen);
    if (idx < 0)
      return;

    double amount =
        QInputDialog::getDouble(this, "Kwota", "Kwota do wpłacenia (PLN):", 0.0,
                                0.01, 1000000.0, 2, &ok);
    if (!ok || amount <= 0.0)
      return;

    double balance = wallet.calculateBalance();
    if (amount > balance) {
      QMessageBox::warning(
          this, "Brak środków",
          QString("Saldo (%1 PLN) jest za niskie!").arg(balance, 0, 'f', 2));
      return;
    }

    QString goalExpCat = "Cel: " + QString::fromStdString(goals[idx].name);
    QString today = QDate::currentDate().toString("yyyy-MM-dd");
    wallet.addTransaction(
        std::make_unique<Expense>(amount, goalExpCat.toStdString(),
                                  today.toStdString(), false, "Oszczędności"));
    wallet.fundGoal(static_cast<size_t>(idx), amount);

    refreshAll();
    autoSave();
  });

  // ── Skrót klawiszowy Ctrl+S — ręczny zapis ──────────────────────────────
  QShortcut *saveShortcut = new QShortcut(QKeySequence::Save, this);
  connect(saveShortcut, &QShortcut::activated, this, [this]() {
    if (m_savePassword.empty()) return;
    DatabaseManager dbManager;
    try {
      dbManager.saveWallet(wallet, "finanse_baza.bin", m_savePassword);
      statusBar()->showMessage("✔  Dane zapisane pomyślnie (Ctrl+S)", 3000);
    } catch (const DatabaseException &e) {
      QMessageBox::critical(this, "Błąd zapisu",
                            QString("Błąd: %1").arg(e.what()));
    }
  });

  // Inicjalizacja wizualizacji
  refreshAll();

  // ── Połączenia: ThemeManager ─────────────────────────────────────────────
  connect(themeToggleBtn, &QPushButton::clicked, ThemeManager::instance(),
          &ThemeManager::toggleTheme);

  connect(ThemeManager::instance(), &ThemeManager::themeChanged, this,
          &MainWindow::onThemeChanged);
}

// ═══════════════════════════════════════════════════════════════════════════════
//  loadWalletData — wywoływane z WelcomeWindow po zalogowaniu
// ═══════════════════════════════════════════════════════════════════════════════
void MainWindow::loadWalletData(Wallet &&loadedWallet, const std::string &password) {
  m_savePassword = password;  // zapamiętaj hasło do auto-zapisu
  wallet = std::move(loadedWallet);
  checkRecurringTransactions();
  refreshAll();
  autoSave(); // zapisz po wczytaniu (na wypadek dodanych transakcji cyklicznych)
}


// ═══════════════════════════════════════════════════════════════════════════════
//  Cel 4 — Reaktywna zmiana motywu wykresów i UI
// ═══════════════════════════════════════════════════════════════════════════════
void MainWindow::onThemeChanged(bool isDark) {
  // 1. Aktualizuj etykietę przycisku
  themeToggleBtn->setText(ThemeManager::instance()->toggleButtonLabel());

  // 2. Zaktualizuj motyw QChart (QSS nie sięga do QtCharts — konieczne ręcznie)
  QChart::ChartTheme chartTheme =
      isDark ? QChart::ChartThemeDark : QChart::ChartThemeLight;

  pieChart->setTheme(chartTheme);
  barChart->setTheme(chartTheme);

  // 3. Zaktualizuj tła ChartView
  QString chartBg = isDark ? "#111827" : "#f1f5f9";
  chartView->setStyleSheet(
      QString("background: %1; border: none;").arg(chartBg));
  barChartView->setStyleSheet(
      QString("background: %1; border: none;").arg(chartBg));

  // 4. Zaktualizuj styl toolbara (ma własne QSS poza globalnym arkuszem)
  if (QWidget *tb = qobject_cast<QWidget *>(themeToggleBtn->parent())) {
    QString tbBg = isDark ? "#0f172a" : "#e2e8f0";
    QString tbBorder = isDark ? "#1e293b" : "#cbd5e1";
    tb->setStyleSheet(QString("background: %1; border-bottom: 1px solid %2;")
                          .arg(tbBg)
                          .arg(tbBorder));
  }

  // 5. Zaktualizuj styl przycisku motywu
  if (isDark) {
    themeToggleBtn->setStyleSheet(R"(
            QPushButton {
                background: rgba(255,255,255,0.07); color: #94a3b8;
                border: 1px solid rgba(255,255,255,0.15); border-radius: 8px;
                font-size: 12px; font-weight: bold; padding: 0 14px;
            }
            QPushButton:hover  { background: rgba(255,255,255,0.13); color: white; }
            QPushButton:pressed{ background: rgba(255,255,255,0.20); }
        )");
  } else {
    themeToggleBtn->setStyleSheet(R"(
            QPushButton {
                background: rgba(0,0,0,0.06); color: #475569;
                border: 1px solid rgba(0,0,0,0.15); border-radius: 8px;
                font-size: 12px; font-weight: bold; padding: 0 14px;
            }
            QPushButton:hover  { background: rgba(0,0,0,0.12); color: #1e293b; }
            QPushButton:pressed{ background: rgba(0,0,0,0.20); }
        )");
  }

  // 6. Przerysuj dane wykresów z nowym motywem
  updateChart();
  updateBarChart();
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Dialog zarządzania limitami kopertowymi
// ═══════════════════════════════════════════════════════════════════════════════
void MainWindow::openLimitsDialog() {
  QDialog *dlg = new QDialog(this);
  dlg->setWindowTitle("Zarządzaj limitami kategorii");
  dlg->setMinimumWidth(440);
  dlg->setAttribute(Qt::WA_DeleteOnClose);
  // Styl dialogu zgodny z aktualnym motywem (ciemny / jasny)
  bool isDarkTheme = ThemeManager::instance()->isDark();
  QString dlgBg         = isDarkTheme ? "#111827" : "#f8fafc";
  QString dlgInput      = isDarkTheme ? "#1e293b" : "#ffffff";
  QString dlgInputBorder= isDarkTheme ? "#334155" : "#cbd5e1";
  QString dlgTextMain   = isDarkTheme ? "#e2e8f0" : "#1e293b";
  QString dlgTextMuted  = isDarkTheme ? "#94a3b8" : "#475569";
  QString dlgAccent     = isDarkTheme ? "#3d8bfd" : "#2563eb";
  QString dlgCancelBg   = isDarkTheme ? "#334155" : "#e2e8f0";
  QString dlgCancelHov  = isDarkTheme ? "#475569" : "#cbd5e1";

  dlg->setStyleSheet(QString(R"(
        QDialog {
            background-color: %1;
        }
        QLabel {
            color: %2;
            font-size: 13px;
        }
        QDoubleSpinBox {
            background-color: %3;
            color: %4;
            border: 1px solid %5;
            border-radius: 6px;
            padding: 5px 10px;
            font-size: 13px;
        }
        QDoubleSpinBox:focus { border: 1px solid %6; }
        QLineEdit {
            background-color: %3;
            color: %4;
            border: 1px solid %5;
            border-radius: 6px;
            padding: 5px 10px;
            font-size: 13px;
        }
        QLineEdit:focus { border: 1px solid %6; }
        QDialogButtonBox QPushButton {
            background-color: %6;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 7px 20px;
            font-size: 13px;
            font-weight: bold;
        }
        QDialogButtonBox QPushButton:hover { opacity: 0.85; }
        QDialogButtonBox QPushButton[text="Cancel"] {
            background-color: %7;
            color: %2;
        }
        QDialogButtonBox QPushButton[text="Cancel"]:hover {
            background-color: %8;
        }
    )")
    .arg(dlgBg)         // %1
    .arg(dlgTextMain)   // %2
    .arg(dlgInput)      // %3
    .arg(dlgTextMain)   // %4
    .arg(dlgInputBorder)// %5
    .arg(dlgAccent)     // %6
    .arg(dlgCancelBg)   // %7
    .arg(dlgCancelHov)  // %8
  );


  QVBoxLayout *dlgLayout = new QVBoxLayout(dlg);
  dlgLayout->setContentsMargins(20, 20, 20, 16);
  dlgLayout->setSpacing(12);

  QLabel *infoLabel = new QLabel(
      "Ustaw miesięczne limity wydatków dla kategorii.\n"
      "Kategoria z limitem 0 nie będzie wyświetlana w paskach kopertowych.",
      dlg);
  infoLabel->setWordWrap(true);
  infoLabel->setStyleSheet("color: #64748b; font-size: 12px;");
  dlgLayout->addWidget(infoLabel);

  // ── Sekcja: Główny budżet miesięczny ───────────────────────────────────────
  QFrame *budgetSectionLine = new QFrame(dlg);
  budgetSectionLine->setFrameShape(QFrame::HLine);
  budgetSectionLine->setStyleSheet("color: #334155;");
  dlgLayout->addWidget(budgetSectionLine);

  QLabel *budgetSectionTitle = new QLabel("💰  Główny budżet miesięczny:", dlg);
  budgetSectionTitle->setStyleSheet(
      "color: #38bdf8; font-weight: bold; font-size: 14px; padding: 2px 0;");
  dlgLayout->addWidget(budgetSectionTitle);

  QLabel *budgetSectionHint = new QLabel(
      "Całkowity limit wydatków w bieżącym miesiącu (widoczny na pasku budżetu).",
      dlg);
  budgetSectionHint->setWordWrap(true);
  budgetSectionHint->setStyleSheet("color: #64748b; font-size: 11px; margin-bottom: 2px;");
  dlgLayout->addWidget(budgetSectionHint);

  QFormLayout *budgetForm = new QFormLayout();
  budgetForm->setSpacing(6);
  budgetForm->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);

  QDoubleSpinBox *globalBudgetSpin = new QDoubleSpinBox(dlg);
  globalBudgetSpin->setRange(1.0, 10000000.0);
  globalBudgetSpin->setDecimals(2);
  globalBudgetSpin->setSuffix(" PLN");
  globalBudgetSpin->setSingleStep(100.0);
  globalBudgetSpin->setValue(wallet.getMonthlyBudgetLimit());
  globalBudgetSpin->setObjectName("globalBudgetSpin");

  QLabel *globalBudgetLabel = new QLabel("Limit globalny:", dlg);
  globalBudgetLabel->setStyleSheet("color: #e2e8f0; font-size: 13px; font-weight: bold;");
  budgetForm->addRow(globalBudgetLabel, globalBudgetSpin);
  dlgLayout->addLayout(budgetForm);

  // ── Sekcja istniejących limitów kategorii ─────────────────────────────────
  QFrame *line1 = new QFrame(dlg);
  line1->setFrameShape(QFrame::HLine);
  line1->setStyleSheet("color: #334155;");
  dlgLayout->addWidget(line1);

  QLabel *existingTitle = new QLabel("Limity kategorii:", dlg);
  existingTitle->setStyleSheet(
      "color: #e2e8f0; font-weight: bold; font-size: 13px;");
  dlgLayout->addWidget(existingTitle);


  // Zbierz aktualne limity + spinboxy do mapy
  const auto &currentLimits = wallet.getCategoryLimits();

  // Zbierz też kategorie z transakcji (tylko Expense), których nie ma w
  // limitach
  QStringList allExpenseCategories;
  for (const auto &t : wallet.getTransactions()) {
    if (dynamic_cast<Expense *>(t.get())) {
      QString cat = QString::fromStdString(t->getCategory());
      if (!allExpenseCategories.contains(cat))
        allExpenseCategories << cat;
    }
  }
  // Dodaj też te, które już mają limity
  for (const auto &kv : currentLimits) {
    QString cat = QString::fromStdString(kv.first);
    if (!allExpenseCategories.contains(cat))
      allExpenseCategories << cat;
  }
  allExpenseCategories.sort();

  // Mapa: nazwa kategorii → QDoubleSpinBox
  QMap<QString, QDoubleSpinBox *> spinBoxes;

  QFormLayout *formLayout = new QFormLayout();
  formLayout->setSpacing(8);
  formLayout->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);

  for (const QString &cat : allExpenseCategories) {
    QDoubleSpinBox *spin = new QDoubleSpinBox(dlg);
    spin->setRange(0.0, 1000000.0);
    spin->setDecimals(2);
    spin->setSuffix(" PLN");
    spin->setSingleStep(50.0);

    auto it = currentLimits.find(cat.toStdString());
    spin->setValue(it != currentLimits.end() ? it->second : 0.0);

    spinBoxes[cat] = spin;
    QLabel *catLabel = new QLabel(cat + ":", dlg);
    catLabel->setStyleSheet("color: #e2e8f0; font-size: 13px;");
    formLayout->addRow(catLabel, spin);
  }

  dlgLayout->addLayout(formLayout);

  // ── Sekcja dodawania nowej kategorii ─────────────────────────────────────
  QFrame *line2 = new QFrame(dlg);
  line2->setFrameShape(QFrame::HLine);
  line2->setStyleSheet("color: #334155;");
  dlgLayout->addWidget(line2);

  QLabel *addTitle = new QLabel("Dodaj nową kategorię:", dlg);
  addTitle->setStyleSheet(
      "color: #e2e8f0; font-weight: bold; font-size: 13px;");
  dlgLayout->addWidget(addTitle);

  QHBoxLayout *addRow = new QHBoxLayout();
  QLineEdit *newCatEdit = new QLineEdit(dlg);
  newCatEdit->setPlaceholderText("Nazwa kategorii...");
  QDoubleSpinBox *newCatSpin = new QDoubleSpinBox(dlg);
  newCatSpin->setRange(0.0, 1000000.0);
  newCatSpin->setDecimals(2);
  newCatSpin->setSuffix(" PLN");
  newCatSpin->setSingleStep(50.0);
  newCatSpin->setValue(500.0);

  QPushButton *addCatBtn = new QPushButton("➕ Dodaj", dlg);
  addCatBtn->setFixedHeight(36);
  addCatBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #0f7a4e;
            color: white;
            border: none;
            border-radius: 6px;
            font-size: 12px;
            font-weight: bold;
            padding: 0 14px;
        }
        QPushButton:hover { background-color: #15a068; }
    )");

  addRow->addWidget(newCatEdit, 2);
  addRow->addWidget(newCatSpin, 1);
  addRow->addWidget(addCatBtn);
  dlgLayout->addLayout(addRow);

  // Dodaj kategorię do formularza
  connect(addCatBtn, &QPushButton::clicked, dlg,
          [&spinBoxes, newCatEdit, newCatSpin, formLayout, dlg]() {
            QString newCat = newCatEdit->text().trimmed();
            if (newCat.isEmpty())
              return;
            if (spinBoxes.contains(newCat)) {
              spinBoxes[newCat]->setValue(newCatSpin->value());
              newCatEdit->clear();
              return;
            }
            QDoubleSpinBox *spin = new QDoubleSpinBox(dlg);
            spin->setRange(0.0, 1000000.0);
            spin->setDecimals(2);
            spin->setSuffix(" PLN");
            spin->setSingleStep(50.0);
            spin->setValue(newCatSpin->value());
            spin->setStyleSheet(R"(
            QDoubleSpinBox {
                background-color: #1e293b;
                color: #e2e8f0;
                border: 1px solid #334155;
                border-radius: 6px;
                padding: 5px 10px;
                font-size: 13px;
            }
            QDoubleSpinBox:focus { border: 1px solid #3d8bfd; }
        )");
            spinBoxes[newCat] = spin;
            QLabel *catLabel = new QLabel(newCat + ":", dlg);
            catLabel->setStyleSheet("color: #e2e8f0; font-size: 13px;");
            formLayout->addRow(catLabel, spin);
            newCatEdit->clear();
          });

  // ── Przyciski OK / Anuluj ─────────────────────────────────────────────────
  QDialogButtonBox *btnBox = new QDialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel, dlg);
  dlgLayout->addWidget(btnBox);

  connect(btnBox, &QDialogButtonBox::accepted, dlg, [this, dlg, &spinBoxes, globalBudgetSpin]() {
    // Zapisz globalny limit budżetu
    wallet.setMonthlyBudgetLimit(globalBudgetSpin->value());

    // Zapisz limity kategorii
    for (auto it = spinBoxes.constBegin(); it != spinBoxes.constEnd(); ++it) {
      wallet.setCategoryLimit(it.key().toStdString(), it.value()->value());
    }
    refreshAll();
    autoSave();
    dlg->accept();
  });
  connect(btnBox, &QDialogButtonBox::rejected, dlg, &QDialog::reject);

  dlg->exec();
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Aktualizacja paska budżetu miesięcznego
// ═══════════════════════════════════════════════════════════════════════════════
void MainWindow::updateBudgetBar() {
  QDate today = QDate::currentDate();
  int thisYear = today.year();
  int thisMonth = today.month();

  double monthlyExpenses = 0.0;
  for (const auto &t : wallet.getTransactions()) {
    if (dynamic_cast<Expense *>(t.get())) {
      QDate txDate =
          QDate::fromString(QString::fromStdString(t->getDate()), "yyyy-MM-dd");
      if (txDate.year() == thisYear && txDate.month() == thisMonth)
        monthlyExpenses += t->getAmount();
    }
  }

  double budgetLimit = wallet.getMonthlyBudgetLimit();
  double ratio = monthlyExpenses / budgetLimit;
  int percent = static_cast<int>(std::min(ratio * 100.0, 100.0));

  budgetBar->setValue(percent);
  budgetBar->setFormat(QString("%1 / %2 PLN  (%3%)")
                           .arg(monthlyExpenses, 0, 'f', 0)
                           .arg(budgetLimit, 0, 'f', 0)
                           .arg(percent));

  if (percent >= 90) {
    budgetBar->setStyleSheet(R"(
            QProgressBar {
                background-color: #1e293b; border: 1px solid #7f1d1d;
                border-radius: 6px; text-align: center; color: white; font-size: 12px;
            }
            QProgressBar::chunk {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #b91c1c, stop:1 #ef4444); border-radius: 6px;
            }
        )");
  } else if (percent >= 70) {
    budgetBar->setStyleSheet(R"(
            QProgressBar {
                background-color: #1e293b; border: 1px solid #78350f;
                border-radius: 6px; text-align: center; color: white; font-size: 12px;
            }
            QProgressBar::chunk {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #b45309, stop:1 #f59e0b); border-radius: 6px;
            }
        )");
  } else {
    budgetBar->setStyleSheet(R"(
            QProgressBar {
                background-color: #1e293b; border: 1px solid #334155;
                border-radius: 6px; text-align: center; color: white; font-size: 12px;
            }
            QProgressBar::chunk {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #1a6fdb, stop:1 #0d6efd); border-radius: 6px;
            }
        )");
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Wykres kołowy
// ═══════════════════════════════════════════════════════════════════════════════
void MainWindow::updateChart() {
  pieSeries->clear();

  QMap<QString, double> categoryTotals;
  for (const auto &t : wallet.getTransactions()) {
    if (dynamic_cast<Expense *>(t.get())) {
      QString cat = QString::fromStdString(t->getCategory());
      categoryTotals[cat] += t->getAmount();
    }
  }

  if (categoryTotals.isEmpty()) {
    auto *slice = pieSeries->append("Brak wydatków", 1.0);
    slice->setColor(QColor("#334155"));
    slice->setLabelColor(QColor("#94a3b8"));
    return;
  }

  static const QStringList palette = {
      "#0d6efd", "#06d6a0", "#ffd166", "#ef476f", "#118ab2",
      "#8338ec", "#fb5607", "#3a86ff", "#38bdf8", "#a3e635"};
  int colorIdx = 0;

  for (auto it = categoryTotals.constBegin(); it != categoryTotals.constEnd();
       ++it) {
    auto *slice = pieSeries->append(
        QString("%1 (%2 PLN)").arg(it.key()).arg(it.value(), 0, 'f', 2),
        it.value());
    slice->setColor(QColor(palette[colorIdx % palette.size()]));
    slice->setLabelColor(QColor("#e2e8f0"));
    slice->setLabelVisible(it.value() / pieSeries->sum() > 0.05);
    ++colorIdx;
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Wykres słupkowy trendów
// ═══════════════════════════════════════════════════════════════════════════════
void MainWindow::updateBarChart() {
  QChart *chart = barChartView->chart();
  chart->removeAllSeries();
  const auto oldAxes = chart->axes();
  for (auto *axis : oldAxes)
    chart->removeAxis(axis);

  QMap<QString, double> monthlyIncome;
  QMap<QString, double> monthlyExpense;

  for (const auto &t : wallet.getTransactions()) {
    QDate txDate =
        QDate::fromString(QString::fromStdString(t->getDate()), "yyyy-MM-dd");
    QString monthKey = txDate.toString("yyyy-MM");

    if (dynamic_cast<Income *>(t.get())) {
      monthlyIncome[monthKey] += t->getAmount();
    } else if (dynamic_cast<Expense *>(t.get())) {
      // Transfer jest pomijany — nie zmienia głównego bilansu
      monthlyExpense[monthKey] += t->getAmount();
    }
  }

  QStringList months;
  for (const auto &k : monthlyIncome.keys())
    if (!months.contains(k))
      months << k;
  for (const auto &k : monthlyExpense.keys())
    if (!months.contains(k))
      months << k;
  std::sort(months.begin(), months.end());

  if (months.isEmpty()) {
    QBarSet *placeholder = new QBarSet("Brak danych");
    placeholder->setColor(QColor("#334155"));
    *placeholder << 0;
    QBarSeries *series = new QBarSeries();
    series->append(placeholder);
    chart->addSeries(series);
    return;
  }

  // Cel 4: pastelowe kolory spójne z paskami kopertowymi
  // Zielony = identyczny jak "OK" w updateEnvelopeBudgets → #22c55e (ciemniejszy pastel)
  // Czerwony = identyczny jak "danger" w updateEnvelopeBudgets → #ef4444
  // Używamy lekko rozjaśnionych wariantów dla przyjemniejszego wykresu słupkowego
  QBarSet *incomeSet  = new QBarSet("Przychody");
  QBarSet *expenseSet = new QBarSet("Wydatki");
  incomeSet->setColor(QColor("#4ade80"));   // pastelowa zieleń (#22c55e rozjaśniona)
  expenseSet->setColor(QColor("#f87171"));  // pastelowa czerwień (#ef4444 rozjaśniona)
  // Etykiety tooltipów w legendzie
  incomeSet->setBorderColor(QColor("#22c55e"));
  expenseSet->setBorderColor(QColor("#ef4444"));

  QStringList displayMonths;
  for (const QString &m : months) {
    incomeSet->append(monthlyIncome.value(m, 0.0));
    expenseSet->append(monthlyExpense.value(m, 0.0));
    QDate d = QDate::fromString(m + "-01", "yyyy-MM-dd");
    displayMonths << d.toString("MMM yy");
  }

  QBarSeries *barSeries = new QBarSeries();
  barSeries->append(incomeSet);
  barSeries->append(expenseSet);
  barSeries->setLabelsVisible(false);
  chart->addSeries(barSeries);

  QBarCategoryAxis *axisX = new QBarCategoryAxis();
  axisX->append(displayMonths);
  axisX->setLabelsColor(QColor("#94a3b8"));
  axisX->setGridLineColor(QColor("#1e293b"));
  chart->addAxis(axisX, Qt::AlignBottom);
  barSeries->attachAxis(axisX);

  QValueAxis *axisY = new QValueAxis();
  axisY->setLabelFormat("%.0f");
  axisY->setLabelsColor(QColor("#94a3b8"));
  axisY->setGridLineColor(QColor("#1e293b"));
  axisY->setLinePenColor(QColor("#334155"));
  chart->addAxis(axisY, Qt::AlignLeft);
  barSeries->attachAxis(axisY);

  chart->legend()->setLabelColor(QColor("#94a3b8"));
}


// ═══════════════════════════════════════════════════════════════════════════════
//  Budżetowanie kopertowe — dynamiczne, z categoryLimits z Wallet
// ═══════════════════════════════════════════════════════════════════════════════
void MainWindow::updateEnvelopeBudgets() {
  // Wyczyść stare widgety
  while (QLayoutItem *item = envelopeLayout->takeAt(0)) {
    if (QWidget *w = item->widget())
      w->deleteLater();
    delete item;
  }

  const auto &limits = wallet.getCategoryLimits();

  if (limits.empty()) {
    QLabel *emptyLabel =
        new QLabel("Brak ustawionych limitów kopertowych.\n"
                   "Kliknij '⚙ Zarządzaj limitami', aby dodać kategorie.");
    emptyLabel->setStyleSheet(
        "color: #475569; font-size: 13px; padding: 16px;");
    emptyLabel->setAlignment(Qt::AlignCenter);
    envelopeLayout->addWidget(emptyLabel);
    envelopeLayout->addStretch();
    return;
  }

  QDate today = QDate::currentDate();
  int thisYear = today.year();
  int thisMonth = today.month();

  // Miesięczne wydatki per kategoria (case-insensitive matching)
  QMap<QString, double> spent;
  for (const auto &t : wallet.getTransactions()) {
    if (dynamic_cast<Expense *>(t.get())) {
      QDate txDate =
          QDate::fromString(QString::fromStdString(t->getDate()), "yyyy-MM-dd");
      if (txDate.year() == thisYear && txDate.month() == thisMonth) {
        QString cat = QString::fromStdString(t->getCategory());
        for (const auto &kv : limits) {
          if (cat.compare(QString::fromStdString(kv.first),
                          Qt::CaseInsensitive) == 0) {
            spent[QString::fromStdString(kv.first)] += t->getAmount();
            break;
          }
        }
      }
    }
  }

  // Domyślne ikony dla znanych kategorii
  static const QMap<QString, QString> icons = {
      {"Jedzenie", "🍕"}, {"Rata", "🏠"},   {"Transport", "🚗"},
      {"Rozrywka", "🎮"}, {"Zakupy", "🛍"},  {"Zdrowie", "💊"},
      {"Edukacja", "📚"}, {"Podróże", "✈"}, {"Sport", "⚽"},
  };

  // Rysuj karty w siatce (max 4 per wiersz)
  QGridLayout *grid = new QGridLayout();
  grid->setSpacing(10);
  int col = 0, row = 0;
  const int MAX_COLS = 4;

  for (const auto &kv : limits) {
    const QString cat = QString::fromStdString(kv.first);
    double limit = kv.second;
    double spentV = spent.value(cat, 0.0);
    int pct = static_cast<int>(std::min(spentV / limit * 100.0, 100.0));

    QWidget *card = new QWidget();
    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(14, 12, 14, 12);
    cardLayout->setSpacing(5);
    card->setStyleSheet(R"(
            background: #1e293b;
            border: 1px solid #334155;
            border-radius: 10px;
        )");

    QString icon = icons.value(cat, "📂");
    QLabel *nameLabel = new QLabel(QString("%1  %2").arg(icon).arg(cat));
    nameLabel->setStyleSheet(
        "color: #e2e8f0; font-weight: bold; font-size: 13px;");

    QLabel *amtLabel = new QLabel(
        QString("%1 / %2 PLN").arg(spentV, 0, 'f', 0).arg(limit, 0, 'f', 0));
    amtLabel->setStyleSheet("color: #94a3b8; font-size: 12px;");

    QProgressBar *bar = new QProgressBar();
    bar->setRange(0, 100);
    bar->setValue(pct);
    bar->setFixedHeight(14);
    bar->setTextVisible(false);

    QString barColor = (pct >= 90)   ? "#ef4444"
                       : (pct >= 70) ? "#f59e0b"
                                     : "#22c55e";
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
        )")
                           .arg(barColor));

    QLabel *pctLabel = new QLabel(QString("%1%").arg(pct));
    pctLabel->setStyleSheet(
        QString("color: %1; font-size: 11px; font-weight: bold;")
            .arg(barColor));
    pctLabel->setAlignment(Qt::AlignRight);

    cardLayout->addWidget(nameLabel);
    cardLayout->addWidget(amtLabel);
    cardLayout->addWidget(bar);
    cardLayout->addWidget(pctLabel);

    grid->addWidget(card, row, col++);
    if (col >= MAX_COLS) {
      col = 0;
      ++row;
    }
  }

  QWidget *gridWrapper = new QWidget();
  gridWrapper->setLayout(grid);
  gridWrapper->setStyleSheet("background: transparent;");
  envelopeLayout->addWidget(gridWrapper);
  envelopeLayout->addStretch();
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Zakładka 4 — Subkonta (Konta)
// ═══════════════════════════════════════════════════════════════════════════════
void MainWindow::refreshAccountsTab() {
  // Wyczyść stare widgety
  while (QLayoutItem *item = accountsLayout->takeAt(0)) {
    if (QWidget *w = item->widget())
      w->deleteLater();
    delete item;
  }

  const auto balances = wallet.getAccountBalances();

  if (balances.empty()) {
    QLabel *emptyLabel = new QLabel(
        "Brak transakcji.\nDodaj transakcję, aby zobaczyć stan subkont.");
    emptyLabel->setStyleSheet(
        "color: #475569; font-size: 14px; padding: 20px;");
    emptyLabel->setAlignment(Qt::AlignCenter);
    accountsLayout->addWidget(emptyLabel);
    accountsLayout->addStretch();
    return;
  }

  // Ikony dla znanych kont
  static const QMap<QString, QString> accountIcons = {
      {"Gotówka", "💵"},
      {"Konto bankowe", "🏦"},
      {"Karta kredytowa", "💳"},
      {"Oszczędności", "🐷"},
  };

  // Oblicz łączne saldo (do wykreślenia udziałów)
  double totalPositive = 0.0;
  for (const auto &kv : balances)
    if (kv.second > 0.0)
      totalPositive += kv.second;

  QGridLayout *grid = new QGridLayout();
  grid->setSpacing(12);
  int col = 0, row = 0;
  const int MAX_COLS = 3;

  for (const auto &kv : balances) {
    const QString accName = QString::fromStdString(kv.first);
    double balance = kv.second;

    QWidget *card = new QWidget();
    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(18, 16, 18, 16);
    cardLayout->setSpacing(6);

    // Kolor karty zależy od salda
    QString cardBorder = (balance >= 0) ? "#1a4f9f" : "#7f1d1d";
    card->setStyleSheet(QString(R"(
            background: #1e293b;
            border: 1px solid %1;
            border-radius: 12px;
        )")
                            .arg(cardBorder));

    // Nagłówek: ikona + nazwa konta
    QString icon = accountIcons.value(accName, "🏧");
    QLabel *titleLabel = new QLabel(QString("%1  %2").arg(icon).arg(accName));
    titleLabel->setStyleSheet(
        "color: #e2e8f0; font-size: 14px; font-weight: bold;");

    // Kwota
    QString balStr = QString("%1 PLN").arg(balance, 0, 'f', 2);
    QLabel *balLabel = new QLabel(balStr);
    QFont balFont = balLabel->font();
    balFont.setPointSize(18);
    balFont.setBold(true);
    balLabel->setFont(balFont);
    balLabel->setStyleSheet(balance >= 0 ? "color: #22c55e;"
                                         : "color: #ef4444;");

    // Mini pasek udziału (jeśli saldo dodatnie)
    if (balance > 0.0 && totalPositive > 0.0) {
      int share = static_cast<int>(balance / totalPositive * 100.0);
      QLabel *shareLabel =
          new QLabel(QString("Udział w łącznym saldo: %1%").arg(share));
      shareLabel->setStyleSheet("color: #64748b; font-size: 11px;");

      QProgressBar *shareBar = new QProgressBar();
      shareBar->setRange(0, 100);
      shareBar->setValue(share);
      shareBar->setFixedHeight(8);
      shareBar->setTextVisible(false);
      shareBar->setStyleSheet(R"(
                QProgressBar {
                    background-color: #0f172a; border: none; border-radius: 3px;
                }
                QProgressBar::chunk {
                    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                        stop:0 #1a6fdb, stop:1 #38bdf8);
                    border-radius: 3px;
                }
            )");

      cardLayout->addWidget(titleLabel);
      cardLayout->addWidget(balLabel);
      cardLayout->addWidget(shareLabel);
      cardLayout->addWidget(shareBar);
    } else {
      cardLayout->addWidget(titleLabel);
      cardLayout->addWidget(balLabel);
      if (balance < 0.0) {
        QLabel *warnLabel = new QLabel("⚠  Ujemne saldo konta");
        warnLabel->setStyleSheet("color: #ef4444; font-size: 11px;");
        cardLayout->addWidget(warnLabel);
      }
    }

    grid->addWidget(card, row, col++);
    if (col >= MAX_COLS) {
      col = 0;
      ++row;
    }
  }

  QWidget *gridWrapper = new QWidget();
  gridWrapper->setLayout(grid);
  gridWrapper->setStyleSheet("background: transparent;");
  accountsLayout->addWidget(gridWrapper);
  accountsLayout->addStretch();
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Zakładka celów
// ═══════════════════════════════════════════════════════════════════════════════
void MainWindow::refreshGoalsTab() {
  while (QLayoutItem *item = goalsLayout->takeAt(0)) {
    if (QWidget *w = item->widget())
      w->deleteLater();
    delete item;
  }

  const auto &goals = wallet.getGoals();

  if (goals.empty()) {
    QLabel *emptyLabel = new QLabel("Brak celów oszczędnościowych.\nKliknij "
                                    "'🎯 Nowy cel', aby dodać pierwszy.");
    emptyLabel->setStyleSheet(
        "color: #475569; font-size: 14px; padding: 20px;");
    emptyLabel->setAlignment(Qt::AlignCenter);
    goalsLayout->addWidget(emptyLabel);
    goalsLayout->addStretch();
    return;
  }

  // Cel 3: siatka kafelków — max 3 w rzędzie
  QGridLayout *grid = new QGridLayout();
  grid->setSpacing(12);
  const int MAX_COLS = 3;
  int col = 0, row = 0;

  for (size_t i = 0; i < goals.size(); ++i) {
    const Goal &g = goals[i];
    double pct_d =
        (g.targetAmount > 0.0)
            ? std::min(g.currentAmount / g.targetAmount * 100.0, 100.0)
            : 0.0;
    int pct = static_cast<int>(pct_d);

    QWidget *card = new QWidget();
    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(16, 12, 16, 12);
    cardLayout->setSpacing(6);
    card->setStyleSheet(R"(
            background: #1e293b;
            border: 1px solid #334155;
            border-radius: 10px;
        )");

    QHBoxLayout *headerRow = new QHBoxLayout();
    QLabel *nameLabel =
        new QLabel(QString("🎯  %1").arg(QString::fromStdString(g.name)));
    nameLabel->setStyleSheet(
        "color: #e2e8f0; font-size: 14px; font-weight: bold;");

    QLabel *pctLabel = new QLabel(QString("%1%").arg(pct));
    pctLabel->setStyleSheet(
        (pct >= 100) ? "color: #22c55e; font-size: 14px; font-weight: bold;"
                     : "color: #38bdf8; font-size: 14px; font-weight: bold;");

    headerRow->addWidget(nameLabel);
    headerRow->addStretch();
    headerRow->addWidget(pctLabel);
    cardLayout->addLayout(headerRow);

    QLabel *amtLabel = new QLabel(QString("%1 / %2 PLN zebrano")
                                      .arg(g.currentAmount, 0, 'f', 2)
                                      .arg(g.targetAmount, 0, 'f', 2));
    amtLabel->setStyleSheet("color: #64748b; font-size: 12px;");
    cardLayout->addWidget(amtLabel);

    QProgressBar *bar = new QProgressBar();
    bar->setRange(0, 100);
    bar->setValue(pct);
    bar->setFixedHeight(16);
    bar->setTextVisible(false);

    QString barColor = (pct >= 100) ? "#22c55e" : "#0d6efd";
    bar->setStyleSheet(QString(R"(
            QProgressBar {
                background-color: #0f172a; border: none; border-radius: 6px;
            }
            QProgressBar::chunk {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 %1, stop:1 %2);
                border-radius: 6px;
            }
        )")
                           .arg(barColor)
                           .arg(pct >= 100 ? "#06d6a0" : "#38bdf8"));
    cardLayout->addWidget(bar);

    if (pct >= 100) {
      QLabel *doneLabel = new QLabel("✅  Cel osiągnięty!");
      doneLabel->setStyleSheet(
          "color: #22c55e; font-size: 12px; font-weight: bold;");
      cardLayout->addWidget(doneLabel);
    }

    grid->addWidget(card, row, col++);
    if (col >= MAX_COLS) {
      col = 0;
      ++row;
    }
  }

  QWidget *gridWrapper = new QWidget();
  gridWrapper->setLayout(grid);
  gridWrapper->setStyleSheet("background: transparent;");
  goalsLayout->addWidget(gridWrapper);
  goalsLayout->addStretch();
}


// ═══════════════════════════════════════════════════════════════════════════════
//  Wyszukiwarka
// ═══════════════════════════════════════════════════════════════════════════════
void MainWindow::applySearch(const QString &text) {
  QString query = text.trimmed().toLower();
  double visibleBalance = 0.0;

  const auto &transactions = wallet.getTransactions();

  for (int i = 0; i < historyTable->rowCount(); ++i) {
    bool matches = query.isEmpty();
    if (!matches) {
      for (int col = 0; col < historyTable->columnCount(); ++col) {
        QTableWidgetItem *item = historyTable->item(i, col);
        if (item && item->text().toLower().contains(query)) {
          matches = true;
          break;
        }
      }
    }
    historyTable->setRowHidden(i, !matches);

    if (matches && i < static_cast<int>(transactions.size())) {
      const auto &t = transactions[i];
      if (dynamic_cast<Income *>(t.get()))
        visibleBalance += t->getAmount();
      else
        visibleBalance -= t->getAmount();
    }
  }

  if (query.isEmpty())
    updateBalanceDisplay();
  else
    balanceLabel->setText(
        QString("Saldo (wyniki: %1 PLN)").arg(visibleBalance, 0, 'f', 2));
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Eksport CSV
// ═══════════════════════════════════════════════════════════════════════════════
void MainWindow::exportToCSV() {
  QString path = QFileDialog::getSaveFileName(
      this, "Eksportuj transakcje do CSV", "transakcje.csv",
      "Pliki CSV (*.csv);;Wszystkie pliki (*)");

  if (path.isEmpty())
    return;

  QFile file(path);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QMessageBox::critical(this, "Błąd", "Nie można otworzyć pliku do zapisu.");
    return;
  }

  QTextStream out(&file);
  out.setEncoding(QStringConverter::Utf8);
  out << "Data,Kategoria,Typ,Kwota (PLN),Konto,Cykliczna\n";

  for (const auto &t : wallet.getTransactions()) {
    QString type = dynamic_cast<Income *>(t.get()) ? "Przychod" : "Wydatek";
    QString rec = t->isRecurring() ? "Tak" : "Nie";
    out << QString::fromStdString(t->getDate()) << ","
        << QString::fromStdString(t->getCategory()) << "," << type << ","
        << QString::number(t->getAmount(), 'f', 2) << ","
        << QString::fromStdString(t->getAccountName()) << "," << rec << "\n";
  }

  file.close();
  QMessageBox::information(this, "Sukces",
                           QString("✔  Wyeksportowano %1 transakcji do:\n%2")
                               .arg(wallet.getTransactions().size())
                               .arg(path));
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Cykliczne transakcje
// ═══════════════════════════════════════════════════════════════════════════════
void MainWindow::checkRecurringTransactions() {
  QDate today = QDate::currentDate();
  QStringList notifications;

  const auto &txList = wallet.getTransactions();
  size_t origCount = txList.size();

  for (size_t i = 0; i < origCount; ++i) {
    const auto &t = txList[i];
    if (!t->isRecurring())
      continue;

    QDate txDate =
        QDate::fromString(QString::fromStdString(t->getDate()), "yyyy-MM-dd");

    if (txDate.year() < today.year() ||
        (txDate.year() == today.year() && txDate.month() < today.month())) {
      QString cat = QString::fromStdString(t->getCategory());
      double amount = t->getAmount();
      std::string acc = t->getAccountName();
      QString todayStr = today.toString("yyyy-MM-dd");

      if (dynamic_cast<Income *>(t.get())) {
        wallet.addTransaction(std::make_unique<Income>(
            amount, cat.toStdString(), todayStr.toStdString(), true, acc));
      } else {
        wallet.addTransaction(std::make_unique<Expense>(
            amount, cat.toStdString(), todayStr.toStdString(), true, acc));
      }

      notifications << QString("• %1: %2 PLN (%3)")
                           .arg(cat)
                           .arg(amount, 0, 'f', 2)
                           .arg(dynamic_cast<Income *>(t.get()) ? "Przychód"
                                                                : "Wydatek");
    }
  }

  if (!notifications.isEmpty()) {
    QMessageBox::information(this, "Transakcje cykliczne",
                             QString("Automatycznie dodano %1 cykliczną/e "
                                     "transakcję/e na dzisiaj:\n\n%2")
                                 .arg(notifications.size())
                                 .arg(notifications.join("\n")));
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Aktualizacja etykiety salda
// ═══════════════════════════════════════════════════════════════════════════════
void MainWindow::updateBalanceDisplay() {
  double balance = wallet.calculateBalance();
  balanceLabel->setText(QString("Saldo: %1 PLN").arg(balance, 0, 'f', 2));
  balanceLabel->setStyleSheet(
      balance >= 0 ? "color: #38bdf8; padding: 4px 2px; font-weight: bold;"
                   : "color: #ef4444; padding: 4px 2px; font-weight: bold;");
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Odświeżenie tabeli (6 kolumn)
// ═══════════════════════════════════════════════════════════════════════════════
void MainWindow::refreshTable() {
  historyTable->setRowCount(0);
  const auto &transactions = wallet.getTransactions();

  for (int i = 0; i < static_cast<int>(transactions.size()); ++i) {
    const auto &t = transactions[i];
    historyTable->insertRow(i);

    historyTable->setItem(
        i, 0, new QTableWidgetItem(QString::fromStdString(t->getDate())));
    historyTable->setItem(
        i, 1, new QTableWidgetItem(QString::fromStdString(t->getCategory())));

    // Typ transakcji — Transfer wyświetlamy neutralnie
    bool isIncome    = (dynamic_cast<Income *>(t.get()) != nullptr);
    bool isTransfer  = (dynamic_cast<Transfer *>(t.get()) != nullptr);
    QString typeStr  = isTransfer ? "Transfer"
                     : isIncome  ? "Przychód"
                                 : "Wydatek";
    auto *typeItem = new QTableWidgetItem(typeStr);
    QColor typeColor = isTransfer ? QColor("#38bdf8")
                     : isIncome  ? QColor("#22c55e")
                                 : QColor("#ef4444");
    typeItem->setForeground(typeColor);
    historyTable->setItem(i, 2, typeItem);

    historyTable->setItem(
        i, 3, new QTableWidgetItem(QString::number(t->getAmount(), 'f', 2)));

    // Konto: dla Transfer pokaż "Z konta → Na konto"
    QString accountStr;
    if (auto *tr = dynamic_cast<Transfer *>(t.get())) {
      accountStr = QString("%1 → %2")
                       .arg(QString::fromStdString(tr->getFromAccount()))
                       .arg(QString::fromStdString(tr->getToAccount()));
    } else {
      accountStr = QString::fromStdString(t->getAccountName());
    }
    historyTable->setItem(i, 4, new QTableWidgetItem(accountStr));

    QString recStr = t->isRecurring() ? "♻  Tak" : "—";
    auto *recItem = new QTableWidgetItem(recStr);
    recItem->setForeground(t->isRecurring() ? QColor("#a3e635")
                                            : QColor("#475569"));
    historyTable->setItem(i, 5, recItem);
  }

  updateBalanceDisplay();

  if (!searchBar->text().isEmpty())
    applySearch(searchBar->text());
}