#pragma once

#include <QWidget>

class QLabel;

class DistanceCard : public QWidget
{
    Q_OBJECT

public:
    explicit DistanceCard(QWidget *parent = nullptr);
    void setDistance(double distanceCm);

private:
    QLabel *m_valueLabel;
};
