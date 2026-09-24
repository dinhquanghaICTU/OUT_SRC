#include "CoolingConfigDialog.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFrame>

// ─────────────────────────────────────────────────────────────
//  Helper: một hàng chỉnh giá trị với nút ▲/▼ lớn
// ─────────────────────────────────────────────────────────────
static QWidget *makeValueRow(QWidget *parent,
                              const QString &labelText,
                              double initVal,
                              double minVal,
                              double maxVal,
                              double step,
                              const QString &suffix,
                              QLabel **outValLabel,   // label hiển thị giá trị
                              double *outValuePtr)    // con trỏ giá trị thực
{
    auto *row = new QFrame(parent);
    row->setStyleSheet(
        "QFrame { background: #0f172a; border-radius: 10px; border: 1px solid #1e293b; }"
    );

    auto *hLayout = new QHBoxLayout(row);
    hLayout->setContentsMargins(12, 8, 12, 8);
    hLayout->setSpacing(10);

    // Tên field
    auto *nameLabel = new QLabel(labelText, row);
    nameLabel->setStyleSheet("font-size: 11px; font-weight: 700; color: #94a3b8;"
                              "background: transparent; border: none;");
    nameLabel->setFixedWidth(160);
    hLayout->addWidget(nameLabel);
    hLayout->addStretch();

    // Nút ▼
    auto *btnDown = new QPushButton(QStringLiteral("▼"), row);
    btnDown->setFixedSize(38, 38);
    btnDown->setCursor(Qt::PointingHandCursor);
    btnDown->setStyleSheet(
        "QPushButton { background: #1e293b; color: #94a3b8; border: 1px solid #334155;"
        "border-radius: 8px; font-size: 16px; font-weight: 900; }"
        "QPushButton:hover { background: #334155; color: #f8fafc; }"
        "QPushButton:pressed { background: #475569; }"
    );

    // Label giá trị
    auto *valLabel = new QLabel(QString::number(initVal, 'f', (step < 1.0 ? 1 : 0)) + suffix, row);
    valLabel->setAlignment(Qt::AlignCenter);
    valLabel->setFixedWidth(88);
    valLabel->setStyleSheet("font-size: 16px; font-weight: 900; color: #38bdf8;"
                             "background: transparent; border: none;");
    *outValLabel = valLabel;

    // Nút ▲
    auto *btnUp = new QPushButton(QStringLiteral("▲"), row);
    btnUp->setFixedSize(38, 38);
    btnUp->setCursor(Qt::PointingHandCursor);
    btnUp->setStyleSheet(
        "QPushButton { background: #1e293b; color: #38bdf8; border: 1px solid #334155;"
        "border-radius: 8px; font-size: 16px; font-weight: 900; }"
        "QPushButton:hover { background: #0284c7; color: #ffffff; }"
        "QPushButton:pressed { background: #0369a1; }"
    );

    hLayout->addWidget(btnDown);
    hLayout->addWidget(valLabel);
    hLayout->addWidget(btnUp);

    // Kết nối nút
    *outValuePtr = initVal;

    QObject::connect(btnDown, &QPushButton::clicked, row, [=]() mutable {
        double cur = *outValuePtr;
        cur -= step;
        if (cur < minVal) cur = minVal;
        *outValuePtr = cur;
        const int decimals = (step < 1.0) ? 1 : 0;
        (*outValLabel)->setText(QString::number(cur, 'f', decimals) + suffix);
    });

    QObject::connect(btnUp, &QPushButton::clicked, row, [=]() mutable {
        double cur = *outValuePtr;
        cur += step;
        if (cur > maxVal) cur = maxVal;
        *outValuePtr = cur;
        const int decimals = (step < 1.0) ? 1 : 0;
        (*outValLabel)->setText(QString::number(cur, 'f', decimals) + suffix);
    });

    return row;
}

