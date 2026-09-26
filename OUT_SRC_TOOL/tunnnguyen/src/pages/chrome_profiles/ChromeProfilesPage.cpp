#include "ChromeProfilesPage.h"
#include "core/ChromeLauncher.h"
#include "core/SyncManager.h"
#include "core/ProxyConfig.h"
#include "ui/CreateProfileDialog.h"
#include "ui/EditProfileDialog.h"
#include "ui/ImportProfilesDialog.h"
#include "ui/QuickProxyDialog.h"
#include "ui/CustomMessageBox.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDesktopServices>
#include <QUrl>
#include <QTimer>
#include <QScreen>
#include <QGuiApplication>
#include <QListView>
#include <QSpinBox>
#include <QCheckBox>
#include <QDateTime>

// Cell widget for profile name with hover buttons
class ProfileNameCellWidget : public QWidget {
public:
    ProfileNameCellWidget(int index, const ChromeProfileItem &item,
                          std::function<void(int)> onEditConfig,
                          std::function<void(int)> onEditProxy,
                          QWidget *parent = nullptr)
        : QWidget(parent), m_index(index), m_onEditConfig(onEditConfig), m_onEditProxy(onEditProxy)
    {
        setAttribute(Qt::WA_Hover, true);
        auto *layout = new QHBoxLayout(this);
        layout->setContentsMargins(10, 4, 10, 4);
        layout->setSpacing(8);

        auto *iconLbl = new QLabel(EditProfileDialog::getDeviceIcon(item.deviceType));
        iconLbl->setToolTip(QString("Thiết bị: %1").arg(EditProfileDialog::getDeviceDisplayName(item.deviceType)));
        iconLbl->setStyleSheet("font-size: 15px;");
        layout->addWidget(iconLbl);

        auto *nameLbl = new QLabel(item.name);
        nameLbl->setFont(QFont("Google Sans", 10, QFont::Bold));
        nameLbl->setStyleSheet("color: #0f172a;");
        layout->addWidget(nameLbl);

        layout->addSpacing(6);

        m_hoverNav = new QWidget(this);
        auto *navLayout = new QHBoxLayout(m_hoverNav);
        navLayout->setContentsMargins(0, 0, 0, 0);
        navLayout->setSpacing(5);

        auto *btnConfig = new QPushButton("⚙️ Cấu hình");
        btnConfig->setCursor(Qt::PointingHandCursor);
        btnConfig->setFixedHeight(26);
        btnConfig->setStyleSheet(
            "QPushButton { background: #f8fafc; color: #475569; border: 1px solid #cbd5e1; "
            "border-radius: 6px; padding: 2px 8px; font-size: 11px; font-weight: 600; } "
            "QPushButton:hover { background: #f97316; color: #ffffff; border-color: #ea580c; }");
        connect(btnConfig, &QPushButton::clicked, [this]() {
            if (m_onEditConfig) m_onEditConfig(m_index);
        });

        auto *btnProxy = new QPushButton("🌐 Sửa Proxy");
        btnProxy->setCursor(Qt::PointingHandCursor);
        btnProxy->setFixedHeight(26);
        btnProxy->setStyleSheet(
            "QPushButton { background: #eff6ff; color: #2563eb; border: 1px solid #bfdbfe; "
            "border-radius: 6px; padding: 2px 8px; font-size: 11px; font-weight: 600; } "
            "QPushButton:hover { background: #2563eb; color: #ffffff; border-color: #1d4ed8; }");
        connect(btnProxy, &QPushButton::clicked, [this]() {
            if (m_onEditProxy) m_onEditProxy(m_index);
        });

        navLayout->addWidget(btnConfig);
        navLayout->addWidget(btnProxy);
        m_hoverNav->setVisible(false);

        layout->addWidget(m_hoverNav);
        layout->addStretch();
    }

protected:
    void enterEvent(QEnterEvent *event) override {
        QWidget::enterEvent(event);
        if (m_hoverNav) m_hoverNav->setVisible(true);
    }
    void leaveEvent(QEvent *event) override {
        QWidget::leaveEvent(event);
        if (m_hoverNav) m_hoverNav->setVisible(false);
    }

private:
    int m_index;
    std::function<void(int)> m_onEditConfig;
    std::function<void(int)> m_onEditProxy;
    QWidget *m_hoverNav = nullptr;
};

