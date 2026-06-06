#include "LoginDialog.hpp"
#include <QVBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>

LoginDialog::LoginDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Logowanie");
    resize(250, 120);

    QVBoxLayout* layout = new QVBoxLayout(this);

    QLabel* label = new QLabel("Podaj haslo do pliku:", this);
    passwordLineEdit = new QLineEdit(this);
    passwordLineEdit->setEchoMode(QLineEdit::Password);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    layout->addWidget(label);
    layout->addWidget(passwordLineEdit);
    layout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

QString LoginDialog::getPassword() const {
    return passwordLineEdit->text();
}