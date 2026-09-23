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
    setWindowTitle(tr("Chọn Thiết Bị ESP32"));
    resize(500, 310);
    setStyleSheet(
        "QDialog { background-color: #070d1e; color: #ecf2ff; font-family: sans-serif; } "
        "QLabel { color: #f1f5f9; } "
        "QScrollArea { border: none; background: transparent; }"
    );

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 14, 16, 14);
    mainLayout->setSpacing(10);

    // --- Header ---
    auto *headerRow = new QHBoxLayout;
    headerRow->setContentsMargins(0, 0, 0, 0);
    headerRow->setSpacing(8);

    auto *headerTextLayout = new QVBoxLayout;
    headerTextLayout->setContentsMargins(0, 0, 0, 0);
    headerTextLayout->setSpacing(2);

    auto *titleLbl = new QLabel(tr("Thiết Bị ESP32 Trực Tuyến"), this);
    titleLbl->setStyleSheet("color: #38bdf8; font-size: 13px; font-weight: 900; background: transparent; border: none;");
    auto *subtitleLbl = new QLabel(tr("Các trạm cảm biến & bơm đang phát sóng trong mạng nội bộ:"), this);
    subtitleLbl->setStyleSheet("color: #94a3b8; font-size: 10px; font-weight: 600; background: transparent; border: none;");

    headerTextLayout->addWidget(titleLbl);
    headerTextLayout->addWidget(subtitleLbl);
    headerRow->addLayout(headerTextLayout, 1);

    auto *refreshBtn = new QPushButton(tr("Làm mới"), this);
    refreshBtn->setCursor(Qt::PointingHandCursor);
    refreshBtn->setStyleSheet(
        "QPushButton { background: #132247; color: #38bdf8; border: 1px solid #234380; border-radius: 6px; padding: 5px 12px; font-size: 10px; font-weight: 800; } "
        "QPushButton:hover { background: #0284c7; color: #ffffff; border-color: #38bdf8; } "
        "QPushButton:pressed { background: #0369a1; }"
    );
    connect(refreshBtn, &QPushButton::clicked, this, [this] {
        if (m_emptyLabel) {
            m_emptyLabel->setText(tr("Đang quét thiết bị trực tuyến..."));
            m_emptyLabel->show();
        }
        emit refreshRequested();
    });
    headerRow->addWidget(refreshBtn);
    headerRow->addSpacing(4);

    auto *closeBtn = new QPushButton(QStringLiteral("Đóng"), this);
    closeBtn->setFixedSize(26, 26);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(
        "QPushButton { background: #132247; color: #94a3b8; border: 1px solid #234380; border-radius: 13px; font-weight: 900; font-size: 11px; } "
        "QPushButton:hover { background: #ef4444; color: #ffffff; border-color: #ef4444; }"
    );
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);
    headerRow->addWidget(closeBtn);
    mainLayout->addLayout(headerRow);

    // --- Scroll Area for Device Cards ---
    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setStyleSheet("background: transparent; border: none;");

    auto *container = new QWidget(scroll);
    container->setStyleSheet("background: transparent; border: none;");
    m_listLayout = new QVBoxLayout(container);
    m_listLayout->setContentsMargins(0, 4, 0, 4);
    m_listLayout->setSpacing(8);

    m_emptyLabel = new QLabel(tr("Chưa phát hiện thiết bị online nào.\nVui lòng bật nguồn ESP32 (son-190782) và bấm 'Làm mới'."), container);
    m_emptyLabel->setStyleSheet("color: #94a3b8; font-size: 11px; font-style: italic; padding: 28px; background: transparent; border: none;");
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
    // Clear old items except m_emptyLabel
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
        const bool isOnline = dev.value(QStringLiteral("online")).toBool(dev.value(QStringLiteral("is_online")).toBool(true));

        // Strict filter for Son devices
        if (devId.compare(QStringLiteral("son-190782"), Qt::CaseInsensitive) != 0 &&
            devId.compare(QStringLiteral("150304"), Qt::CaseInsensitive) != 0) {
            continue;
        }

        count++;
        auto *card = new QFrame;
        card->setObjectName(QStringLiteral("onlineDeviceCard"));
        card->setFixedHeight(66);
        card->setStyleSheet(
            "QFrame#onlineDeviceCard { "
            "  background-color: #0d1733; "
            "  border: 1.5px solid #1c2b54; "
            "  border-radius: 8px; "
            "} "
            "QFrame#onlineDeviceCard:hover { "
            "  border-color: #10b981; "
            "  background-color: #111f44; "
            "}"
        );

        auto *cardLayout = new QHBoxLayout(card);
        cardLayout->setContentsMargins(12, 8, 12, 8);
        cardLayout->setSpacing(12);

        // Chip Avatar Icon Container
        auto *avatarBox = new QFrame(card);
        avatarBox->setFixedSize(36, 36);
        avatarBox->setStyleSheet(
            "background-color: #162a56; "
            "border: 1px solid #234380; "
            "border-radius: 6px;"
        );
        auto *avatarLayout = new QVBoxLayout(avatarBox);
        avatarLayout->setContentsMargins(0, 0, 0, 0);
        auto *avatarText = new QLabel(QStringLiteral("ESP"), avatarBox);
        avatarText->setAlignment(Qt::AlignCenter);
        avatarText->setStyleSheet("color: #38bdf8; font-size: 11px; font-weight: 900; background: transparent; border: none;");
        avatarLayout->addWidget(avatarText);
        cardLayout->addWidget(avatarBox, 0, Qt::AlignVCenter);

        // Center Info
        auto *infoCol = new QVBoxLayout;
        infoCol->setContentsMargins(0, 0, 0, 0);
        infoCol->setSpacing(2);

        auto *titleRow = new QHBoxLayout;
        titleRow->setContentsMargins(0, 0, 0, 0);
        titleRow->setSpacing(8);

        QString displayName = name;
        if (displayName.isEmpty() || displayName == devId) {
            displayName = QStringLiteral("Trạm Bơm & Mực Nước");
        }
        auto *nameLbl = new QLabel(displayName, card);
        nameLbl->setStyleSheet("color: #ffffff; font-size: 12px; font-weight: 800; border: none; background: transparent;");
        titleRow->addWidget(nameLbl);

        auto *onlineBadge = new QLabel(isOnline ? tr("● Trực tuyến") : tr("Ngoại tuyến"), card);
        onlineBadge->setFixedHeight(18);
        onlineBadge->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        onlineBadge->setAlignment(Qt::AlignCenter);
        onlineBadge->setStyleSheet(isOnline
            ? "color: #10b981; font-size: 9px; font-weight: 800; background: rgba(16, 185, 129, 0.15); border: 1px solid rgba(16, 185, 129, 0.4); border-radius: 9px; padding: 1px 7px;"
            : "color: #ef4444; font-size: 9px; font-weight: 800; background: rgba(239, 68, 68, 0.15); border: 1px solid rgba(239, 68, 68, 0.4); border-radius: 9px; padding: 1px 7px;");
        titleRow->addWidget(onlineBadge);
        titleRow->addStretch();
        infoCol->addLayout(titleRow);

        auto *subInfo = new QLabel(QStringLiteral("ID: <b style='color: #38bdf8;'>%1</b> · Firmware: v%2 · Loại: Cảm biến & Máy bơm").arg(devId, fwVer), card);
        subInfo->setStyleSheet("color: #94a3b8; font-size: 9px; border: none; background: transparent;");
        infoCol->addWidget(subInfo);

        cardLayout->addLayout(infoCol, 1);

        auto *selectBtn = new QPushButton(tr("+ Thêm"), card);
        selectBtn->setCursor(Qt::PointingHandCursor);
        selectBtn->setFixedSize(76, 28);
        selectBtn->setStyleSheet(
            "QPushButton { background: #10b981; color: #ffffff; border: none; border-radius: 6px; font-size: 11px; font-weight: 900; } "
            "QPushButton:hover { background: #059669; } "
            "QPushButton:pressed { background: #047857; }"
        );
        connect(selectBtn, &QPushButton::clicked, this, [this, devId, name] {
            emit deviceSelected(devId, name.isEmpty() ? devId : name);
            accept();
        });
        cardLayout->addWidget(selectBtn, 0, Qt::AlignVCenter);

        m_listLayout->addWidget(card);
    }

    if (count == 0) {
        m_emptyLabel->setText(tr("Chưa phát hiện thiết bị online nào.\nVui lòng bật nguồn ESP32 (son-190782) và nhấn 'Làm mới'."));
        m_emptyLabel->show();
        m_listLayout->addWidget(m_emptyLabel);
    } else {
        m_emptyLabel->hide();
    }
}