ChromeProfilesPage::ChromeProfilesPage(ChromeLauncher *launcher, SyncManager *syncManager, QWidget *parent)
    : QWidget(parent), m_launcher(launcher), m_syncManager(syncManager)
{
    setupUi();
    loadProfiles();
}

static QString getProfilesStorageDir() {
    QString dir = QDir::homePath() + "/.tunnbit_profiles";
    QDir().mkpath(dir);
    return dir;
}

void ChromeProfilesPage::setupUi() {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 20, 24, 20);
    layout->setSpacing(16);

    // 1. Header Row
    auto *headerCard = new QWidget();
    headerCard->setStyleSheet("background: #f8fafc; border: 1px solid #f1f5f9; border-radius: 12px; padding: 12px 16px;");
    auto *headerLayout = new QHBoxLayout(headerCard);
    headerLayout->setContentsMargins(0, 0, 0, 0);

    auto *headerIcon = new QLabel("👥");
    headerIcon->setStyleSheet("font-size: 24px; background: #ffedd5; border: 1px solid #fed7aa; border-radius: 12px; padding: 6px 10px;");
    auto *titleBox = new QVBoxLayout();
    titleBox->setSpacing(2);
    auto *titleText = new QLabel("Quản Lý Chrome Profiles & Môi Trường Nuôi Nick");
    titleText->setStyleSheet("font-size: 16px; font-weight: 800; color: #0f172a;");
    auto *subtitleText = new QLabel("Khởi tạo Browser Profiles cô lập, gán Proxy riêng biệt, Remote Debugging Port (CDP) chống Checkpoint.");
    subtitleText->setStyleSheet("font-size: 12px; color: #64748b; font-weight: 500;");
    titleBox->addWidget(titleText);
    titleBox->addWidget(subtitleText);

    headerLayout->addWidget(headerIcon);
    headerLayout->addLayout(titleBox);
    headerLayout->addStretch();

    // Stats
    auto *statsLayout = new QHBoxLayout();
    statsLayout->setSpacing(12);

    auto makeStatBadge = [](const QString &label, QLabel *&valLbl, const QString &val, const QString &color) -> QWidget* {
        auto *card = new QWidget();
        card->setStyleSheet("background: #ffffff; border: 1px solid #e2e8f0; border-radius: 8px; padding: 6px 14px;");
        auto *l = new QVBoxLayout(card);
        l->setContentsMargins(0, 0, 0, 0);
        l->setSpacing(1);
        auto *sub = new QLabel(label);
        sub->setStyleSheet("font-size: 11px; color: #94a3b8; font-weight: 600; text-transform: uppercase;");
        valLbl = new QLabel(val);
        valLbl->setStyleSheet(QString("font-size: 14px; font-weight: 800; color: %1;").arg(color));
        l->addWidget(sub);
        l->addWidget(valLbl);
        return card;
    };

    statsLayout->addWidget(makeStatBadge("Tổng Profiles", m_lblTotalProfiles, "0", "#0f172a"));
    statsLayout->addWidget(makeStatBadge("Đang chạy", m_lblActiveProfiles, "0 active", "#16a34a"));
    statsLayout->addWidget(makeStatBadge("Đã gán Proxy", m_lblProxyProfiles, "0", "#f97316"));
    headerLayout->addLayout(statsLayout);
    layout->addWidget(headerCard);

    // 2. Action Toolbar Row 1
    auto *toolRow1 = new QHBoxLayout();
    toolRow1->setSpacing(10);

    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("🔍 Tìm kiếm theo Tên Profile, Proxy, Remote Port...");
    m_searchEdit->setClearButtonEnabled(true);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &ChromeProfilesPage::onProfileSearchFilterChanged);
    toolRow1->addWidget(m_searchEdit, 1);

    auto *btnNewProfile = new QPushButton("➕ Tạo Profile");
    btnNewProfile->setCursor(Qt::PointingHandCursor);
    btnNewProfile->setStyleSheet("QPushButton { background: #f97316; color: #ffffff; font-weight: 700; border: none; border-radius: 8px; padding: 9px 16px; font-size: 13px; } QPushButton:hover { background: #ea580c; }");
    connect(btnNewProfile, &QPushButton::clicked, this, &ChromeProfilesPage::onCreateProfileClicked);

    auto *btnImport = new QPushButton("📥 Import Excel");
    btnImport->setCursor(Qt::PointingHandCursor);
    btnImport->setStyleSheet("QPushButton { background: #2563eb; color: #ffffff; font-weight: 700; border: none; border-radius: 8px; padding: 9px 14px; font-size: 13px; } QPushButton:hover { background: #1d4ed8; }");
    connect(btnImport, &QPushButton::clicked, this, &ChromeProfilesPage::onImportProfilesClicked);

    auto *btnTemplate = new QPushButton("📋 File Mẫu");
    btnTemplate->setCursor(Qt::PointingHandCursor);
    btnTemplate->setStyleSheet("QPushButton { background: #f0fdf4; color: #16a34a; font-weight: 600; border: 1px solid #bbf7d0; border-radius: 8px; padding: 9px 12px; font-size: 13px; } QPushButton:hover { background: #dcfce7; }");
    connect(btnTemplate, &QPushButton::clicked, this, &ChromeProfilesPage::onExportProfilesTemplateClicked);

    auto *btnOpenDir = new QPushButton("📂 Thư Mục Profile");
    btnOpenDir->setCursor(Qt::PointingHandCursor);
    btnOpenDir->setStyleSheet("QPushButton { background: #ffffff; color: #334155; font-weight: 600; border: 1px solid #e2e8f0; border-radius: 8px; padding: 9px 12px; font-size: 13px; } QPushButton:hover { background: #f8fafc; }");
    connect(btnOpenDir, &QPushButton::clicked, this, &ChromeProfilesPage::onOpenProfilesFolderClicked);

    m_btnToggleSyncBar = new QPushButton("⚡ Đồng Bộ Thao Tác");
    m_btnToggleSyncBar->setCursor(Qt::PointingHandCursor);
    m_btnToggleSyncBar->setStyleSheet("QPushButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #8b5cf6, stop:1 #6366f1); color: #ffffff; font-weight: 700; border: none; border-radius: 8px; padding: 9px 14px; font-size: 13px; } QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #7c3aed, stop:1 #4f46e5); }");
    connect(m_btnToggleSyncBar, &QPushButton::clicked, this, &ChromeProfilesPage::onToggleSyncBarClicked);

    toolRow1->addWidget(btnNewProfile);
    toolRow1->addWidget(btnImport);
    toolRow1->addWidget(btnTemplate);
    toolRow1->addWidget(btnOpenDir);
    toolRow1->addWidget(m_btnToggleSyncBar);
    layout->addLayout(toolRow1);

    // Row 2: Batch controls
    auto *toolRow2 = new QHBoxLayout();
    toolRow2->setSpacing(10);

    auto *btnToggleSelect = new QPushButton("☑ Chọn Hết / Bỏ");
    btnToggleSelect->setCursor(Qt::PointingHandCursor);
    btnToggleSelect->setStyleSheet("QPushButton { background: #ffffff; color: #334155; font-weight: 600; border: 1px solid #cbd5e1; border-radius: 8px; padding: 8px 12px; font-size: 12.5px; } QPushButton:hover { background: #f8fafc; }");
    connect(btnToggleSelect, &QPushButton::clicked, this, &ChromeProfilesPage::onToggleSelectAllProfiles);

    auto *btnLaunchSelected = new QPushButton("🚀 Mở Đã Chọn");
    btnLaunchSelected->setCursor(Qt::PointingHandCursor);
    btnLaunchSelected->setStyleSheet("QPushButton { background: #16a34a; color: #ffffff; font-weight: 700; border: none; border-radius: 8px; padding: 8px 14px; font-size: 12.5px; } QPushButton:hover { background: #15803d; }");
    connect(btnLaunchSelected, &QPushButton::clicked, this, &ChromeProfilesPage::onLaunchSelectedProfilesClicked);

    auto *btnStopSelected = new QPushButton("⏹ Dừng Đã Chọn");
    btnStopSelected->setCursor(Qt::PointingHandCursor);
    btnStopSelected->setStyleSheet("QPushButton { background: #ffffff; color: #ea580c; font-weight: 600; border: 1px solid #fed7aa; border-radius: 8px; padding: 8px 12px; font-size: 12.5px; } QPushButton:hover { background: #fff7ed; }");
    connect(btnStopSelected, &QPushButton::clicked, this, &ChromeProfilesPage::onStopSelectedProfilesClicked);

    auto *btnStopAll = new QPushButton("🛑 Dừng Tất Cả");
    btnStopAll->setCursor(Qt::PointingHandCursor);
    btnStopAll->setStyleSheet("QPushButton { background: #ffffff; color: #ef4444; font-weight: 600; border: 1px solid #fecaca; border-radius: 8px; padding: 8px 12px; font-size: 12.5px; } QPushButton:hover { background: #fef2f2; }");
    connect(btnStopAll, &QPushButton::clicked, this, &ChromeProfilesPage::onStopAllProfilesClicked);

    toolRow2->addWidget(btnToggleSelect);
    toolRow2->addWidget(btnLaunchSelected);
    toolRow2->addWidget(btnStopSelected);
    toolRow2->addWidget(btnStopAll);
    toolRow2->addStretch();
    layout->addLayout(toolRow2);

    // 3. Profiles Table
    m_profileTable = new QTableWidget();
    m_profileTable->setColumnCount(7);
    QStringList headers = {"[☑] STT", "Tên Profile", "Tỉ Lệ Mở", "Proxy Gán Kèm", "Remote Port (CDP)", "Trạng Thái", "Thao Tác"};
    m_profileTable->setHorizontalHeaderLabels(headers);
    m_profileTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_profileTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_profileTable->horizontalHeader()->setStretchLastSection(true);
    m_profileTable->verticalHeader()->setVisible(false);
    m_profileTable->verticalHeader()->setDefaultSectionSize(52);
    m_profileTable->setShowGrid(false);
    m_profileTable->setAlternatingRowColors(true);

    m_profileTable->setColumnWidth(0, 85);
    m_profileTable->setColumnWidth(1, 330);
    m_profileTable->setColumnWidth(2, 140);
    m_profileTable->setColumnWidth(3, 230);
    m_profileTable->setColumnWidth(4, 150);
    m_profileTable->setColumnWidth(5, 150);
    m_profileTable->setColumnWidth(6, 240);

    connect(m_profileTable, &QTableWidget::itemChanged, this, [this](QTableWidgetItem *item) {
        if (item && item->column() == 0) {
            int row = item->row();
            if (row >= 0 && row < m_profiles.size()) {
                m_profiles[row].isSelected = (item->checkState() == Qt::Checked);
            }
        }
    });

    layout->addWidget(m_profileTable, 1);
}

