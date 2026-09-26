#include "ProxyPoolPage.h"
#include "core/ProxyChecker.h"
#include "core/ProxyConfig.h"
#include "ui/CustomMessageBox.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QListView>

ProxyPoolPage::ProxyPoolPage(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void ProxyPoolPage::setupUi() {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 20, 24, 20);
    layout->setSpacing(14);

    // 1. Header Card with KPI Stats
    auto *headerCard = new QWidget();
    headerCard->setStyleSheet("background: #f8fafc; border: 1px solid #f1f5f9; border-radius: 12px; padding: 12px 16px;");
    auto *headerLayout = new QHBoxLayout(headerCard);
    headerLayout->setContentsMargins(0, 0, 0, 0);

    auto *headerIcon = new QLabel("🌐");
    headerIcon->setStyleSheet("font-size: 24px; background: #e0f2fe; border: 1px solid #bae6fd; border-radius: 12px; padding: 6px 10px;");
    auto *titleBox = new QVBoxLayout();
    titleBox->setSpacing(2);
    auto *titleText = new QLabel("Kho Proxy (Proxy Pool) & Kiểm Tra Tình Trạng Kết Nối");
    titleText->setStyleSheet("font-size: 16px; font-weight: 800; color: #0f172a;");
    auto *subtitleText = new QLabel("Tự động trích xuất Proxy từ các Chrome Profiles, kiểm tra độ trễ (Ping ms), quốc gia và tình trạng Sống / Chết.");
    subtitleText->setStyleSheet("font-size: 12px; color: #64748b; font-weight: 500;");
    titleBox->addWidget(titleText);
    titleBox->addWidget(subtitleText);

    headerLayout->addWidget(headerIcon);
    headerLayout->addLayout(titleBox);
    headerLayout->addStretch();

    // KPI Stats
    auto *statsLayout = new QHBoxLayout();
    statsLayout->setSpacing(10);

    auto createStatCard = [](const QString &title, QLabel *&valLbl, const QString &valColor, const QString &defaultVal = "0") {
        auto *card = new QWidget();
        card->setStyleSheet("background: #ffffff; border: 1px solid #e2e8f0; border-radius: 8px; padding: 6px 12px;");
        auto *ly = new QVBoxLayout(card);
        ly->setContentsMargins(0, 0, 0, 0);
        ly->setSpacing(1);
        auto *t = new QLabel(title);
        t->setStyleSheet("font-size: 10.5px; font-weight: 700; color: #64748b; text-transform: uppercase;");
        valLbl = new QLabel(defaultVal);
        valLbl->setStyleSheet(QString("font-size: 14px; font-weight: 800; color: %1;").arg(valColor));
        ly->addWidget(t);
        ly->addWidget(valLbl);
        return card;
    };

    statsLayout->addWidget(createStatCard("TỔNG PROXY", m_lblStatTotalProxy, "#0f172a"));
    statsLayout->addWidget(createStatCard("🟢 ĐANG SỐNG", m_lblStatLiveProxy, "#16a34a"));
    statsLayout->addWidget(createStatCard("🔴 ĐÃ CHẾT", m_lblStatDieProxy, "#ef4444"));

    headerLayout->addLayout(statsLayout);
    layout->addWidget(headerCard);

    // 2. Toolbar
    auto *toolRow = new QHBoxLayout();
    toolRow->setSpacing(10);

    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("🔍 Tìm kiếm theo IP, Cổng, Quốc gia, Tên Profile...");
    m_searchEdit->setClearButtonEnabled(true);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &ProxyPoolPage::onProxyPoolSearchChanged);
    toolRow->addWidget(m_searchEdit, 1);

    m_filterCombo = new QComboBox();
    m_filterCombo->setView(new QListView(m_filterCombo));
    m_filterCombo->addItem("Tất Cả Trạng Thái");
    m_filterCombo->addItem("🟢 Chỉ Proxy Sống (Live)");
    m_filterCombo->addItem("🔴 Chỉ Proxy Chết (Die)");
    m_filterCombo->addItem("⚪ Chưa Kiểm Tra");
    connect(m_filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ProxyPoolPage::onFilterProxyStatusChanged);
    toolRow->addWidget(m_filterCombo);

    m_btnCheckAll = new QPushButton("⚡ Kiểm Tra Toàn Bộ Proxy (Check All)");
    m_btnCheckAll->setCursor(Qt::PointingHandCursor);
    m_btnCheckAll->setStyleSheet("QPushButton { background: #2563eb; color: #ffffff; font-weight: 700; border: none; border-radius: 8px; padding: 9px 16px; font-size: 13px; } QPushButton:hover { background: #1d4ed8; }");
    connect(m_btnCheckAll, &QPushButton::clicked, this, &ProxyPoolPage::onCheckAllProxiesClicked);
    toolRow->addWidget(m_btnCheckAll);

    auto *btnSync = new QPushButton("🔄 Đồng Bộ Từ Profiles");
    btnSync->setCursor(Qt::PointingHandCursor);
    btnSync->setStyleSheet("QPushButton { background: #ffffff; color: #334155; font-weight: 600; border: 1px solid #cbd5e1; border-radius: 8px; padding: 9px 14px; font-size: 13px; } QPushButton:hover { background: #f8fafc; }");
    connect(btnSync, &QPushButton::clicked, this, &ProxyPoolPage::onSyncProxiesFromProfiles);
    toolRow->addWidget(btnSync);

    layout->addLayout(toolRow);

    // 3. Table
    m_proxyTable = new QTableWidget();
    m_proxyTable->setColumnCount(7);
    QStringList headers = {"STT", "Địa Chỉ Proxy", "Profile Đang Gán", "Trạng Thái", "Độ Trễ (Ping)", "Quốc Gia & IP Public", "Thao Tác"};
    m_proxyTable->setHorizontalHeaderLabels(headers);
    m_proxyTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_proxyTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_proxyTable->verticalHeader()->setVisible(false);
    m_proxyTable->verticalHeader()->setDefaultSectionSize(52);
    m_proxyTable->setShowGrid(false);
    m_proxyTable->setAlternatingRowColors(true);

    m_proxyTable->setColumnWidth(0, 60);
    m_proxyTable->setColumnWidth(1, 240);
    m_proxyTable->setColumnWidth(2, 220);
    m_proxyTable->setColumnWidth(3, 150);
    m_proxyTable->setColumnWidth(4, 120);
    m_proxyTable->setColumnWidth(5, 240);
    m_proxyTable->horizontalHeader()->setStretchLastSection(true);

    layout->addWidget(m_proxyTable, 1);
}

