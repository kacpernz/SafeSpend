#pragma once
#include <QObject>
#include <QString>

// === ThemeManager ===
// Singleton zarządzający motywem wizualnym. Emituje themeChanged(bool isDark)
// po każdym przełączeniu — widgety (MainWindow, WelcomeWindow) reagują m.in.
// ręczną aktualizacją QChart::setTheme(), bo QSS nie sięga do QtCharts.

class ThemeManager : public QObject {
    Q_OBJECT

private:
    explicit ThemeManager(QObject* parent = nullptr);

    bool m_isDark = true;

    static QString darkStyleSheet();
    static QString lightStyleSheet();

public:
    static ThemeManager* instance();

    bool isDark() const { return m_isDark; }

    void applyTheme(bool isDark);
    void toggleTheme();
    QString toggleButtonLabel() const;

signals:
    // Emitowany po każdej zmianie motywu; isDark = nowa wartość
    void themeChanged(bool isDark);
};