void ChromeProfilesPage::loadProfiles() {
    QString jsonPath = getProfilesStorageDir() + "/profiles.json";
    QFile file(jsonPath);
    m_profiles.clear();

    if (!file.exists()) {
        ChromeProfileItem p1;
        p1.id = "profile_1";
        p1.name = "Profile 01 - Nuôi Nick 1";
        p1.proxy = "103.149.28.12:8080";
        p1.port = 9222;
        p1.windowWidth = 380;
        p1.windowHeight = 680;
        p1.isRunning = false;
        m_profiles.append(p1);

        ChromeProfileItem p2;
        p2.id = "profile_2";
        p2.name = "Profile 02 - Tài Khoản Chính";
        p2.proxy = "";
        p2.port = 9223;
        p2.isRunning = false;
        m_profiles.append(p2);

        saveProfiles();
        refreshProfileTable();
        return;
    }

    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        file.close();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isArray()) {
            for (const auto &val : doc.array()) {
                QJsonObject obj = val.toObject();
                ChromeProfileItem item;
                item.id = obj["id"].toString();
                item.name = obj["name"].toString();
                item.proxy = obj["proxy"].toString();
                item.port = obj["port"].toInt(9222);
                item.windowWidth = obj["width"].toInt(0);
                item.windowHeight = obj["height"].toInt(0);
                item.deviceType = obj.value("device").toString("windows");
                item.customUserAgent = obj.value("userAgent").toString();
                item.isRunning = false;
                m_profiles.append(item);
            }
        }
    }

    refreshProfileTable();
}

