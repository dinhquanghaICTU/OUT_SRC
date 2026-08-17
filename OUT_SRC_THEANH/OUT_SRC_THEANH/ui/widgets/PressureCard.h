#pragma once

#include <QWidget>

class QLabel;

class PressureCard : public QWidget
{
    Q_OBJECT

public:
    explicit PressureCard(QWidget *parent = nullptr);
    void setPressure(double pressureHpa);

private:
    QLabel *m_valueLabel;
};
