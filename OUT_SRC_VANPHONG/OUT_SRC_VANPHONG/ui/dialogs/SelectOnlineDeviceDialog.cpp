#include "SelectOnlineDeviceDialog.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QJsonObject>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

SelectOnlineDeviceDialog::SelectOnlineDeviceDialog(const QJsonArray &devices, QWidget *parent)
    : QDialog(parent), m_devices(devices)
{
    setWindowTitle(tr("Chọn thiết bị tưới tự động"));
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setModal(true);

    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(12, 12, 12, 12);

    auto *card = new QFrame(this);
    card->setObjectName(QStringLiteral("selectDevCard"));
    card->setStyleSheet(QStringLiteral(
        "QFrame#selectDevCard { background-color: #081a11; border: 1.5px solid #1b4332; border-radius: 12px; }"));

    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(18, 18, 18, 18);
    layout->setSpacing(12);

    auto *title = new QLabel(tr("🌿 Thiết bị Tưới Cây Thông Minh trực tuyến"), card);
    title->setStyleSheet(QStringLiteral("color: #34d399; font-size: 14px; font-weight: 700;"));
    layout->addWidget(title);

    m_listWidget = new QListWidget(card);
    m_listWidget->setStyleSheet(QStringLiteral(
        "QListWidget { background-color: #0e291b; border: 1px solid #1b4332; border-radius: 8px; color: #f1f5f9; padding: 6px; } "
        "QListWidget::item { padding: 8px; border-radius: 6px; } "
        "QListWidget::item:selected { background-color: #059669; color: white; }"));

    for (const auto &val : devices) {
        const QJsonObject dev = val.toObject();
        const QString id = dev.value(QStringLiteral("device_id")).toString();
        const QString name = dev.value(QStringLiteral("name")).toString(id);
        const bool online = dev.value(QStringLiteral("online")).toBool(true);
        auto *item = new QListWidgetItem(QStringLiteral("%1  [%2] - %3").arg(
            online ? QStringLiteral("🟢") : QStringLiteral("⚪"), id, name));
        item->setData(Qt::UserRole, id);
        item->setData(Qt::UserRole + 1, name);
        m_listWidget->addItem(item);
    }
    if (m_listWidget->count() > 0)
        m_listWidget->setCurrentRow(0);

    layout->addWidget(m_listWidget);

    auto *btnLayout = new QHBoxLayout;
    btnLayout->addStretch();

    m_cancelBtn = new QPushButton(tr("Hủy"), card);
    m_cancelBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background-color: #334155; color: #cbd5e1; border: none; border-radius: 6px; padding: 6px 16px; font-weight: 600; }"));
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnLayout->addWidget(m_cancelBtn);

    m_selectBtn = new QPushButton(tr("Chọn thiết bị"), card);
    m_selectBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background-color: #10b981; color: white; border: none; border-radius: 6px; padding: 6px 16px; font-weight: 600; }"));
    connect(m_selectBtn, &QPushButton::clicked, this, [this] {
        auto *item = m_listWidget->currentItem();
        if (item) {
            m_selectedId = item->data(Qt::UserRole).toString();
            m_selectedName = item->data(Qt::UserRole + 1).toString();
            accept();
        }
    });
    btnLayout->addWidget(m_selectBtn);

    layout->addLayout(btnLayout);
    rootLayout->addWidget(card);
    resize(420, 300);
}

QString SelectOnlineDeviceDialog::selectedDeviceId() const
{
    return m_selectedId;
}

QString SelectOnlineDeviceDialog::selectedDeviceName() const
{
    return m_selectedName;
}
