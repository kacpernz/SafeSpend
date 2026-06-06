#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include "Wallet.hpp"

class WelcomeWindow : public QWidget {
    Q_OBJECT

private:
    QLineEdit*   passwordInput;
    QPushButton* loginButton;
    QPushButton* createButton;
    QLabel*      statusLabel;

    void onLoginClicked();
    void onCreateWalletClicked();
    void launchMainWindow(Wallet&& wallet);

public:
    explicit WelcomeWindow(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
};
