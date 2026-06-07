#pragma once
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QString>
#include <QFormLayout>

class AddTransactionDialog : public QDialog {
    Q_OBJECT

private:
    QComboBox*      typeComboBox;
    QDoubleSpinBox* amountSpinBox;

    // Widgety dla Income/Expense
    QLabel*         categoryLabel;
    QLineEdit*      categoryLineEdit;
    QLabel*         accountLabel;
    QComboBox*      accountComboBox;
    QLabel*         recurringLabel;
    QCheckBox*      recurringCheckBox;

    // Widgety dla Transfer
    QLabel*         fromAccountLabel;
    QComboBox*      fromAccountComboBox;
    QLabel*         toAccountLabel;
    QComboBox*      toAccountComboBox;

    QFormLayout*    formLayout;

    void updateFieldVisibility(const QString& type);

public:
    AddTransactionDialog(QWidget* parent = nullptr);

    QString getTransactionType() const;
    double  getAmount()          const;
    QString getCategory()        const;
    bool    getIsRecurring()     const;
    QString getAccountName()     const;
    QString getFromAccount()     const;
    QString getToAccount()       const;
};