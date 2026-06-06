#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QString>

class LoginDialog : public QDialog {
    Q_OBJECT

private:
    QLineEdit* passwordLineEdit;

public:
    LoginDialog(QWidget* parent = nullptr);
    QString getPassword() const;
};