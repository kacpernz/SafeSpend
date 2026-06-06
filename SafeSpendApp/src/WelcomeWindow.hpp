#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <vector>
#include <memory>
#include "ITransaction.hpp"

class WelcomeWindow : public QWidget {
    Q_OBJECT

private:
    QLineEdit* passwordInput;
    QPushButton* loginButton;
    QPushButton* createButton;
    QLabel*      statusLabel;

    void onLoginClicked();
    void onCreateWalletClicked();
    void launchMainWindow(std::vector<std::unique_ptr<ITransaction>> data);

public:
    explicit WelcomeWindow(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
};