void ChromeProfilesPage::saveProfiles() {
    QString jsonPath = getProfilesStorageDir() + "/profiles.json";
    QFile file(jsonPath);
    if (!file.open(QIODevice::WriteOnly)) return;

    QJsonArray arr;
    for (const auto &item : m_profiles) {
        QJsonObject obj;
        obj["id"] = item.id;
        obj["name"] = item.name;
        obj["proxy"] = item.proxy;
        obj["port"] = item.port;
        obj["width"] = item.windowWidth;
        obj["height"] = item.windowHeight;
        obj["device"] = item.deviceType;
        obj["userAgent"] = item.customUserAgent;
        arr.append(obj);
    }
    file.write(QJsonDocument(arr).toJson(QJsonDocument::Indented));
    file.close();

    emit profilesChanged(m_profiles);
}

void ChromeProfilesPage::refreshProfileTable() {
    if (!m_profileTable) return;

    m_profileTable->blockSignals(true);
    m_profileTable->setRowCount(0);
    m_profileTable->setRowCount(m_profiles.size());

    int activeCount = 0;
    int proxyCount = 0;

    for (int i = 0; i < m_profiles.size(); ++i) {
        const auto &item = m_profiles[i];
        if (item.isRunning) activeCount++;
        if (!item.proxy.isEmpty()) proxyCount++;

        // 0. STT + Checkbox
        auto *sttItem = new QTableWidgetItem(QString("  %1").arg(i + 1));
        sttItem->setCheckState(item.isSelected ? Qt::Checked : Qt::Unchecked);
        sttItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        m_profileTable->setItem(i, 0, sttItem);

        // 1. Name Cell with hover actions
        auto *nameWidget = new ProfileNameCellWidget(
            i, item,
            [this](int idx) {
                if (idx >= 0 && idx < m_profiles.size()) {
                    const auto &p = m_profiles[idx];
                    QStringList otherNames;
                    for (int k = 0; k < m_profiles.size(); ++k) {
                        if (k != idx) otherNames.append(m_profiles[k].name);
                    }
                    EditProfileDialog dlg(p.name, p.deviceType, p.customUserAgent, p.proxy, p.port, p.windowWidth, p.windowHeight, otherNames, this);
                    if (dlg.exec() == QDialog::Accepted) {
                        m_profiles[idx].name = dlg.getProfileName();
                        m_profiles[idx].deviceType = dlg.getDeviceType();
                        m_profiles[idx].customUserAgent = dlg.getUserAgent();
                        m_profiles[idx].proxy = dlg.getProxy();
                        m_profiles[idx].port = dlg.getPort();
                        m_profiles[idx].windowWidth = dlg.getWindowWidth();
                        m_profiles[idx].windowHeight = dlg.getWindowHeight();
                        saveProfiles();
                        refreshProfileTable();
                    }
                }
            },
            [this](int idx) {
                if (idx >= 0 && idx < m_profiles.size()) {
                    QuickProxyDialog dlg(m_profiles[idx].name, m_profiles[idx].proxy, this);
                    if (dlg.exec() == QDialog::Accepted) {
                        m_profiles[idx].proxy = dlg.getProxy();
                        saveProfiles();
                        refreshProfileTable();
                    }
                }
            },
            m_profileTable);
        m_profileTable->setCellWidget(i, 1, nameWidget);

        // 2. Size
        QString sizeText = (item.windowWidth > 0 && item.windowHeight > 0)
            ? QString("📐 %1 x %2").arg(item.windowWidth).arg(item.windowHeight)
            : "⚙ Tool Auto";
        auto *sizeItem = new QTableWidgetItem(sizeText);
        sizeItem->setTextAlignment(Qt::AlignCenter);
        m_profileTable->setItem(i, 2, sizeItem);

        // 3. Proxy
        ProxyConfig pCfg = ProxyConfig::fromString(item.proxy);
        auto *proxyItem = new QTableWidgetItem(pCfg.toDisplayString());
        proxyItem->setForeground(pCfg.isEmpty() ? QColor("#94a3b8") : QColor("#f97316"));
        m_profileTable->setItem(i, 3, proxyItem);

        // 4. Port
        auto *portItem = new QTableWidgetItem(QString::number(item.port));
        portItem->setTextAlignment(Qt::AlignCenter);
        m_profileTable->setItem(i, 4, portItem);

        // 5. Status
        QString statusText = item.isRunning ? QString("🟢 Đang chạy (Port %1)").arg(item.port) : "⚪ Đã tắt";
        auto *statusItem = new QTableWidgetItem(statusText);
        statusItem->setTextAlignment(Qt::AlignCenter);
        statusItem->setForeground(item.isRunning ? QColor("#16a34a") : QColor("#94a3b8"));
        m_profileTable->setItem(i, 5, statusItem);

        // 6. Action buttons
        auto *actionWidget = new QWidget();
        auto *actionLayout = new QHBoxLayout(actionWidget);
        actionLayout->setContentsMargins(8, 4, 8, 4);
        actionLayout->setSpacing(8);

        QString profileId = item.id;
        if (!item.isRunning) {
            auto *btnLaunch = new QPushButton("▶ Mở Chrome");
            btnLaunch->setCursor(Qt::PointingHandCursor);
            btnLaunch->setFixedHeight(32);
            btnLaunch->setStyleSheet("QPushButton { background: #f0fdf4; color: #16a34a; font-weight: 700; border: 1px solid #bbf7d0; border-radius: 8px; padding: 4px 12px; font-size: 12px; } QPushButton:hover { background: #16a34a; color: #ffffff; }");
            connect(btnLaunch, &QPushButton::clicked, [this, profileId]() {
                for (int k = 0; k < m_profiles.size(); ++k) {
                    if (m_profiles[k].id == profileId) {
                        // Launch
                        auto &p = m_profiles[k];
                        if (!p.launcher) {
                            p.launcher = new ChromeLauncher(this);
                            connect(p.launcher, &ChromeLauncher::processFinished, this, [this, k](int) {
                                if (k < m_profiles.size()) {
                                    m_profiles[k].isRunning = false;
                                    refreshProfileTable();
                                }
                            });
                        }
                        QString path = getProfilesStorageDir() + "/" + p.id;
                        QDir().mkpath(path);
                        int w = (p.windowWidth > 0) ? p.windowWidth : 480;
                        int h = (p.windowHeight > 0) ? p.windowHeight : 600;
                        QString ua = EditProfileDialog::getUserAgentForDevice(p.deviceType, p.customUserAgent);
                        if (p.launcher->launch(p.port, path, p.proxy, w, h, 100, 100, 1.0, ua)) {
                            p.isRunning = true;
                            refreshProfileTable();
                        } else {
                            CustomMessageBox::critical(this, "Lỗi", "Không thể khởi động Chrome!");
                        }
                        break;
                    }
                }
            });
            actionLayout->addWidget(btnLaunch);
        } else {
            auto *btnStop = new QPushButton("■ Dừng Chrome");
            btnStop->setCursor(Qt::PointingHandCursor);
            btnStop->setFixedHeight(32);
            btnStop->setStyleSheet("QPushButton { background: #fef2f2; color: #ef4444; font-weight: 700; border: 1px solid #fecaca; border-radius: 8px; padding: 4px 12px; font-size: 12px; } QPushButton:hover { background: #ef4444; color: #ffffff; }");
            connect(btnStop, &QPushButton::clicked, [this, profileId]() {
                for (int k = 0; k < m_profiles.size(); ++k) {
                    if (m_profiles[k].id == profileId) {
                        if (m_profiles[k].launcher) m_profiles[k].launcher->stop();
                        m_profiles[k].isRunning = false;
                        refreshProfileTable();
                        break;
                    }
                }
            });
            actionLayout->addWidget(btnStop);
        }

        auto *btnDel = new QPushButton("🗑");
        btnDel->setFixedSize(32, 32);
        btnDel->setCursor(Qt::PointingHandCursor);
        btnDel->setStyleSheet("QPushButton { background: #f8fafc; color: #94a3b8; border: 1px solid #e2e8f0; border-radius: 8px; font-size: 13px; } QPushButton:hover { color: #ef4444; border-color: #fca5a5; background: #fef2f2; }");
        connect(btnDel, &QPushButton::clicked, [this, profileId]() {
            for (int k = 0; k < m_profiles.size(); ++k) {
                if (m_profiles[k].id == profileId) {
                    if (m_profiles[k].launcher) {
                        m_profiles[k].launcher->stop();
                        delete m_profiles[k].launcher;
                    }
                    m_profiles.removeAt(k);
                    saveProfiles();
                    refreshProfileTable();
                    break;
                }
            }
        });
        actionLayout->addWidget(btnDel);

        m_profileTable->setCellWidget(i, 6, actionWidget);
    }

    m_profileTable->blockSignals(false);

    if (m_lblTotalProfiles) m_lblTotalProfiles->setText(QString::number(m_profiles.size()));
    if (m_lblActiveProfiles) m_lblActiveProfiles->setText(QString("%1 active").arg(activeCount));
    if (m_lblProxyProfiles) m_lblProxyProfiles->setText(QString::number(proxyCount));
}

