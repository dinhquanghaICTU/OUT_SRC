#include "CoolingConfigDialog.h"

#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

CoolingConfigDialog::CoolingConfigDialog(const QJsonObject &currentConfig, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Cấu Hình Hệ Thống Làm Mát"));
    setModal(true);
    setFixedSize(450, 340);
    setStyleSheet(
        "QDialog { background-color: #0f172a; color: #f8fafc; font-family: 'Noto Sans', sans-serif; } "
        "QLabel { color: #94a3b8; font-size: 12px; font-weight: 600; } "
        "QDoubleSpinBox, QSpinBox { background-color: #1e293b; color: #38bdf8; border: 1.5px solid #334155; "
        "border-radius: 8px; padding: 6px 12px; font-size: 13px; font-weight: 700; } "
        "QDoubleSpinBox:focus, QSpinBox:focus { border-color: #06b6d4; background-color: #0b1329; } "
        "QPushButton { border-radius: 10px; font-weight: 700; font-size: 12px; padding: 8px 18px; } "
        "QPushButton#saveBtn { background-color: #06b6d4; color: #0b1329; border: none; } "
        "QPushButton#saveBtn:hover { background-color: #22d3ee; } "
        "QPushButton#cancelBtn { background-color: #334155; color: #f8fafc; border: none; } "
        "QPushButton#cancelBtn:hover { background-color: #475569; }"
    );

    setupUi();

    // Fill initial values from currentConfig
    const double fanStart = currentConfig.value(QStringLiteral("fan_start_temp")).toDouble(35.0);
    const double fanStop = currentConfig.value(QStringLiteral("fan_stop_temp")).toDouble(28.0);
    const double soundMax = currentConfig.value(QStringLiteral("max_sound_vpp")).toDouble(1.5);
    const int interval = currentConfig.value(QStringLiteral("sampling_interval_seconds")).toInt(2);

    m_fanStartSpin->setValue(fanStart);
    m_fanStopSpin->setValue(fanStop);
    m_soundMaxSpin->setValue(soundMax);
    m_intervalSpin->setValue(interval);
}

void CoolingConfigDialog::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 20);
    mainLayout->setSpacing(16);

    // Header Title
    auto *headerLayout = new QHBoxLayout;
    auto *iconLabel = new QLabel(QStringLiteral(""), this); iconLabel->hide();
    iconLabel->setStyleSheet("font-size: 24px; color: #06b6d4;");
    headerLayout->addWidget(iconLabel);

    auto *titleBox = new QVBoxLayout;
    auto *titleLabel = new QLabel(tr("Ngưỡng Tự Động Làm Mát"), this);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: 900; color: #f8fafc;");
    auto *subtitleLabel = new QLabel(tr("Cài đặt nhiệt độ kích hoạt quạt và chu kỳ lấy mẫu"), this);
    subtitleLabel->setStyleSheet("font-size: 11px; color: #64748b; font-weight: 500;");
    titleBox->addWidget(titleLabel);
    titleBox->addWidget(subtitleLabel);
    headerLayout->addLayout(titleBox);
    headerLayout->addStretch();
    mainLayout->addLayout(headerLayout);

    // Form Container Card
    auto *formCard = new QFrame(this);
    formCard->setStyleSheet("background-color: #1e293b; border-radius: 12px; border: 1px solid #334155;");
    auto *formLayout = new QFormLayout(formCard);
    formLayout->setContentsMargins(18, 16, 18, 16);
    formLayout->setSpacing(12);
    formLayout->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    m_fanStartSpin = new QDoubleSpinBox(formCard);
    m_fanStartSpin->setRange(15.0, 70.0);
    m_fanStartSpin->setSingleStep(0.5);
    m_fanStartSpin->setSuffix(QStringLiteral(" °C"));
    formLayout->addRow(tr("Nhiệt độ TỰ BẬT Quạt (Cao):"), m_fanStartSpin);

    m_fanStopSpin = new QDoubleSpinBox(formCard);
    m_fanStopSpin->setRange(10.0, 60.0);
    m_fanStopSpin->setSingleStep(0.5);
    m_fanStopSpin->setSuffix(QStringLiteral(" °C"));
    formLayout->addRow(tr("Nhiệt độ TỰ TẮT Quạt (Mát):"), m_fanStopSpin);

    m_soundMaxSpin = new QDoubleSpinBox(formCard);
    m_soundMaxSpin->setRange(0.1, 5.0);
    m_soundMaxSpin->setSingleStep(0.1);
    m_soundMaxSpin->setSuffix(QStringLiteral(" Vpp"));
    formLayout->addRow(tr("Ngưỡng cảnh báo độ ồn quạt:"), m_soundMaxSpin);

    m_intervalSpin = new QSpinBox(formCard);
    m_intervalSpin->setRange(1, 60);
    m_intervalSpin->setSingleStep(1);
    m_intervalSpin->setSuffix(QStringLiteral(" giây"));
    formLayout->addRow(tr("Chu kỳ cập nhật mẫu:"), m_intervalSpin);

    mainLayout->addWidget(formCard);

    // Dialog Action Buttons
    auto *btnLayout = new QHBoxLayout;
    btnLayout->addStretch();

    auto *cancelBtn = new QPushButton(tr("Hủy Bỏ"), this);
    cancelBtn->setObjectName(QStringLiteral("cancelBtn"));
    cancelBtn->setCursor(Qt::PointingHandCursor);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnLayout->addWidget(cancelBtn);

    auto *saveBtn = new QPushButton(tr("Lưu Cấu Hình"), this);
    saveBtn->setObjectName(QStringLiteral("saveBtn"));
    saveBtn->setCursor(Qt::PointingHandCursor);
    connect(saveBtn, &QPushButton::clicked, this, [this] {
        if (m_fanStopSpin->value() >= m_fanStartSpin->value()) {
            m_fanStopSpin->setValue(m_fanStartSpin->value() - 2.0);
        }
        accept();
    });
    btnLayout->addWidget(saveBtn);

    mainLayout->addLayout(btnLayout);
}

QJsonObject CoolingConfigDialog::configData() const
{
    return QJsonObject{
        {QStringLiteral("fan_start_temp"), m_fanStartSpin->value()},
        {QStringLiteral("fan_stop_temp"), m_fanStopSpin->value()},
        {QStringLiteral("max_sound_vpp"), m_soundMaxSpin->value()},
        {QStringLiteral("sampling_interval_seconds"), m_intervalSpin->value()}
    };
}
