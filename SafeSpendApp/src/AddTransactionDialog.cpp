#include "AddTransactionDialog.hpp"
#include <QFormLayout>
#include <QDialogButtonBox>

AddTransactionDialog::AddTransactionDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Dodaj transakcję");
    resize(320, 190);

    QFormLayout* layout = new QFormLayout(this);
    layout->setSpacing(10);
    layout->setContentsMargins(20, 20, 20, 16);

    typeComboBox = new QComboBox(this);
    typeComboBox->addItem("Przychod");
    typeComboBox->addItem("Wydatek");

    amountSpinBox = new QDoubleSpinBox(this);
    amountSpinBox->setMaximum(1000000.0);
    amountSpinBox->setSuffix(" PLN");

    categoryLineEdit = new QLineEdit(this);
    categoryLineEdit->setPlaceholderText("np. Jedzenie, Rata, Wynagrodzenie...");

    recurringCheckBox = new QCheckBox("Płatność co miesiąc", this);
    recurringCheckBox->setStyleSheet("QCheckBox { color: #9baacf; font-size: 13px; }");

    layout->addRow("Typ:",       typeComboBox);
    layout->addRow("Kwota:",     amountSpinBox);
    layout->addRow("Kategoria:", categoryLineEdit);
    layout->addRow("",           recurringCheckBox);

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