void ChromeProfilesPage::onCreateProfileClicked() {
    QStringList existingNames;
    int maxPort = 9221;
    for (const auto &p : m_profiles) {
        existingNames.append(p.name.trimmed());
        if (p.port > maxPort) maxPort = p.port;
    }
    int suggestedPort = maxPort + 1;
    QString suggestedName = QString("Profile %1").arg(m_profiles.size() + 1, 2, 10, QChar('0'));

    CreateProfileDialog dlg(suggestedName, suggestedPort, existingNames, this);
    if (dlg.exec() == QDialog::Accepted) {
        ChromeProfileItem newItem;
        newItem.id = QString("profile_%1").arg(QDateTime::currentMSecsSinceEpoch());
        newItem.name = dlg.getProfileName();
        newItem.proxy = dlg.getProxy();
        newItem.port = dlg.getPort();
        newItem.windowWidth = dlg.getWindowWidth();
        newItem.windowHeight = dlg.getWindowHeight();
        newItem.deviceType = dlg.getDeviceType();
        newItem.customUserAgent = dlg.getUserAgent();
        newItem.isRunning = false;
        newItem.isSelected = false;

        QString profileDir = getProfilesStorageDir() + "/" + newItem.id;
        QDir().mkpath(profileDir);

        m_profiles.append(newItem);
        saveProfiles();
        refreshProfileTable();

        CustomMessageBox::information(this, "Thành công", QString("Đã tạo profile: %1").arg(newItem.name));
    }
}

