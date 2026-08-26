#include "PumpAutoConfigDialog.h"
#include "VirtualKeyboard.h"

#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

PumpAutoConfigDialog::PumpAutoConfigDialog(const QString &deviceId,
                                           bool autoMode,
                                           double startDistanceCm,
                                           double stopDistanceCm,
                                           QWidget *parent)
    : QDialog(parent), m_deviceId(deviceId)
{
    setWindowTitle(QStringLiteral("Cài đặt Bơm Tự Động: %1").arg(deviceId));
    resize(480, 320);
    setStyleSheet(
        "QDialog { background-color: #070d1e; color: #ecf2ff; font-family: sans-serif; } "
        "QLabel { color: #f1f5f9; } "
        "QDoubleSpinBox { background-color: #0f1c3f; color: #ffffff; border: 1.5px solid #233870; border-radius: 8px; font-size: 13px; font-weight: 700; padding: 4px 8px; min-height: 32px; } "
        "QDoubleSpinBox:focus { border: 2px solid #10b981; background-color: #162447; } "
        "QCheckBox { color: #cbd5e1; font-size: 13px; font-weight: 800; spacing: 8px; }"
    );

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(18, 16, 18, 16);
    mainLayout->setSpacing(12);

    // --- Header ---
    auto *headerRow = new QHBoxLayout;
    auto *titleIcon = new QLabel(QStringLiteral("⚡"));
    titleIcon->setStyleSheet("font-size: 18px;");
    auto *titleLbl = new QLabel(QStringLiteral("Tự Động Bật/Tắt Bơm Theo Khoảng Cách"));
    titleLbl->setStyleSheet("color: #38bdf8; font-size: 14px; font-weight: 900;");
    headerRow->addWidget(titleIcon);
    headerRow->addWidget(titleLbl);
    headerRow->addStretch();

    auto *closeBtn = new QPushButton(QStringLiteral("✕"));
    closeBtn->setFixedSize(26, 26);
    closeBtn->setStyleSheet("background: #1e293b; color: #ef4444; border: 1px solid #334155; border-radius: 13px; font-weight: 900; font-size: 11px;");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);
    headerRow->addWidget(closeBtn);
    mainLayout->addLayout(headerRow);

    // Container Frame
    auto *card = new QFrame;
    card->setStyleSheet("background-color: #0d1733; border: 1px solid #1c2b54; border-radius: 10px;");
    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(16, 14, 16, 14);
    cardLayout->setSpacing(12);

    // 1. Auto mode checkbox
    m_autoModeCheck = new QCheckBox(QStringLiteral("Kích hoạt chế độ Tự động Bơm (Auto Mode)"));
    m_autoModeCheck->setChecked(autoMode);
    m_autoModeCheck->setStyleSheet("QCheckBox { color: #10b981; font-size: 13px; font-weight: 900; }");
    cardLayout->addWidget(m_autoModeCheck);

    // 2. Form Grid for distances
    auto *formGrid = new QGridLayout;
    formGrid->setHorizontalSpacing(14);
    formGrid->setVerticalSpacing(10);

    auto *lblStart = new QLabel(QStringLiteral("BẬT Bơm khi khoảng cách ≥:"));
    lblStart->setStyleSheet("color: #94a3b8; font-size: 12px; font-weight: 700;");
    m_startDistanceSpin = new QDoubleSpinBox;
    m_startDistanceSpin->setRange(1.0, 500.0);
    m_startDistanceSpin->setValue(startDistanceCm > 0 ? startDistanceCm : 35.0);
    m_startDistanceSpin->setSuffix(QStringLiteral(" cm (Nước cạn)"));
    m_startDistanceSpin->setDecimals(1);
    if (auto *le = m_startDistanceSpin->findChild<QLineEdit*>()) {
        VirtualKeyboardDialog::attachToLineEdit(le, QStringLiteral("Nhập khoảng cách Bật Bơm (cm)"));
    }
    formGrid->addWidget(lblStart, 0, 0);
    formGrid->addWidget(m_startDistanceSpin, 0, 1);

    auto *lblStop = new QLabel(QStringLiteral("TẮT Bơm khi khoảng cách ≤:"));
    lblStop->setStyleSheet("color: #94a3b8; font-size: 12px; font-weight: 700;");
    m_stopDistanceSpin = new QDoubleSpinBox;
    m_stopDistanceSpin->setRange(1.0, 500.0);
    m_stopDistanceSpin->setValue(stopDistanceCm > 0 ? stopDistanceCm : 10.0);
    m_stopDistanceSpin->setSuffix(QStringLiteral(" cm (Nước đầy)"));
    m_stopDistanceSpin->setDecimals(1);
    if (auto *le = m_stopDistanceSpin->findChild<QLineEdit*>()) {
        VirtualKeyboardDialog::attachToLineEdit(le, QStringLiteral("Nhập khoảng cách Tắt Bơm (cm)"));
    }
    formGrid->addWidget(lblStop, 1, 0);
    formGrid->addWidget(m_stopDistanceSpin, 1, 1);

    cardLayout->addLayout(formGrid);

    auto *hintLbl = new QLabel(QStringLiteral("💡 Khi nước trong bể vơi đi (khoảng cách cảm biến siêu âm tăng lên), ESP32 sẽ tự kích hoạt Bơm. Khi nước dâng đầy (khoảng cách chạm ngưỡng ngắt), ESP32 sẽ tự ngắt Bơm."));
    hintLbl->setStyleSheet("color: #64748b; font-size: 10px; line-height: 14px; font-style: italic;");
    hintLbl->setWordWrap(true);
    cardLayout->addWidget(hintLbl);

    mainLayout->addWidget(card, 1);

    // --- Action Buttons ---
    auto *btnRow = new QHBoxLayout;
    btnRow->setSpacing(10);

    auto *saveBtn = new QPushButton(QStringLiteral("💾 Lưu Cấu Hình Xuống ESP32"));
    saveBtn->setCursor(Qt::PointingHandCursor);
    saveBtn->setMinimumHeight(36);
    saveBtn->setStyleSheet(
        "QPushButton { background: #10b981; color: #ffffff; border: none; border-radius: 8px; font-size: 12px; font-weight: 900; padding: 0 16px; } "
        "QPushButton:hover { background: #059669; } "
        "QPushButton:pressed { background: #047857; }"
    );

    m_statusLabel = new QLabel;
    m_statusLabel->setStyleSheet("color: #10b981; font-weight: 800; font-size: 11px;");

    connect(saveBtn, &QPushButton::clicked, this, [this] {
        const double startVal = m_startDistanceSpin->value();
        const double stopVal = m_stopDistanceSpin->value();

        QJsonObject thresholds;
        QJsonObject distObj;
        distObj.insert(QStringLiteral("min"), stopVal);
        distObj.insert(QStringLiteral("max"), startVal);
        distObj.insert(QStringLiteral("warning_below"), stopVal);
        distObj.insert(QStringLiteral("warning_above"), startVal);
        thresholds.insert(QStringLiteral("distance_cm"), distObj);

        QJsonObject config;
        config.insert(QStringLiteral("auto_mode"), m_autoModeCheck->isChecked());
        config.insert(QStringLiteral("distance_start_cm"), startVal);
        config.insert(QStringLiteral("distance_stop_cm"), stopVal);
        config.insert(QStringLiteral("sampling_interval_ms"), 2000);
        config.insert(QStringLiteral("thresholds"), thresholds);

        emit configSaved(m_deviceId, config);

        m_statusLabel->setText(QStringLiteral("✓ Đã gửi cấu hình xuống ESP32!"));
        QTimer::singleShot(1500, this, &QDialog::accept);
    });

    btnRow->addWidget(saveBtn);
    btnRow->addWidget(m_statusLabel);
    btnRow->addStretch();
    mainLayout->addLayout(btnRow);
}
