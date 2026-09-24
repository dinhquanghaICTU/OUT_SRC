#pragma once

#include <QDialog>
#include <QJsonObject>

class QLabel;

class CoolingConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CoolingConfigDialog(const QJsonObject &currentConfig, QWidget *parent = nullptr);
    ~CoolingConfigDialog() override = default;

    QJsonObject configData() const;

private:
    void setupUi();

    // Các giá trị thực (double để tránh cast, intervalSec lưu dạng double rồi cast khi trả ra)
    double m_fanStartTemp = 35.0;
    double m_fanStopTemp  = 28.0;
    double m_maxSoundVpp  = 1.5;
    double m_intervalSec  = 2.0;

    // Label hiển thị giá trị hiện tại của mỗi field
    QLabel *m_fanStartLabel = nullptr;
    QLabel *m_fanStopLabel  = nullptr;
    QLabel *m_soundMaxLabel = nullptr;
    QLabel *m_intervalLabel = nullptr;
};