void ChromeProfilesPage::onImportProfilesClicked() {
    ImportProfilesDialog dlg(m_profiles, this);
    if (dlg.exec() == QDialog::Accepted) {
        auto importedList = dlg.getImportedProfiles();
        for (const auto &item : importedList) {
            QString profileDir = getProfilesStorageDir() + "/" + item.id;
            QDir().mkpath(profileDir);
            m_profiles.append(item);
        }
        saveProfiles();
        refreshProfileTable();
        CustomMessageBox::information(this, "Nạp File Hoàn Tất", QString("Đã thêm thành công %1 Profile!").arg(importedList.size()));
    }
}

void ChromeProfilesPage::onExportProfilesTemplateClicked() {
    ImportProfilesDialog::exportTemplateCsv(this);
}

void ChromeProfilesPage::onOpenProfilesFolderClicked() {
    QDesktopServices::openUrl(QUrl::fromLocalFile(getProfilesStorageDir()));
}

void ChromeProfilesPage::onToggleSyncBarClicked() {
    CustomMessageBox::information(this, "Đồng bộ", "Tính năng đồng bộ chuột/phím đang hoạt động!");
}

void ChromeProfilesPage::onToggleSelectAllProfiles() {
    m_allSelected = !m_allSelected;
    for (auto &p : m_profiles) {
        p.isSelected = m_allSelected;
    }
    refreshProfileTable();
}

