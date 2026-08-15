#include "DistanceCard.h"

#include <QLabel>
#include <QVBoxLayout>

DistanceCard::DistanceCard(QWidget *parent)
    : QWidget(parent), m_valueLabel(new QLabel(QStringLiteral("-- cm"), this))
{
    setObjectName(QStringLiteral("metricCard"));
    m_valueLabel->setObjectName(QStringLiteral("metricValue"));
    auto *layout = new QVBoxLayout(this);
    auto *title = new QLabel(QStringLiteral("KHOẢNG CÁCH HIỆN TẠI"), this);
    title->setObjectName(QStringLiteral("metricTitle"));
    layout->addWidget(title);
    layout->addWidget(m_valueLabel);
}

void DistanceCard::setDistance(double distanceCm)
{
    m_valueLabel->setText(QString::number(distanceCm, 'f', 1) + QStringLiteral(" cm"));
}
