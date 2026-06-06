#include "ThemeManager.hpp"
#include <QApplication>

// ─── Singleton ────────────────────────────────────────────────────────────────
ThemeManager::ThemeManager(QObject* parent) : QObject(parent) {}

ThemeManager* ThemeManager::instance() {
    static ThemeManager inst;
    return &inst;
}

// ─── Ciemny motyw ─────────────────────────────────────────────────────────────
QString ThemeManager::darkStyleSheet() {
    return R"(
        /* ── Tła główne ─────────────────────────────────────────────────── */
        QMainWindow, QDialog, QWidget {
            background-color: #111827;
            color: #f0f0f0;
            font-family: "Segoe UI", Arial, sans-serif;
        }

        /* ── Etykiety ───────────────────────────────────────────────────── */
        QLabel {
            color: #e2e8f0;
            background: transparent;
        }

        /* ── Przyciski ──────────────────────────────────────────────────── */
        QPushButton {
            background-color: #0d6efd;
            color: white;
            border-radius: 6px;
            padding: 8px 16px;
            font-weight: bold;
            font-size: 13px;
            border: none;
        }
        QPushButton:hover   { background-color: #3d8bfd; }
        QPushButton:pressed { background-color: #0a53be; }
        QPushButton:disabled { background-color: #334155; color: #64748b; }

        /* ── Tabela ─────────────────────────────────────────────────────── */
        QTableWidget {
            background-color: #1e293b;
            alternate-background-color: #243044;
            color: #e2e8f0;
            gridline-color: #334155;
            border: 1px solid #334155;
            border-radius: 4px;
            selection-background-color: #1a4f9f;
        }
        QTableWidget::item:selected { background-color: #1a4f9f; }
        QHeaderView::section {
            background-color: #0f172a;
            color: #94a3b8;
            padding: 6px;
            border: none;
            border-bottom: 1px solid #334155;
            font-weight: bold;
        }

        /* ── Pola tekstowe ──────────────────────────────────────────────── */
        QLineEdit, QDoubleSpinBox, QComboBox, QSpinBox {
            background-color: #1e293b;
            color: #e2e8f0;
            border: 1px solid #334155;
            padding: 6px 10px;
            border-radius: 6px;
            font-size: 13px;
        }
        QLineEdit:focus, QDoubleSpinBox:focus, QComboBox:focus {
            border: 1px solid #3d8bfd;
        }
        QComboBox::drop-down { border: none; }
        QComboBox QAbstractItemView {
            background-color: #1e293b;
            color: #e2e8f0;
            selection-background-color: #0d6efd;
        }

        /* ── Dialogi systemowe ──────────────────────────────────────────── */
        QMessageBox {
            background-color: #1e293b;
        }
        QMessageBox QLabel { color: #e2e8f0; }
        QInputDialog { background-color: #1e293b; }
        QInputDialog QLabel { color: #e2e8f0; }

        /* ── Paski postępu ──────────────────────────────────────────────── */
        QProgressBar {
            background-color: #1e293b;
            border: 1px solid #334155;
            border-radius: 6px;
            text-align: center;
            color: white;
            font-size: 12px;
        }
        QProgressBar::chunk {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #1a6fdb, stop:1 #0d6efd);
            border-radius: 6px;
        }

        /* ── Przewijanie ────────────────────────────────────────────────── */
        QScrollArea { background: #111827; border: none; }
        QScrollBar:vertical {
            background: #1e293b; width: 8px; border-radius: 4px;
        }
        QScrollBar::handle:vertical {
            background: #334155; border-radius: 4px; min-height: 20px;
        }
        QScrollBar::handle:vertical:hover { background: #475569; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }

        /* ── Zakładki ───────────────────────────────────────────────────── */
        QTabWidget::pane { border: none; background-color: #111827; }
        QTabBar::tab {
            background: #1e293b; color: #94a3b8;
            padding: 10px 22px; font-size: 13px; font-weight: bold;
            border: none; min-width: 110px;
        }
        QTabBar::tab:selected  { background: #0d6efd; color: white; }
        QTabBar::tab:hover:!selected { background: #273549; color: #cbd5e1; }

        /* ── Separator ──────────────────────────────────────────────────── */
        QFrame[frameShape="4"], QFrame[frameShape="5"] {
            color: #334155;
        }

        /* ── Pasek statusu ──────────────────────────────────────────────── */
        QStatusBar { background: #0f172a; color: #64748b; }
    )";
}

// ─── Jasny motyw ──────────────────────────────────────────────────────────────
QString ThemeManager::lightStyleSheet() {
    return R"(
        /* ── Tła główne ─────────────────────────────────────────────────── */
        QMainWindow, QDialog, QWidget {
            background-color: #f1f5f9;
            color: #1e293b;
            font-family: "Segoe UI", Arial, sans-serif;
        }

        /* ── Etykiety ───────────────────────────────────────────────────── */
        QLabel {
            color: #1e293b;
            background: transparent;
        }

        /* ── Przyciski ──────────────────────────────────────────────────── */
        QPushButton {
            background-color: #2563eb;
            color: white;
            border-radius: 6px;
            padding: 8px 16px;
            font-weight: bold;
            font-size: 13px;
            border: none;
        }
        QPushButton:hover   { background-color: #1d4ed8; }
        QPushButton:pressed { background-color: #1e40af; }
        QPushButton:disabled { background-color: #cbd5e1; color: #94a3b8; }

        /* ── Tabela ─────────────────────────────────────────────────────── */
        QTableWidget {
            background-color: #ffffff;
            alternate-background-color: #f8fafc;
            color: #1e293b;
            gridline-color: #e2e8f0;
            border: 1px solid #e2e8f0;
            border-radius: 4px;
            selection-background-color: #bfdbfe;
        }
        QTableWidget::item:selected { background-color: #bfdbfe; color: #1e3a8a; }
        QHeaderView::section {
            background-color: #e2e8f0;
            color: #475569;
            padding: 6px;
            border: none;
            border-bottom: 1px solid #cbd5e1;
            font-weight: bold;
        }

        /* ── Pola tekstowe ──────────────────────────────────────────────── */
        QLineEdit, QDoubleSpinBox, QComboBox, QSpinBox {
            background-color: #ffffff;
            color: #1e293b;
            border: 1px solid #cbd5e1;
            padding: 6px 10px;
            border-radius: 6px;
            font-size: 13px;
        }
        QLineEdit:focus, QDoubleSpinBox:focus, QComboBox:focus {
            border: 1px solid #2563eb;
        }
        QComboBox::drop-down { border: none; }
        QComboBox QAbstractItemView {
            background-color: #ffffff;
            color: #1e293b;
            selection-background-color: #bfdbfe;
        }

        /* ── Dialogi systemowe ──────────────────────────────────────────── */
        QMessageBox { background-color: #ffffff; }
        QMessageBox QLabel { color: #1e293b; }
        QInputDialog { background-color: #ffffff; }
        QInputDialog QLabel { color: #1e293b; }

        /* ── Paski postępu ──────────────────────────────────────────────── */
        QProgressBar {
            background-color: #e2e8f0;
            border: 1px solid #cbd5e1;
            border-radius: 6px;
            text-align: center;
            color: #1e293b;
            font-size: 12px;
        }
        QProgressBar::chunk {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #3b82f6, stop:1 #2563eb);
            border-radius: 6px;
        }

        /* ── Przewijanie ────────────────────────────────────────────────── */
        QScrollArea { background: #f1f5f9; border: none; }
        QScrollBar:vertical {
            background: #e2e8f0; width: 8px; border-radius: 4px;
        }
        QScrollBar::handle:vertical {
            background: #94a3b8; border-radius: 4px; min-height: 20px;
        }
        QScrollBar::handle:vertical:hover { background: #64748b; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }

        /* ── Zakładki ───────────────────────────────────────────────────── */
        QTabWidget::pane { border: none; background-color: #f1f5f9; }
        QTabBar::tab {
            background: #e2e8f0; color: #475569;
            padding: 10px 22px; font-size: 13px; font-weight: bold;
            border: none; min-width: 110px;
        }
        QTabBar::tab:selected  { background: #2563eb; color: white; }
        QTabBar::tab:hover:!selected { background: #cbd5e1; color: #1e293b; }

        /* ── Separator ──────────────────────────────────────────────────── */
        QFrame[frameShape="4"], QFrame[frameShape="5"] {
            color: #e2e8f0;
        }

        /* ── Pasek statusu ──────────────────────────────────────────────── */
        QStatusBar { background: #e2e8f0; color: #64748b; }
    )";
}

// ─── Publiczne metody ─────────────────────────────────────────────────────────
void ThemeManager::applyTheme(bool isDark) {
    m_isDark = isDark;
    qApp->setStyleSheet(isDark ? darkStyleSheet() : lightStyleSheet());
    emit themeChanged(isDark);
}

void ThemeManager::toggleTheme() {
    applyTheme(!m_isDark);
}

QString ThemeManager::toggleButtonLabel() const {
    return m_isDark ? "☀️  Jasny" : "🌙  Ciemny";
}
