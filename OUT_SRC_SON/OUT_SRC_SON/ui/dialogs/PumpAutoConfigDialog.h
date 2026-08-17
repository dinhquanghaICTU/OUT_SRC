#pragma once

#include <QDialog>
#include <QJsonObject>
#include <QString>

class QCheckBox;
class QDoubleSpinBox;
class QLabel;

class PumpAutoConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PumpAutoConfigDialog(const QString &deviceId,
                                  bool autoMode,
                                  double startDistanceCm,
                                  double stopDistanceCm,
                                  QWidget *parent = nullptr);

signals:
    void configSaved(const QString &deviceId, const QJsonObject &config);

private:
    QString m_deviceId;
    QCheckBox *m_autoModeCheck = nullptr;
    QDoubleSpinBox *m_startDistanceSpin = nullptr;
    QDoubleSpinBox *m_stopDistanceSpin = nullptr;
    QLabel *m_statusLabel = nullptr;
};