void ProxyPoolPage::updateProfiles(const QList<ChromeProfileItem> &profiles) {
    m_cachedProfiles = profiles;
    refreshProxyPoolTable();
}

void ProxyPoolPage::refreshProxyPoolTable() {
    if (!m_proxyTable) return;

    QMap<QString, ProxyPoolItem> existingResults;
    for (const auto &item : m_proxyItems) {
        existingResults[item.rawProxy] = item;
    }

    QMap<QString, QStringList> profileProxyMap;
    for (const auto &p : m_cachedProfiles) {
        QString px = p.proxy.trimmed();
        if (px.isEmpty()) px = "[DIRECT_MACHINE]";
        profileProxyMap[px].append(p.name);
    }

    m_proxyItems.clear();
    for (auto it = profileProxyMap.begin(); it != profileProxyMap.end(); ++it) {
        QString raw = it.key();
        ProxyPoolItem item;
        if (existingResults.contains(raw)) {
            item = existingResults[raw];
            item.associatedProfileNames = it.value();
        } else {
            item.rawProxy = raw;
            item.associatedProfileNames = it.value();
            item.checked = false;
            item.isChecking = false;
            item.isLive = false;
            item.pingMs = -1;
        }
        m_proxyItems.append(item);
    }

    updateProxyPoolKpis();

    m_proxyTable->blockSignals(true);
    m_proxyTable->setRowCount(0);
    m_proxyTable->setRowCount(m_proxyItems.size());

    for (int i = 0; i < m_proxyItems.size(); ++i) {
        const auto &item = m_proxyItems[i];

        // 0. STT
        auto *sttItem = new QTableWidgetItem(QString::number(i + 1));
        sttItem->setTextAlignment(Qt::AlignCenter);
        sttItem->setFont(QFont("Google Sans", 10, QFont::Bold));
        m_proxyTable->setItem(i, 0, sttItem);

        // 1. Proxy
        QString dispProxy;
        if (item.rawProxy == "[DIRECT_MACHINE]") {
            dispProxy = "⚡ Direct (Mạng máy chủ)";
        } else {
            ProxyConfig cfg = ProxyConfig::fromString(item.rawProxy);
            dispProxy = cfg.toDisplayString();
        }
        auto *proxyItem = new QTableWidgetItem(dispProxy);
        proxyItem->setFont(QFont("Google Sans", 10, QFont::Bold));
        proxyItem->setToolTip(item.rawProxy == "[DIRECT_MACHINE]" ? "Kết nối mạng trực tiếp" : item.rawProxy);
        proxyItem->setForeground(item.rawProxy == "[DIRECT_MACHINE]" ? QColor("#64748b") : QColor("#0f172a"));
        m_proxyTable->setItem(i, 1, proxyItem);

        // 2. Profiles
        QString profNames = item.associatedProfileNames.join(", ");
        if (profNames.length() > 45) {
            profNames = profNames.left(42) + "...";
        }
        auto *profItem = new QTableWidgetItem(QString("👥 %1 (%2)").arg(profNames).arg(item.associatedProfileNames.size()));
        profItem->setToolTip(item.associatedProfileNames.join("\n"));
        profItem->setForeground(QColor("#2563eb"));
        m_proxyTable->setItem(i, 2, profItem);

        // 3. Status
        QString statusText;
        QColor statusColor("#94a3b8");
        if (item.isChecking) {
            statusText = "⏳ Đang kiểm tra...";
            statusColor = QColor("#ea580c");
        } else if (!item.checked) {
            statusText = "⚪ Chưa kiểm tra";
            statusColor = QColor("#94a3b8");
        } else if (item.isLive) {
            statusText = "🟢 Sống (Live)";
            statusColor = QColor("#16a34a");
        } else {
            statusText = "🔴 Chết (Die)";
            statusColor = QColor("#ef4444");
        }
        auto *statusItem = new QTableWidgetItem(statusText);
        statusItem->setTextAlignment(Qt::AlignCenter);
        statusItem->setForeground(statusColor);
        statusItem->setFont(QFont("Google Sans", 9.5, QFont::Bold));
        m_proxyTable->setItem(i, 3, statusItem);

        // 4. Ping
        QString pingText = "—";
        QColor pingColor("#94a3b8");
        if (item.checked && item.isLive) {
            pingText = QString("%1 ms").arg(item.pingMs);
            if (item.pingMs < 350) pingColor = QColor("#16a34a");
            else if (item.pingMs < 800) pingColor = QColor("#ea580c");
            else pingColor = QColor("#ef4444");
        }
        auto *pingItem = new QTableWidgetItem(pingText);
        pingItem->setTextAlignment(Qt::AlignCenter);
        pingItem->setForeground(pingColor);
        pingItem->setFont(QFont("Google Sans", 10, QFont::Bold));
        m_proxyTable->setItem(i, 4, pingItem);

        // 5. Geo
        QString geoText = "—";
        if (item.checked) {
            if (item.isLive) {
                geoText = QString("%1 %2 (%3)").arg(item.flagEmoji, item.country, item.publicIp);
            } else {
                geoText = item.errorMsg.isEmpty() ? "Lỗi kết nối" : item.errorMsg;
            }
        }
        auto *geoItem = new QTableWidgetItem(geoText);
        if (item.checked && !item.isLive) {
            geoItem->setForeground(QColor("#ef4444"));
        } else if (item.checked && item.isLive) {
            geoItem->setForeground(QColor("#0f172a"));
        } else {
            geoItem->setForeground(QColor("#94a3b8"));
        }
        m_proxyTable->setItem(i, 5, geoItem);

        // 6. Action
        auto *actionWidget = new QWidget();
        auto *actionLayout = new QHBoxLayout(actionWidget);
        actionLayout->setContentsMargins(6, 4, 6, 4);
        actionLayout->setSpacing(6);

        auto *btnCheck = new QPushButton("🔍 Kiểm Tra");
        btnCheck->setCursor(Qt::PointingHandCursor);
        btnCheck->setFixedHeight(28);
        btnCheck->setEnabled(!item.isChecking);
        btnCheck->setStyleSheet("QPushButton { background: #eff6ff; color: #2563eb; font-weight: 700; border: 1px solid #bfdbfe; border-radius: 6px; padding: 2px 10px; font-size: 11.5px; } QPushButton:hover { background: #2563eb; color: #ffffff; }");
        connect(btnCheck, &QPushButton::clicked, [this, i]() { onCheckSingleProxy(i); });
        actionLayout->addWidget(btnCheck);
        actionLayout->addStretch();
        m_proxyTable->setCellWidget(i, 6, actionWidget);
    }

    m_proxyTable->blockSignals(false);
    onProxyPoolSearchChanged(m_searchEdit ? m_searchEdit->text() : "");
}

