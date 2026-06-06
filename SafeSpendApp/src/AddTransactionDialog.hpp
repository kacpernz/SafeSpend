#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QString>

class AddTransactionDialog : public QDialog {
    Q_OBJECT

private:
    QComboBox*     typeComboBox;
    QDoubleSpinBox* amountSpinBox;
    QLineEdit*     categoryLineEdit;
    QCheckBox*     recurringCheckBox;

public:
    AddTransactionDialog(QWidget* parent = nullptr);

    QString getTransactionType() const;
    double  getAmount()          const;
    QString getCategory()        const;
    bool    getIsRecurring()     const;
};