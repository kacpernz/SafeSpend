#include "WelcomeWindow.hpp"
#include "MainWindow.hpp"
#include "DatabaseManager.hpp"
#include "DatabaseException.hpp"

#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QInputDialog>
#include <QPainter>
#include <QLinearGradient>
#include <QFont>
#include <QGraphicsDropShadowEffect>

// ─── helper: create a subtle glow/shadow for a widget ────────────────────────
static void addShadow(QWidget* w, QColor color = QColor(0, 0, 0, 120), int blur = 18) {
    auto* effect = new QGraphicsDropShadowEffect(w);
    effect->setBlurRadius(blur);
    effect->setColor(color);
    effect->setOffset(0, 4);
    w->setGraphicsEffect(effect);
}

// ─── constructor ─────────────────────────────────────────────────────────────
WelcomeWindow::WelcomeWindow(QWidget* parent)
    : QWidget(parent)
{
    setWindowTitle("SafeSpend");
    setFixedSize(420, 520);
    setAttribute(Qt::WA_DeleteOnClose);

    // ── root layout ──────────────────────────────────────────────────────────
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(48, 48, 48, 40);
    root->setSpacing(0);

    // ── icon label (lock emoji rendered as large text) ────────────────────────
    QLabel* iconLabel = new QLabel("🔐", this);
    iconLabel->setAlignment(Qt::AlignHCenter);
    QFont iconFont = iconLabel->font();
    iconFont.setPointSize(52);
    iconLabel->setFont(iconFont);
    iconLabel->setStyleSheet("background: transparent; color: white;");
    root->addWidget(iconLabel);

    root->addSpacing(16);

    // ── main title ────────────────────────────────────────────────────────────
    QLabel* titleLabel = new QLabel("SafeSpend", this);
    titleLabel->setAlignment(Qt::AlignHCenter);
    QFont titleFont;
    titleFont.setFamily("Arial");
    titleFont.setPointSize(30);
    titleFont.setWeight(QFont::Bold);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("background: transparent; color: #ffffff; letter-spacing: 1px;");
    addShadow(titleLabel, QColor(13, 110, 253, 160), 24);
    root->addWidget(titleLabel);

    root->addSpacing(6);

    // ── subtitle ──────────────────────────────────────────────────────────────
    QLabel* subtitleLabel = new QLabel("Twój bezpieczny portfel finansowy", this);
    subtitleLabel->setAlignment(Qt::AlignHCenter);
    QFont subFont;
    subFont.setFamily("Arial");
    subFont.setPointSize(11);
    subtitleLabel->setFont(subFont);
    subtitleLabel->setStyleSheet("background: transparent; color: #9baacf;");
    root->addWidget(subtitleLabel);

    root->addSpacing(36);

    // ── password input ────────────────────────────────────────────────────────
    passwordInput = new QLineEdit(this);
    passwordInput->setEchoMode(QLineEdit::Password);
    passwordInput->setPlaceholderText("🔑  Wpisz hasło...");
    passwordInput->setFixedHeight(48);
    passwordInput->setStyleSheet(R"(
        QLineEdit {
            background-color: rgba(255, 255, 255, 0.08);
            color: #ffffff;
            border: 1.5px solid rgba(255, 255, 255, 0.15);
            border-radius: 10px;
            padding: 0 16px;
            font-size: 14px;
        }
        QLineEdit:focus {
            border: 1.5px solid #3d8bfd;
            background-color: rgba(255, 255, 255, 0.12);
        }
    )");
    root->addWidget(passwordInput);

    root->addSpacing(12);

    // ── status label (inline error / info messages) ───────────────────────────
    statusLabel = new QLabel("", this);
    statusLabel->setAlignment(Qt::AlignHCenter);
    statusLabel->setWordWrap(true);
    QFont stFont;
    stFont.setPointSize(10);
    statusLabel->setFont(stFont);
    statusLabel->setStyleSheet("background: transparent; color: #ff6b6b;");
    statusLabel->setFixedHeight(36);
    root->addWidget(statusLabel);

    root->addSpacing(4);

    // ── login button ──────────────────────────────────────────────────────────
    loginButton = new QPushButton("  Zaloguj do portfela", this);
    loginButton->setFixedHeight(50);
    loginButton->setCursor(Qt::PointingHandCursor);
    loginButton->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #1a6fdb, stop:1 #0d6efd);
            color: white;
            border: none;
            border-radius: 10px;
            font-size: 15px;
            font-weight: bold;
            padding-left: 8px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #2277e8, stop:1 #1a6fdb);
        }
        QPushButton:pressed {
            background: #0a53be;
        }
    )");
    addShadow(loginButton, QColor(13, 110, 253, 140), 20);
    root->addWidget(loginButton);

    root->addSpacing(12);

    // ── create wallet button ──────────────────────────────────────────────────
    createButton = new QPushButton("  Utwórz nowy portfel", this);
    createButton->setFixedHeight(50);
    createButton->setCursor(Qt::PointingHandCursor);
    createButton->setStyleSheet(R"(
        QPushButton {
            background: transparent;
            color: #9baacf;
            border: 1.5px solid rgba(155, 170, 207, 0.4);
            border-radius: 10px;
            font-size: 15px;
            font-weight: bold;
            padding-left: 8px;
        }
        QPushButton:hover {
            color: #ffffff;
            border: 1.5px solid rgba(255, 255, 255, 0.55);
            background: rgba(255, 255, 255, 0.05);
        }
        QPushButton:pressed {
            background: rgba(255, 255, 255, 0.10);
        }
    )");
    root->addWidget(createButton);

    root->addStretch();

    // ── footer ────────────────────────────────────────────────────────────────
    QLabel* footerLabel = new QLabel("Dane szyfrowane algorytmem XOR", this);
    footerLabel->setAlignment(Qt::AlignHCenter);
    QFont footerFont;
    footerFont.setPointSize(9);
    footerLabel->setFont(footerFont);
    footerLabel->setStyleSheet("background: transparent; color: #4a5568;");
    root->addWidget(footerLabel);

    // ── connections ───────────────────────────────────────────────────────────
    connect(loginButton,  &QPushButton::clicked, this, &WelcomeWindow::onLoginClicked);
    connect(createButton, &QPushButton::clicked, this, &WelcomeWindow::onCreateWalletClicked);
    connect(passwordInput, &QLineEdit::returnPressed, this, &WelcomeWindow::onLoginClicked);
}

