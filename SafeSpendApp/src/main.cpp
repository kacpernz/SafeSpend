#include <QApplication>
#include "WelcomeWindow.hpp"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // ── Dark Mode QSS (stosowany globalnie na całą aplikację) ─────────────────
    app.setStyleSheet(R"(
        QMainWindow, QDialog, QWidget {
            background-color: #111827;
        }
        QLabel {
            color: #f0f0f0;
        }
        QPushButton {
            background-color: #0d6efd;
            color: white;
            border-radius: 6px;
            padding: 8px 16px;
            font-weight: bold;
            font-size: 14px;
            border: none;
        }
        QPushButton:hover {
            background-color: #0b5ed7;
        }
        QPushButton:pressed {
            background-color: #0a53be;
        }
        QTableWidget {
            background-color: #1e293b;
            color: #ffffff;
            gridline-color: #334155;
            border: 1px solid #334155;
            border-radius: 4px;
            selection-background-color: #1a6fdb;
        }
        QHeaderView::section {
            background-color: #0f172a;
            color: #94a3b8;
            padding: 6px;
            border: 1px solid #334155;
            font-weight: bold;
        }
        QLineEdit, QDoubleSpinBox, QComboBox {
            background-color: #1e293b;
            color: white;
            border: 1px solid #334155;
            padding: 6px;
            border-radius: 4px;
        }
        QLineEdit:focus, QDoubleSpinBox:focus, QComboBox:focus {
            border: 1px solid #3d8bfd;
        }
        QMessageBox {
            background-color: #1e293b;
        }
        QMessageBox QLabel {
            color: white;
        }
        QInputDialog {
            background-color: #1e293b;
        }
        QInputDialog QLabel {
            color: #f0f0f0;
        }
    )");

    // ── Punkt startowy: nowoczesne okno powitalne ─────────────────────────────
    WelcomeWindow* welcome = new WelcomeWindow();
    welcome->show();

    return app.exec();
}