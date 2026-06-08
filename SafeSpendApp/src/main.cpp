#include <QApplication>
#include "WelcomeWindow.hpp"
#include "ThemeManager.hpp"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    ThemeManager::instance()->applyTheme(true);

    WelcomeWindow* welcome = new WelcomeWindow();
    welcome->show();

    return app.exec();
}