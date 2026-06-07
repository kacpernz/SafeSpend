#include "WelcomeWindow.hpp"
#include "MainWindow.hpp"
#include "DatabaseManager.hpp"
#include "DatabaseException.hpp"
#include "ThemeManager.hpp"

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

// ─── helper: shadow ───────────────────────────────────────────────────────────
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
    setFixedSize(420, 540);
    setAttribute(Qt::WA_DeleteOnClose);

    // ── root layout ──────────────────────────────────────────────────────────
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(48, 40, 48, 32);
    root->setSpacing(0);

    // ── Przycisk motywu (prawy górny róg) ────────────────────────────────────
    QHBoxLayout* topBar = new QHBoxLayout();
    topBar->addStretch();

    themeToggleBtn = new QPushButton(ThemeManager::instance()->toggleButtonLabel(), this);
    themeToggleBtn->setFixedHeight(30);
    themeToggleBtn->setCursor(Qt::PointingHandCursor);
    themeToggleBtn->setStyleSheet(R"(
        QPushButton {
            background: rgba(255,255,255,0.10);
            color: #94a3b8;
            border: 1px solid rgba(255,255,255,0.18);
            border-radius: 8px;
            font-size: 12px;
            font-weight: bold;
            padding: 0 12px;
        }
        QPushButton:hover {
            background: rgba(255,255,255,0.18);
            color: white;
        }
    )");
    topBar->addWidget(themeToggleBtn);
    root->addLayout(topBar);
    root->addSpacing(8);

    // ── icon label ────────────────────────────────────────────────────
    m_iconLabel = new QLabel("🔐", this);
    m_iconLabel->setAlignment(Qt::AlignHCenter);
    QFont iconFont = m_iconLabel->font();
    iconFont.setPointSize(52);
    m_iconLabel->setFont(iconFont);
    m_iconLabel->setStyleSheet("background: transparent; color: white;");
    root->addWidget(m_iconLabel);
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

    // ── status label ─────────────────────────────────────────────────────────
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
        QPushButton:pressed { background: #0a53be; }
    )");
    addShadow(loginButton, QColor(13, 110, 253, 140), 20);
    root->addWidget(loginButton);
    root->addSpacing(12);

    // ── create button ─────────────────────────────────────────────────────────
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
        QPushButton:pressed { background: rgba(255, 255, 255, 0.10); }
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
    connect(loginButton,   &QPushButton::clicked, this, &WelcomeWindow::onLoginClicked);
    connect(createButton,  &QPushButton::clicked, this, &WelcomeWindow::onCreateWalletClicked);
    connect(passwordInput, &QLineEdit::returnPressed, this, &WelcomeWindow::onLoginClicked);

    // Przycisk motywu
    connect(themeToggleBtn, &QPushButton::clicked, this, [this]() {
        ThemeManager::instance()->toggleTheme();
    });

    // Reaguj na zmianę motywu (aktualizuj etykietę + odmaluj gradient)
    connect(ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &WelcomeWindow::onThemeChanged);
}

// ─── gradient background — dostosowany do aktywnego motywu ───────────────────
void WelcomeWindow::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QLinearGradient gradient(0, 0, 0, height());
    if (ThemeManager::instance()->isDark()) {
        gradient.setColorAt(0.0, QColor("#0f1629"));
        gradient.setColorAt(0.6, QColor("#111827"));
        gradient.setColorAt(1.0, QColor("#0a0e1a"));
    } else {
        gradient.setColorAt(0.0, QColor("#dbeafe"));
        gradient.setColorAt(0.6, QColor("#eff6ff"));
        gradient.setColorAt(1.0, QColor("#f1f5f9"));
    }
    painter.fillRect(rect(), gradient);
}

// ─── Reakcja na zmianę motywu ───────────────────────────────────────────────
void WelcomeWindow::onThemeChanged(bool isDark) {
    // Zaktualizuj etykietę przycisku
    themeToggleBtn->setText(ThemeManager::instance()->toggleButtonLabel());
    // Odmaluj gradient tła
    update();

    // Dostosuj kolor ikony do motywu
    if (m_iconLabel) {
        m_iconLabel->setStyleSheet(
            isDark ? "background: transparent; color: white;"
                   : "background: transparent; color: #1e3a8a;");
    }
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
        launchMainWindow(std::move(loadedWallet), password);

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
        this, "Potwierdź hasło",
        "Powtórz hasło do nowego portfela:",
        QLineEdit::Password, QString(), &ok);

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
        QMessageBox::critical(this, "Błąd zapisu",
            QString("Nie udało się utworzyć pliku portfela.\n\nSzczegóły: %1")
                .arg(QString::fromStdString(e.what())));
        return;
    }

    statusLabel->setStyleSheet("background: transparent; color: #06d6a0;");
    statusLabel->setText("✔  Portfel utworzony pomyślnie!");
    launchMainWindow(std::move(emptyWallet), password);
}

// ─── launch MainWindow ────────────────────────────────────────────────────────
void WelcomeWindow::launchMainWindow(Wallet&& wallet, const QString& password) {
    MainWindow* mainWin = new MainWindow();
    mainWin->setAttribute(Qt::WA_DeleteOnClose);
    mainWin->loadWalletData(std::move(wallet), password.toStdString());

    connect(mainWin, &QObject::destroyed, qApp, &QCoreApplication::quit);

    mainWin->show();
    this->hide();
}