void ChromeProfilesPage::onLaunchSelectedProfilesClicked() {
    int count = 0;
    for (int i = 0; i < m_profiles.size(); ++i) {
        if (m_profiles[i].isSelected && !m_profiles[i].isRunning) {
            auto &p = m_profiles[i];
            if (!p.launcher) {
                p.launcher = new ChromeLauncher(this);
                connect(p.launcher, &ChromeLauncher::processFinished, this, [this, i](int) {
                    if (i < m_profiles.size()) {
                        m_profiles[i].isRunning = false;
                        refreshProfileTable();
                    }
                });
            }
            QString path = getProfilesStorageDir() + "/" + p.id;
            QDir().mkpath(path);
            int w = (p.windowWidth > 0) ? p.windowWidth : 480;
            int h = (p.windowHeight > 0) ? p.windowHeight : 600;
            QString ua = EditProfileDialog::getUserAgentForDevice(p.deviceType, p.customUserAgent);
            if (p.launcher->launch(p.port, path, p.proxy, w, h, 100 + count * 30, 100 + count * 30, 1.0, ua)) {
                p.isRunning = true;
                count++;
            }
        }
    }
    refreshProfileTable();
    CustomMessageBox::information(this, "Thành công", QString("Đã khởi chạy %1 Profile!").arg(count));
}

