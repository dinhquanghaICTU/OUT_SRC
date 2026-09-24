#include "CoolingConfigDialog.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFrame>

// ─────────────────────────────────────────────────────────────
//  Helper: card 1 field - label trên, [−] value [+] dưới
// ─────────────────────────────────────────────────────────────
static QFrame *makeFieldCard(QWidget *parent,
                              const QString &labelText,
                              double initVal,
                              double minVal,
                              double maxVal,
                              double step,
                              const QString &suffix,
                              QLabel **outValLabel,
                              double *outValue)
{
    *outValue = initVal;

    auto *card = new QFrame(parent);
    card->setStyleSheet(
        "QFrame { background: #1e293b; border-radius: 10px; border: 1px solid #334155; }"
    );

    auto *vlay = new QVBoxLayout(card);
    vlay->setContentsMargins(10, 8, 10, 8);
    vlay->setSpacing(6);

    // Tên field
    auto *nameLabel = new QLabel(labelText, card);
    nameLabel->setAlignment(Qt::AlignCenter);
    nameLabel->setStyleSheet(
        "font-size: 10px; font-weight: 700; color: #94a3b8;"
        "background: transparent; border: none;"
    );
    nameLabel->setWordWrap(true);
    vlay->addWidget(nameLabel);

    // Hàng điều chỉnh: [−] [value] [+]
    auto *hlay = new QHBoxLayout;
    hlay->setSpacing(6);
    hlay->setContentsMargins(0, 0, 0, 0);

    const QString btnStyle =
        "QPushButton { background: #0f172a; color: #38bdf8; border: 1px solid #334155;"
        "border-radius: 8px; font-size: 18px; font-weight: 900; min-width: 36px; min-height: 36px; }"
        "QPushButton:hover  { background: #0284c7; color: #ffffff; border-color: #0284c7; }"
        "QPushButton:pressed { background: #0369a1; }";

    auto *btnMinus = new QPushButton(QStringLiteral("−"), card);
    btnMinus->setFixedSize(36, 36);
    btnMinus->setCursor(Qt::PointingHandCursor);
    btnMinus->setStyleSheet(btnStyle);

    const int decimals = (step < 1.0) ? 1 : 0;
    auto *valLabel = new QLabel(QString::number(initVal, 'f', decimals) + suffix, card);
    valLabel->setAlignment(Qt::AlignCenter);
    valLabel->setFixedWidth(90);
    valLabel->setStyleSheet(
        "font-size: 15px; font-weight: 900; color: #38bdf8;"
        "background: transparent; border: none;"
    );
    *outValLabel = valLabel;

    auto *btnPlus = new QPushButton(QStringLiteral("+"), card);
    btnPlus->setFixedSize(36, 36);
    btnPlus->setCursor(Qt::PointingHandCursor);
    btnPlus->setStyleSheet(btnStyle);

    hlay->addStretch();
    hlay->addWidget(btnMinus);
    hlay->addWidget(valLabel);
    hlay->addWidget(btnPlus);
    hlay->addStretch();
    vlay->addLayout(hlay);

    // Connect buttons
    QObject::connect(btnMinus, &QPushButton::clicked, card, [=]() mutable {
        double cur = *outValue - step;
        if (cur < minVal) cur = minVal;
        *outValue = cur;
        (*outValLabel)->setText(QString::number(cur, 'f', decimals) + suffix);
    });
    QObject::connect(btnPlus, &QPushButton::clicked, card, [=]() mutable {
        double cur = *outValue + step;
        if (cur > maxVal) cur = maxVal;
        *outValue = cur;
        (*outValLabel)->setText(QString::number(cur, 'f', decimals) + suffix);
    });

    return card;
}