// ─────────────────────────────────────────────────────────────
//  CoolingConfigDialog
// ─────────────────────────────────────────────────────────────
CoolingConfigDialog::CoolingConfigDialog(const QJsonObject &currentConfig, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Cấu Hình Hệ Thống Làm Mát"));
    setModal(true);
    setFixedSize(480, 390);
    setStyleSheet(
        "QDialog { background-color: #0b1329; color: #f8fafc; font-family: 'Noto Sans', sans-serif; } "
        "QPushButton { border-radius: 10px; font-weight: 700; font-size: 12px; padding: 10px 22px; } "
        "QPushButton#saveBtn  { background-color: #06b6d4; color: #0b1329; border: none; } "
        "QPushButton#saveBtn:hover  { background-color: #22d3ee; } "
        "QPushButton#cancelBtn { background-color: #334155; color: #f8fafc; border: none; } "
        "QPushButton#cancelBtn:hover { background-color: #475569; }"
    );

    m_fanStartTemp  = currentConfig.value(QStringLiteral("fan_start_temp")).toDouble(35.0);
    m_fanStopTemp   = currentConfig.value(QStringLiteral("fan_stop_temp")).toDouble(28.0);
    m_maxSoundVpp   = currentConfig.value(QStringLiteral("max_sound_vpp")).toDouble(1.5);
    m_intervalSec   = currentConfig.value(QStringLiteral("sampling_interval_seconds")).toDouble(2.0);

    setupUi();
}

void CoolingConfigDialog::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 18, 20, 18);
    mainLayout->setSpacing(12);

    // ── Header ──────────────────────────────────────────────
    auto *titleLabel = new QLabel(tr("⚙  Ngưỡng Tự Động Làm Mát"), this);
    titleLabel->setStyleSheet("font-size: 15px; font-weight: 900; color: #f8fafc;");
    auto *subLabel = new QLabel(tr("Nhấn ▲ / ▼ để điều chỉnh từng thông số"), this);
    subLabel->setStyleSheet("font-size: 10px; color: #64748b;");
    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(subLabel);

    // Separator
    auto *sep = new QFrame(this);
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("color: #1e293b;");
    mainLayout->addWidget(sep);

    // ── Các hàng điều chỉnh ─────────────────────────────────
    mainLayout->addWidget(makeValueRow(
        this,
        tr("Bật quạt khi nhiệt độ ≥"),
        m_fanStartTemp, 15.0, 70.0, 0.5, QStringLiteral(" °C"),
        &m_fanStartLabel, &m_fanStartTemp
    ));

    mainLayout->addWidget(makeValueRow(
        this,
        tr("Tắt quạt khi nhiệt độ ≤"),
        m_fanStopTemp, 10.0, 60.0, 0.5, QStringLiteral(" °C"),
        &m_fanStopLabel, &m_fanStopTemp
    ));

    mainLayout->addWidget(makeValueRow(
        this,
        tr("Ngưỡng cảnh báo độ ồn"),
        m_maxSoundVpp, 0.1, 5.0, 0.1, QStringLiteral(" Vpp"),
        &m_soundMaxLabel, &m_maxSoundVpp
    ));

    mainLayout->addWidget(makeValueRow(
        this,
        tr("Chu kỳ cập nhật mẫu"),
        m_intervalSec, 1.0, 60.0, 1.0, QStringLiteral(" giây"),
        &m_intervalLabel, &m_intervalSec
    ));

    mainLayout->addStretch();

    // ── Nút hành động ──────────────────────────────────────
    auto *btnLayout = new QHBoxLayout;
    btnLayout->addStretch();

    auto *cancelBtn = new QPushButton(tr("Hủy Bỏ"), this);
    cancelBtn->setObjectName(QStringLiteral("cancelBtn"));
    cancelBtn->setCursor(Qt::PointingHandCursor);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnLayout->addWidget(cancelBtn);

    auto *saveBtn = new QPushButton(tr("✔  Lưu Cấu Hình"), this);
    saveBtn->setObjectName(QStringLiteral("saveBtn"));
    saveBtn->setCursor(Qt::PointingHandCursor);
    connect(saveBtn, &QPushButton::clicked, this, [this] {
        // Đảm bảo fan_stop < fan_start
        if (m_fanStopTemp >= m_fanStartTemp) {
            m_fanStopTemp = m_fanStartTemp - 2.0;
            if (m_fanStopLabel)
                m_fanStopLabel->setText(QString::number(m_fanStopTemp, 'f', 1) + QStringLiteral(" °C"));
        }
        accept();
    });
    btnLayout->addWidget(saveBtn);

    mainLayout->addLayout(btnLayout);
}

QJsonObject CoolingConfigDialog::configData() const
{
    return QJsonObject{
        {QStringLiteral("fan_start_temp"),             m_fanStartTemp},
        {QStringLiteral("fan_stop_temp"),              m_fanStopTemp},
        {QStringLiteral("max_sound_vpp"),              m_maxSoundVpp},
        {QStringLiteral("sampling_interval_seconds"),  static_cast<int>(m_intervalSec)}
    };
}
