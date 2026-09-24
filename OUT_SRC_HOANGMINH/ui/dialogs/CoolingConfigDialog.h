#pragma once

#include <QDialog>
#include <QJsonObject>

class QDoubleSpinBox;
class QSpinBox;
class QLineEdit;

class CoolingConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CoolingConfigDialog(const QJsonObject &currentConfig, QWidget *parent = nullptr);
    ~CoolingConfigDialog() override = default;

    QJsonObject configData() const;

private:
    void setupUi();

    QDoubleSpinBox *m_fanStartSpin = nullptr;
    QDoubleSpinBox *m_fanStopSpin = nullptr;
    QDoubleSpinBox *m_soundMaxSpin = nullptr;
    QSpinBox *m_intervalSpin = nullptr;
};
