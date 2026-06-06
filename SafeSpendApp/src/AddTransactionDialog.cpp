#include "AddTransactionDialog.hpp"
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QLabel>

AddTransactionDialog::AddTransactionDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Dodaj transakcję");
    resize(360, 230);
    setStyleSheet(R"(
        QDialog {
            background-color: #111827;
        }
        QLabel {
            color: #94a3b8;
            font-size: 13px;
        }
        QComboBox, QLineEdit, QDoubleSpinBox {
            background-color: #1e293b;
            color: #e2e8f0;
            border: 1px solid #334155;
            border-radius: 6px;
            padding: 6px 10px;
            font-size: 13px;
        }
        QComboBox::drop-down {
            border: none;
        }
        QComboBox QAbstractItemView {
            background-color: #1e293b;
            color: #e2e8f0;
            selection-background-color: #0d6efd;
        }
        QDialogButtonBox QPushButton {
            background-color: #0d6efd;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 6px 18px;
            font-size: 13px;
            font-weight: bold;
        }
        QDialogButtonBox QPushButton:hover {
            background-color: #3d8bfd;
        }
    )");

    QFormLayout* layout = new QFormLayout(this);
    layout->setSpacing(12);
    layout->setContentsMargins(20, 20, 20, 16);

    typeComboBox = new QComboBox(this);
    typeComboBox->addItem("Wydatek");
    typeComboBox->addItem("Przychod");

    amountSpinBox = new QDoubleSpinBox(this);
    amountSpinBox->setMaximum(1000000.0);
    amountSpinBox->setDecimals(2);
    amountSpinBox->setSuffix(" PLN");

    categoryLineEdit = new QLineEdit(this);
    categoryLineEdit->setPlaceholderText("np. Jedzenie, Rata, Wynagrodzenie...");

    accountComboBox = new QComboBox(this);
    accountComboBox->addItem("Gotówka");
    accountComboBox->addItem("Konto bankowe");
    accountComboBox->addItem("Karta kredytowa");
    accountComboBox->addItem("Oszczędności");

    recurringCheckBox = new QCheckBox("Płatność co miesiąc", this);
    recurringCheckBox->setStyleSheet("QCheckBox { color: #9baacf; font-size: 13px; }");

    layout->addRow("Typ:",        typeComboBox);
    layout->addRow("Kwota:",      amountSpinBox);
    layout->addRow("Kategoria:",  categoryLineEdit);
    layout->addRow("Konto:",      accountComboBox);
    layout->addRow("",            recurringCheckBox);

    QDialogButtonBox* buttonBox =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
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