// ─────────────────────────────────────────────────────────────
//  CoolingConfigDialog
// ─────────────────────────────────────────────────────────────
CoolingConfigDialog::CoolingConfigDialog(const QJsonObject &currentConfig, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Cấu Hình Hệ Thống Làm Mát"));
    setModal(true);
    setFixedSize(500, 360);
    setStyleSheet(
        "QDialog { background-color: #0b1329; color: #f8fafc;"
        "font-family: 'Noto Sans', sans-serif; } "
        "QPushButton#saveBtn   { background: #06b6d4; color: #0b1329; border: none;"
        "border-radius: 10px; font-weight: 800; font-size: 12px; padding: 9px 24px; } "
        "QPushButton#saveBtn:hover   { background: #22d3ee; } "
        "QPushButton#cancelBtn { background: #334155; color: #f8fafc; border: none;"
        "border-radius: 10px; font-weight: 700; font-size: 12px; padding: 9px 24px; } "
        "QPushButton#cancelBtn:hover { background: #475569; }"
    );

    m_fanStartTemp = currentConfig.value(QStringLiteral("fan_start_temp")).toDouble(35.0);
    m_fanStopTemp  = currentConfig.value(QStringLiteral("fan_stop_temp")).toDouble(28.0);
    m_maxSoundVpp  = currentConfig.value(QStringLiteral("max_sound_vpp")).toDouble(1.5);
    m_intervalSec  = currentConfig.value(QStringLiteral("sampling_interval_seconds")).toDouble(2.0);

    setupUi();
}

void CoolingConfigDialog::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(18, 14, 18, 14);
    mainLayout->setSpacing(10);

    // ── Header ──────────────────────────────────────────────
    auto *titleLabel = new QLabel(tr("⚙  Ngưỡng Tự Động Làm Mát"), this);
    titleLabel->setStyleSheet(
        "font-size: 14px; font-weight: 900; color: #f8fafc;"
        "background: transparent;"
    );
    auto *subLabel = new QLabel(tr("Nhấn  −  /  +  để điều chỉnh từng thông số"), this);
    subLabel->setStyleSheet(
        "font-size: 10px; color: #64748b; background: transparent;"
    );
    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(subLabel);

    // Separator
    auto *sep = new QFrame(this);
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("QFrame { color: #1e293b; border: none;"
                       "border-top: 1px solid #1e293b; background: transparent; }");
    mainLayout->addWidget(sep);

    // ── Grid 2×2 các field ──────────────────────────────────
    // Hàng 1: bật quạt | tắt quạt
    auto *row1 = new QHBoxLayout;
    row1->setSpacing(10);
    row1->addWidget(makeFieldCard(this,
        tr("Bật quạt khi\nnhiệt độ ≥"),
        m_fanStartTemp, 15.0, 70.0, 0.5, QStringLiteral(" °C"),
        &m_fanStartLabel, &m_fanStartTemp));
    row1->addWidget(makeFieldCard(this,
        tr("Tắt quạt khi\nnhiệt độ ≤"),
        m_fanStopTemp, 10.0, 60.0, 0.5, QStringLiteral(" °C"),
        &m_fanStopLabel, &m_fanStopTemp));
    mainLayout->addLayout(row1);

    // Hàng 2: độ ồn | chu kỳ
    auto *row2 = new QHBoxLayout;
    row2->setSpacing(10);
    row2->addWidget(makeFieldCard(this,
        tr("Ngưỡng cảnh báo\nđộ ồn quạt"),
        m_maxSoundVpp, 0.1, 5.0, 0.1, QStringLiteral(" Vpp"),
        &m_soundMaxLabel, &m_maxSoundVpp));
    row2->addWidget(makeFieldCard(this,
        tr("Chu kỳ cập nhật\nmẫu dữ liệu"),
        m_intervalSec, 1.0, 60.0, 1.0, QStringLiteral(" giây"),
        &m_intervalLabel, &m_intervalSec));
    mainLayout->addLayout(row2);

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
                m_fanStopLabel->setText(
                    QString::number(m_fanStopTemp, 'f', 1) + QStringLiteral(" °C"));
        }
        accept();
    });
    btnLayout->addWidget(saveBtn);

    mainLayout->addLayout(btnLayout);
}

QJsonObject CoolingConfigDialog::configData() const
{
    return QJsonObject{
        {QStringLiteral("fan_start_temp"),            m_fanStartTemp},
        {QStringLiteral("fan_stop_temp"),             m_fanStopTemp},
        {QStringLiteral("max_sound_vpp"),             m_maxSoundVpp},
        {QStringLiteral("sampling_interval_seconds"), static_cast<int>(m_intervalSec)}
    };
}