int ProxyPoolPage::getLiveProxies() const {
    int count = 0;
    for (const auto &item : m_proxyItems) {
        if (item.checked && item.isLive) count++;
    }
    return count;
}

void ProxyPoolPage::updateProxyPoolKpis() {
    int total = m_proxyItems.size();
    int live = 0, die = 0;
    for (const auto &item : m_proxyItems) {
        if (item.checked) {
            if (item.isLive) live++;
            else die++;
        }
    }
    if (m_lblStatTotalProxy) m_lblStatTotalProxy->setText(QString::number(total));
    if (m_lblStatLiveProxy) m_lblStatLiveProxy->setText(QString("%1 live").arg(live));
    if (m_lblStatDieProxy) m_lblStatDieProxy->setText(QString::number(die));
}

void ProxyPoolPage::onCheckSingleProxy(int itemIndex) {
    if (itemIndex < 0 || itemIndex >= m_proxyItems.size()) return;
    auto &item = m_proxyItems[itemIndex];
    if (item.isChecking) return;

    item.isChecking = true;
    if (m_proxyTable && itemIndex < m_proxyTable->rowCount()) {
        auto *statusItem = m_proxyTable->item(itemIndex, 3);
        if (statusItem) {
            statusItem->setText("⏳ Đang kiểm tra...");
            statusItem->setForeground(QColor("#ea580c"));
        }
    }

    QString targetProxy = (item.rawProxy == "[DIRECT_MACHINE]") ? "" : item.rawProxy;
    auto *checker = new SingleProxyChecker(targetProxy, 8000, this);
    connect(checker, &SingleProxyChecker::finished, this, [this, itemIndex](const ProxyCheckResult &res) {
        if (itemIndex >= 0 && itemIndex < m_proxyItems.size()) {
            auto &it = m_proxyItems[itemIndex];
            it.isChecking = false;
            it.checked = true;
            it.isLive = res.isLive;
            it.pingMs = res.pingMs;
            it.country = res.country;
            it.countryCode = res.countryCode;
            it.publicIp = res.publicIp;
            it.flagEmoji = res.flagEmoji;
            it.errorMsg = res.errorMsg;

            updateProxyPoolKpis();

            if (m_proxyTable && itemIndex < m_proxyTable->rowCount()) {
                auto *statusItem = m_proxyTable->item(itemIndex, 3);
                if (statusItem) {
                    statusItem->setText(it.isLive ? "🟢 Sống (Live)" : "🔴 Chết (Die)");
                    statusItem->setForeground(it.isLive ? QColor("#16a34a") : QColor("#ef4444"));
                }
                auto *pingItem = m_proxyTable->item(itemIndex, 4);
                if (pingItem) {
                    if (it.isLive) {
                        pingItem->setText(QString("%1 ms").arg(it.pingMs));
                        pingItem->setForeground(it.pingMs < 350 ? QColor("#16a34a") : (it.pingMs < 800 ? QColor("#ea580c") : QColor("#ef4444")));
                    } else {
                        pingItem->setText("—");
                        pingItem->setForeground(QColor("#94a3b8"));
                    }
                }
                auto *geoItem = m_proxyTable->item(itemIndex, 5);
                if (geoItem) {
                    if (it.isLive) {
                        geoItem->setText(QString("%1 %2 (%3)").arg(it.flagEmoji, it.country, it.publicIp));
                        geoItem->setForeground(QColor("#0f172a"));
                    } else {
                        geoItem->setText(it.errorMsg);
                        geoItem->setForeground(QColor("#ef4444"));
                    }
                }
            }
        }
    });
    checker->start();
}