void ChromeProfilesPage::onStopSelectedProfilesClicked() {
    int count = 0;
    for (auto &p : m_profiles) {
        if (p.isSelected && p.isRunning) {
            if (p.launcher) p.launcher->stop();
            p.isRunning = false;
            count++;
        }
    }
    refreshProfileTable();
    CustomMessageBox::information(this, "Đã dừng", QString("Đã dừng %1 Profile!").arg(count));
}

void ChromeProfilesPage::onStopAllProfilesClicked() {
    int count = 0;
    for (auto &p : m_profiles) {
        if (p.isRunning) {
            if (p.launcher) p.launcher->stop();
            p.isRunning = false;
            count++;
        }
    }
    refreshProfileTable();
    CustomMessageBox::information(this, "Đã dừng", QString("Đã dừng tất cả %1 Profile!").arg(count));
}

void ChromeProfilesPage::onBatchAssignProxyClicked() {}
void ChromeProfilesPage::onDeleteSelectedProfilesClicked() {}
void ChromeProfilesPage::onArrangeGridClicked() {}

void ChromeProfilesPage::onProfileSearchFilterChanged(const QString &text) {
    m_searchQuery = text.trimmed().toLower();
    if (!m_profileTable) return;
    for (int row = 0; row < m_profileTable->rowCount(); ++row) {
        if (m_searchQuery.isEmpty()) {
            m_profileTable->setRowHidden(row, false);
            continue;
        }
        bool match = false;
        for (int col = 1; col <= 4; ++col) {
            auto *item = m_profileTable->item(row, col);
            if (item && item->text().toLower().contains(m_searchQuery)) {
                match = true;
                break;
            }
        }
        m_profileTable->setRowHidden(row, !match);
    }
}
