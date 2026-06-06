#pragma once
#include <QObject>
#include <QString>

// ─── ThemeManager ─────────────────────────────────────────────────────────────
// Singleton-helper zarządzający motywem wizualnym aplikacji.
// Używamy wzorca "sygnalizator Q_OBJECT" — ThemeManager::instance() zwraca
// obiekt, który emituje sygnał themeChanged(bool isDark) po każdym toggleTheme().
// Inne widgety (MainWindow, WelcomeWindow) łączą się z tym sygnałem i reagują
// aktualizując QChart::setTheme() lub inne zależne od koloru elementy.
// ─────────────────────────────────────────────────────────────────────────────

class ThemeManager : public QObject {
    Q_OBJECT

private:
    explicit ThemeManager(QObject* parent = nullptr);

    bool m_isDark = true;

    // Zwraca arkusz QSS dla ciemnego motywu
    static QString darkStyleSheet();

    // Zwraca arkusz QSS dla jasnego motywu
    static QString lightStyleSheet();

public:
    // Zwraca jedyną instancję (lazy singleton)
    static ThemeManager* instance();

    // Zwraca true jeśli aktywny jest ciemny motyw
    bool isDark() const { return m_isDark; }

    // Aplikuje wybrany motyw globalnie (qApp->setStyleSheet)
    void applyTheme(bool isDark);

    // Przełącza między trybami i emituje themeChanged()
    void toggleTheme();

    // Tekst etykiety przycisku dla aktualnego motywu
    QString toggleButtonLabel() const;

signals:
    // Emitowany po każdej zmianie motywu; isDark = nowa wartość
    void themeChanged(bool isDark);
};