void ProxyPoolPage::onCheckAllProxiesClicked() {
    if (m_proxyItems.isEmpty()) {
        CustomMessageBox::information(this, "Thông báo", "Chưa có Proxy nào trong danh sách!");
        return;
    }
    for (int i = 0; i < m_proxyItems.size(); ++i) {
        onCheckSingleProxy(i);
    }
}

void ProxyPoolPage::onSyncProxiesFromProfiles() {
    refreshProxyPoolTable();
    CustomMessageBox::information(this, "Đã Đồng Bộ", QString("Đã quét và đồng bộ %1 địa chỉ Proxy từ Profiles!").arg(m_proxyItems.size()));
}

void ProxyPoolPage::onFilterProxyStatusChanged(int index) {
    m_filterStatus = index;
    onProxyPoolSearchChanged(m_searchEdit ? m_searchEdit->text() : "");
}

void ProxyPoolPage::onProxyPoolSearchChanged(const QString &text) {
    if (!m_proxyTable) return;
    QString query = text.trimmed().toLower();

    for (int row = 0; row < m_proxyItems.size(); ++row) {
        const auto &item = m_proxyItems[row];
        bool matchesStatus = true;
        if (m_filterStatus == 1) matchesStatus = (item.checked && item.isLive);
        else if (m_filterStatus == 2) matchesStatus = (item.checked && !item.isLive);
        else if (m_filterStatus == 3) matchesStatus = (!item.checked);

        bool matchesQuery = true;
        if (!query.isEmpty()) {
            QString combined = QString("%1 %2 %3 %4 %5")
                .arg(item.rawProxy)
                .arg(item.associatedProfileNames.join(" "))
                .arg(item.country)
                .arg(item.publicIp)
                .arg(item.countryCode).toLower();
            matchesQuery = combined.contains(query);
        }

        m_proxyTable->setRowHidden(row, !(matchesStatus && matchesQuery));
    }
}
