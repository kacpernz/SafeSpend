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
    QLabel*      m_iconLabel;      // przechowujemy bezpośrednio, bez wyszukiwania
    QPushButton* themeToggleBtn;

    void onLoginClicked();
    void onCreateWalletClicked();
    void launchMainWindow(Wallet&& wallet, const QString& password);

private slots:
    void onThemeChanged(bool isDark);

public:
    explicit WelcomeWindow(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
};

