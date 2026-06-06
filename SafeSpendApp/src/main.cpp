#include <QApplication>
#include "WelcomeWindow.hpp"
#include "ThemeManager.hpp"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // ── Zastosuj domyślny motyw (ciemny) przez ThemeManager ──────────────────
    ThemeManager::instance()->applyTheme(true);

    // ── Punkt startowy: nowoczesne okno powitalne ─────────────────────────────
    WelcomeWindow* welcome = new WelcomeWindow();
    welcome->show();

    return app.exec();
}