// ─── custom gradient background ──────────────────────────────────────────────
void WelcomeWindow::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QLinearGradient gradient(0, 0, 0, height());
    gradient.setColorAt(0.0, QColor("#0f1629"));
    gradient.setColorAt(0.6, QColor("#111827"));
    gradient.setColorAt(1.0, QColor("#0a0e1a"));
    painter.fillRect(rect(), gradient);
}

// ─── login slot ──────────────────────────────────────────────────────────────
void WelcomeWindow::onLoginClicked() {
    QString password = passwordInput->text();
    if (password.isEmpty()) {
        statusLabel->setText("⚠  Wpisz hasło przed zalogowaniem.");
        return;
    }

    statusLabel->setStyleSheet("background: transparent; color: #ffd166;");
    statusLabel->setText("⏳  Odszyfrowywanie danych...");
    QApplication::processEvents();

    DatabaseManager dbManager;
    try {
        Wallet loadedWallet;
        dbManager.loadWallet(loadedWallet, "finanse_baza.bin", password.toStdString());
        launchMainWindow(std::move(loadedWallet));
    } catch (const DatabaseException& e) {
        statusLabel->setStyleSheet("background: transparent; color: #ff6b6b;");
        statusLabel->setText(QString("✗  Nieprawidłowe hasło lub uszkodzony plik."));
        passwordInput->clear();
        passwordInput->setFocus();
    }
}

// ─── create new wallet slot ───────────────────────────────────────────────────
void WelcomeWindow::onCreateWalletClicked() {
    QString password = passwordInput->text();
    if (password.isEmpty()) {
        statusLabel->setStyleSheet("background: transparent; color: #ff6b6b;");
        statusLabel->setText("⚠  Wpisz hasło dla nowego portfela.");
        return;
    }

    bool ok = false;
    QString confirm = QInputDialog::getText(
        this,
        "Potwierdź hasło",
        "Powtórz hasło do nowego portfela:",
        QLineEdit::Password,
        QString(),
        &ok
    );

    if (!ok) return;

    if (confirm != password) {
        statusLabel->setStyleSheet("background: transparent; color: #ff6b6b;");
        statusLabel->setText("✗  Hasła nie są identyczne. Spróbuj ponownie.");
        return;
    }

    DatabaseManager dbManager;
    Wallet emptyWallet;

    try {
        dbManager.saveWallet(emptyWallet, "finanse_baza.bin", password.toStdString());
    } catch (const DatabaseException& e) {
        QMessageBox::critical(
            this,
            "Błąd zapisu",
            QString("Nie udało się utworzyć pliku portfela.\n\nSzczegóły: %1")
                .arg(QString::fromStdString(e.what()))
        );
        return;
    }

    statusLabel->setStyleSheet("background: transparent; color: #06d6a0;");
    statusLabel->setText("✔  Portfel utworzony pomyślnie!");
    launchMainWindow(std::move(emptyWallet));
}

// ─── launch MainWindow and hide self ─────────────────────────────────────────
void WelcomeWindow::launchMainWindow(Wallet&& wallet) {
    MainWindow* mainWin = new MainWindow();
    mainWin->setAttribute(Qt::WA_DeleteOnClose);
    mainWin->loadWalletData(std::move(wallet));

    connect(mainWin, &QObject::destroyed, qApp, &QCoreApplication::quit);

    mainWin->show();
    this->hide();
}
