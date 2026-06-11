#include "AddTransactionDialog.hpp"
#include "ThemeManager.hpp"
#include <QDialogButtonBox>
#include <QVBoxLayout>

static const QStringList ALL_ACCOUNTS = {"Gotówka", "Konto bankowe", "Karta kredytowa", "Oszczędności"};

AddTransactionDialog::AddTransactionDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Dodaj transakcję");
    resize(380, 280);
    setMinimumWidth(360);

    // === Styl dynamiczny (motyw) ===
    bool isDark = ThemeManager::instance()->isDark();
    QString bg      = isDark ? "#111827" : "#f8fafc";
    QString input   = isDark ? "#1e293b" : "#ffffff";
    QString border  = isDark ? "#334155" : "#cbd5e1";
    QString text    = isDark ? "#e2e8f0" : "#1e293b";
    QString accent  = isDark ? "#3d8bfd" : "#2563eb";
    QString subtext = isDark ? "#94a3b8" : "#475569";

    setStyleSheet(QString(R"(
        QDialog { background-color: %1; }
        QLabel  { color: %2; font-size: 13px; }
        QComboBox, QLineEdit, QDoubleSpinBox {
            background-color: %3;
            color: %4;
            border: 1px solid %5;
            border-radius: 6px;
            padding: 6px 10px;
            font-size: 13px;
        }
        QComboBox::drop-down { border: none; }
        QComboBox QAbstractItemView {
            background-color: %3;
            color: %4;
            selection-background-color: %6;
        }
        QComboBox:focus, QLineEdit:focus, QDoubleSpinBox:focus {
            border: 1px solid %6;
        }
        QCheckBox { color: %7; font-size: 13px; }
        QDialogButtonBox QPushButton {
            background-color: %6;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 6px 18px;
            font-size: 13px;
            font-weight: bold;
        }
        QDialogButtonBox QPushButton:hover { opacity: 0.85; }
    )").arg(bg).arg(text).arg(input).arg(text).arg(border).arg(accent).arg(subtext));

    // === Główny layout ===
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 16);
    mainLayout->setSpacing(8);

    formLayout = new QFormLayout();
    formLayout->setSpacing(10);
    formLayout->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    mainLayout->addLayout(formLayout);

    // === Typ transakcji ===
    typeComboBox = new QComboBox(this);
    typeComboBox->addItem("Wydatek");
    typeComboBox->addItem("Przychod");
    typeComboBox->addItem("Transfer");
    formLayout->addRow("Typ:", typeComboBox);

    // === Kwota ===
    amountSpinBox = new QDoubleSpinBox(this);
    amountSpinBox->setMaximum(1000000.0);
    amountSpinBox->setDecimals(2);
    amountSpinBox->setSuffix(" PLN");
    formLayout->addRow("Kwota:", amountSpinBox);

    // === Pola dla Income / Expense ===
    categoryLabel    = new QLabel("Kategoria:", this);
    categoryLineEdit = new QLineEdit(this);
    categoryLineEdit->setPlaceholderText("np. Jedzenie, Rata, Wynagrodzenie...");
    formLayout->addRow(categoryLabel, categoryLineEdit);

    accountLabel   = new QLabel("Konto:", this);
    accountComboBox = new QComboBox(this);
    for (const QString& acc : ALL_ACCOUNTS)
        accountComboBox->addItem(acc);
    formLayout->addRow(accountLabel, accountComboBox);

    recurringLabel    = new QLabel("", this);
    recurringCheckBox = new QCheckBox("Płatność co miesiąc", this);
    formLayout->addRow(recurringLabel, recurringCheckBox);

    // === Pola dla Transfer ===
    fromAccountLabel   = new QLabel("Z konta:", this);
    fromAccountComboBox = new QComboBox(this);
    for (const QString& acc : ALL_ACCOUNTS)
        fromAccountComboBox->addItem(acc);
    formLayout->addRow(fromAccountLabel, fromAccountComboBox);

    toAccountLabel   = new QLabel("Na konto:", this);
    toAccountComboBox = new QComboBox(this);
    for (const QString& acc : ALL_ACCOUNTS)
        toAccountComboBox->addItem(acc);
    if (toAccountComboBox->count() > 1)
        toAccountComboBox->setCurrentIndex(1);
    formLayout->addRow(toAccountLabel, toAccountComboBox);

    // === Przyciski OK / Anuluj ===
    QDialogButtonBox* buttonBox =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    mainLayout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // === Dynamiczne przełączanie pól przy zmianie typu ===
    connect(typeComboBox, &QComboBox::currentTextChanged,
            this, &AddTransactionDialog::updateFieldVisibility);

    updateFieldVisibility(typeComboBox->currentText());
}

void AddTransactionDialog::updateFieldVisibility(const QString& type) {
    bool isTransfer = (type == "Transfer");

    categoryLabel->setVisible(!isTransfer);
    categoryLineEdit->setVisible(!isTransfer);
    accountLabel->setVisible(!isTransfer);
    accountComboBox->setVisible(!isTransfer);
    recurringLabel->setVisible(!isTransfer);
    recurringCheckBox->setVisible(!isTransfer);

    fromAccountLabel->setVisible(isTransfer);
    fromAccountComboBox->setVisible(isTransfer);
    toAccountLabel->setVisible(isTransfer);
    toAccountComboBox->setVisible(isTransfer);

    adjustSize();
}

QString AddTransactionDialog::getTransactionType() const {
    return typeComboBox->currentText();
}

double AddTransactionDialog::getAmount() const {
    return amountSpinBox->value();
}

QString AddTransactionDialog::getCategory() const {
    return categoryLineEdit->text();
}

bool AddTransactionDialog::getIsRecurring() const {
    return recurringCheckBox->isChecked();
}

QString AddTransactionDialog::getAccountName() const {
    return accountComboBox->currentText();
}

QString AddTransactionDialog::getFromAccount() const {
    return fromAccountComboBox->currentText();
}

QString AddTransactionDialog::getToAccount() const {
    return toAccountComboBox->currentText();
}