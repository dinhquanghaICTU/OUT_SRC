#include "PressureCard.h"

#include <QLabel>
#include <QVBoxLayout>

PressureCard::PressureCard(QWidget *parent)
    : QWidget(parent), m_valueLabel(new QLabel(QStringLiteral("-- hPa"), this))
{
    setObjectName(QStringLiteral("metricCard"));
    m_valueLabel->setObjectName(QStringLiteral("metricValue"));
    auto *layout = new QVBoxLayout(this);
    auto *title = new QLabel(QStringLiteral("ÁP SUẤT KHÍ QUYỂN"), this);
    title->setObjectName(QStringLiteral("metricTitle"));
    layout->addWidget(title);
    layout->addWidget(m_valueLabel);
}

void PressureCard::setPressure(double pressureHpa)
{
    m_valueLabel->setText(QString::number(pressureHpa, 'f', 1) + QStringLiteral(" hPa"));
}
