#include "SelectOnlineDeviceDialog.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

SelectOnlineDeviceDialog::SelectOnlineDeviceDialog(const QJsonArray &availableDevices, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("Chọn thiết bị Smart Lighting Online"));
    resize(460, 280);
    setStyleSheet(
        "QDialog { background-color: #0f0d1a; color: #fef3c7; font-family: sans-serif; } "
        "QLabel { color: #fef3c7; } "
        "QScrollArea { border: none; background: transparent; }"
    );

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(14, 12, 14, 12);
    mainLayout->setSpacing(8);

    // --- Header ---
    auto *headerRow = new QHBoxLayout;
    auto *titleIcon = new QLabel(QStringLiteral("💡"));
    titleIcon->setStyleSheet("font-size: 15px;");
    auto *titleLbl = new QLabel(QStringLiteral("Danh Sách Thiết Bị Online (TUANANH)"));
    titleLbl->setStyleSheet("color: #f59e0b; font-size: 13px; font-weight: 900;");
    headerRow->addWidget(titleIcon);
    headerRow->addWidget(titleLbl);
    headerRow->addStretch();

    auto *refreshBtn = new QPushButton(QStringLiteral("🔄 Quét lại"));
    refreshBtn->setCursor(Qt::PointingHandCursor);
    refreshBtn->setStyleSheet(
        "QPushButton { background: #1c1830; color: #f59e0b; border: 1px solid #3d3266; border-radius: 6px; padding: 4px 10px; font-size: 10px; font-weight: 800; } "
        "QPushButton:hover { background: #282245; color: #ffffff; }"
    );
    connect(refreshBtn, &QPushButton::clicked, this, [this] {
        if (m_emptyLabel) m_emptyLabel->setText(QStringLiteral("Đang quét thiết bị ESP32 Smart Light..."));
        emit refreshRequested();
    });
    headerRow->addWidget(refreshBtn);

    auto *closeBtn = new QPushButton(QStringLiteral("✕"));
    closeBtn->setFixedSize(22, 22);
    closeBtn->setStyleSheet("background: #24141e; color: #ef4444; border: 1px solid #4a1d2e; border-radius: 11px; font-weight: 900; font-size: 10px;");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);
    headerRow->addWidget(closeBtn);
    mainLayout->addLayout(headerRow);

    // --- Scroll Area ---
    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet("background: transparent; border: none;");

    auto *container = new QWidget;
    container->setStyleSheet("background: transparent;");
    m_listLayout = new QVBoxLayout(container);
    m_listLayout->setContentsMargins(0, 0, 0, 0);
    m_listLayout->setSpacing(6);

    m_emptyLabel = new QLabel(QStringLiteral("Chưa phát hiện thiết bị online nào.\nVui lòng bật nguồn ESP32 Tuấn Anh (150808) và kiểm tra WiFi/MQTT."));
    m_emptyLabel->setStyleSheet("color: #94a3b8; font-size: 11px; font-style: italic; padding: 24px;");
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_listLayout->addWidget(m_emptyLabel);

    scroll->setWidget(container);
    mainLayout->addWidget(scroll, 1);

    populateDeviceList(availableDevices);
}

void SelectOnlineDeviceDialog::updateAvailableDevices(const QJsonArray &availableDevices)
{
    populateDeviceList(availableDevices);
}

void SelectOnlineDeviceDialog::populateDeviceList(const QJsonArray &devices)
{
    // Clear old items
    QLayoutItem *item;
    while ((item = m_listLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            if (item->widget() != m_emptyLabel)
                delete item->widget();
        }
        delete item;
    }

    int count = 0;
    for (const auto &val : devices) {
        const auto dev = val.toObject();
        const QString devId = dev.value(QStringLiteral("device_id")).toString();
        const QString name = dev.value(QStringLiteral("name")).toString(devId);
        const QString fwVer = dev.value(QStringLiteral("firmware_version")).toString(QStringLiteral("1.0.0"));
        const bool isOnline = dev.value(QStringLiteral("is_online")).toBool(true);

        // Filter for TUANANH firmware ID: 150808 or Tuananh-150808
        if (devId.compare(QStringLiteral("150808"), Qt::CaseInsensitive) != 0 &&
            devId.compare(QStringLiteral("Tuananh-150808"), Qt::CaseInsensitive) != 0) {
            continue;
        }

        count++;
        auto *card = new QFrame;
        card->setStyleSheet(
            "QFrame { "
            "  background-color: #19142b; "
            "  border: 1px solid #382c5c; "
            "  border-radius: 8px; "
            "} "
            "QFrame:hover { "
            "  border-color: #f59e0b; "
            "  background-color: #241c3d; "
            "}"
        );

        auto *cardLayout = new QHBoxLayout(card);
        cardLayout->setContentsMargins(8, 6, 8, 6);
        cardLayout->setSpacing(8);

        auto *iconLbl = new QLabel(QStringLiteral("🔆"));
        iconLbl->setStyleSheet("font-size: 16px; border: none; background: transparent;");
        cardLayout->addWidget(iconLbl);

        auto *infoCol = new QVBoxLayout;
        infoCol->setSpacing(1);

        auto *titleRow = new QHBoxLayout;
        auto *nameLbl = new QLabel(name.isEmpty() ? devId : name);
        nameLbl->setStyleSheet("color: #ffffff; font-size: 11px; font-weight: 800; border: none; background: transparent;");
        auto *onlineBadge = new QLabel(isOnline ? QStringLiteral("🟢 ONLINE") : QStringLiteral("🔴 OFFLINE"));
        onlineBadge->setStyleSheet(isOnline
            ? "color: #10b981; font-size: 8px; font-weight: 900; background: rgba(16, 185, 129, 0.15); border-radius: 3px; padding: 1px 4px;"
            : "color: #ef4444; font-size: 8px; font-weight: 900; background: rgba(239, 68, 68, 0.15); border-radius: 3px; padding: 1px 4px;");
        titleRow->addWidget(nameLbl);
        titleRow->addWidget(onlineBadge);
        titleRow->addStretch();
        infoCol->addLayout(titleRow);

        auto *subInfo = new QLabel(QStringLiteral("ID: <b style='color: #f59e0b;'>%1</b> | FW: v%2").arg(devId, fwVer));
        subInfo->setStyleSheet("color: #94a3b8; font-size: 9px; border: none; background: transparent;");
        infoCol->addWidget(subInfo);

        cardLayout->addLayout(infoCol, 1);

        auto *selectBtn = new QPushButton(QStringLiteral("＋ Thêm"));
        selectBtn->setCursor(Qt::PointingHandCursor);
        selectBtn->setFixedSize(62, 26);
        selectBtn->setStyleSheet(
            "QPushButton { background: #f59e0b; color: #000000; border: none; border-radius: 5px; font-size: 10px; font-weight: 900; } "
            "QPushButton:hover { background: #fbbf24; } "
            "QPushButton:pressed { background: #d97706; }"
        );
        connect(selectBtn, &QPushButton::clicked, this, [this, devId, name] {
            emit deviceSelected(devId, name.isEmpty() ? devId : name);
            accept();
        });
        cardLayout->addWidget(selectBtn);

        m_listLayout->addWidget(card);
    }

    if (count == 0) {
        m_emptyLabel->show();
        m_listLayout->addWidget(m_emptyLabel);
    } else {
        m_emptyLabel->hide();
    }
}
