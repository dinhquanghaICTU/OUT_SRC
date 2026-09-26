#include "DashboardPage.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QFrame>

DashboardPage::DashboardPage(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void DashboardPage::setupUi() {
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setStyleSheet(
        "QScrollArea { background-color: #f8fafc; border: none; }"
        "QWidget#dashScrollContent { background-color: #f8fafc; }");

    auto *contentWidget = new QWidget();
    contentWidget->setObjectName("dashScrollContent");
    auto *layout = new QVBoxLayout(contentWidget);
    layout->setContentsMargins(36, 26, 36, 32);
    layout->setSpacing(22);

    // 1. TOP GREETING & STATUS ROW
    auto *topGreetingRow = new QHBoxLayout();
    auto *greetCol = new QVBoxLayout();
    greetCol->setSpacing(4);

    auto *greetTitle = new QLabel("Tổng Quan Hệ Thống ⚡");
    greetTitle->setStyleSheet(
        "font-size: 22px; font-weight: 800; color: #0f172a; "
        "font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif; "
        "border: none; background: transparent;");
    auto *greetSub = new QLabel("Trung tâm điều khiển & tự động hóa MMO TunnBit Platform");
    greetSub->setStyleSheet("font-size: 13px; color: #64748b; font-weight: 500; border: none; background: transparent;");
    greetCol->addWidget(greetTitle);
    greetCol->addWidget(greetSub);
    topGreetingRow->addLayout(greetCol, 1);

    auto *statusRow = new QHBoxLayout();
    statusRow->setSpacing(10);
    statusRow->setAlignment(Qt::AlignVCenter | Qt::AlignRight);

    auto *badgeSys = new QLabel("🟢 Hệ thống: Hoạt động");
    badgeSys->setFixedHeight(32);
    badgeSys->setStyleSheet(
        "background: #f0fdf4; color: #16a34a; font-size: 11.5px; font-weight: 700; "
        "border: 1px solid #bbf7d0; border-radius: 7px; padding: 0 14px;");
    auto *badgeVer = new QLabel("⚡ v2.5.0 Pro");
    badgeVer->setFixedHeight(32);
    badgeVer->setStyleSheet(
        "background: #f8fafc; color: #475569; font-size: 11.5px; font-weight: 700; "
        "border: 1px solid #e2e8f0; border-radius: 7px; padding: 0 14px;");
    statusRow->addWidget(badgeSys);
    statusRow->addWidget(badgeVer);
    topGreetingRow->addLayout(statusRow);

    layout->addLayout(topGreetingRow);

    // 2. HERO BANNER
    auto *heroCard = new QWidget();
    heroCard->setObjectName("dashHero");
    heroCard->setMinimumHeight(125);
    heroCard->setStyleSheet(
        "QWidget#dashHero { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #0f172a, stop:0.6 #1e293b, stop:1 #334155); "
        "border-radius: 16px; border: 1px solid #334155; } "
        "QLabel { border: none; background: transparent; }");

    auto *heroLayout = new QHBoxLayout(heroCard);
    heroLayout->setContentsMargins(28, 20, 28, 20);
    heroLayout->setSpacing(24);

    auto *heroLeft = new QVBoxLayout();
    heroLeft->setSpacing(8);

    auto *badgeRow = new QHBoxLayout();
    auto *heroBadge = new QLabel("⚡ TUNNBIT AUTOMATION V2.5 PRO");
    heroBadge->setStyleSheet(
        "background: #ea580c; color: #ffffff; border-radius: 5px; padding: 4px 10px; font-size: 10.5px; font-weight: 700; border: none;");
    badgeRow->addWidget(heroBadge);
    badgeRow->addStretch();
    heroLeft->addLayout(badgeRow);

    auto *heroHeadline = new QLabel("Hệ Sinh Thái Nuôi & Tự Động Hóa MMO Toàn Diện");
    heroHeadline->setStyleSheet(
        "color: #ffffff; font-size: 20px; font-weight: 800; "
        "font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;");
    heroLeft->addWidget(heroHeadline);

    auto *heroSub = new QLabel("Anti-Detect Browser • Siêu đồng bộ Master-Worker • Proxy Pool tự động check ping");
    heroSub->setStyleSheet("color: #94a3b8; font-size: 12.5px; font-weight: 500;");
    heroLeft->addWidget(heroSub);

    heroLayout->addLayout(heroLeft, 1);

    auto *heroRight = new QHBoxLayout();
    heroRight->setSpacing(12);
    heroRight->setAlignment(Qt::AlignVCenter);

    auto *btnHeroProfiles = new QPushButton("👥 Mở Profiles");
    btnHeroProfiles->setCursor(Qt::PointingHandCursor);
    btnHeroProfiles->setFixedHeight(42);
    btnHeroProfiles->setStyleSheet(
        "QPushButton { background: #f97316; color: #ffffff; font-weight: 700; border-radius: 8px; "
        "padding: 0 22px; font-size: 13px; border: none; } QPushButton:hover { background: #ea580c; }");
    connect(btnHeroProfiles, &QPushButton::clicked, this, [this]() { emit requestNavigate(2); });
    heroRight->addWidget(btnHeroProfiles);

    auto *btnHeroUpgrade = new QPushButton("💳 Nâng Cấp VIP");
    btnHeroUpgrade->setCursor(Qt::PointingHandCursor);
    btnHeroUpgrade->setFixedHeight(42);
    btnHeroUpgrade->setStyleSheet(
        "QPushButton { background: #10b981; color: #ffffff; font-weight: 700; border-radius: 8px; "
        "padding: 0 22px; font-size: 13px; border: none; } QPushButton:hover { background: #059669; }");
    connect(btnHeroUpgrade, &QPushButton::clicked, this, [this]() { emit requestNavigate(5); });
    heroRight->addWidget(btnHeroUpgrade);

    heroLayout->addLayout(heroRight);
    layout->addWidget(heroCard);

    // 3. 4 SPACIOUS KPI METRIC CARDS
    auto *kpiRow = new QHBoxLayout();
    kpiRow->setSpacing(16);

    auto createKpiCard = [](const QString &icon, const QString &title, QLabel *&valLbl, const QString &defaultVal,
                            QLabel *&subLbl, const QString &defaultSub, const QString &iconBg) {
        auto *card = new QWidget();
        card->setObjectName("kpiCard");
        card->setStyleSheet(
            "QWidget#kpiCard { background: #ffffff; border: 1px solid #e2e8f0; border-radius: 14px; } "
            "QWidget#kpiCard:hover { border-color: #cbd5e1; } "
            "QLabel { border: none; background: transparent; }");
        auto *cLayout = new QVBoxLayout(card);
        cLayout->setContentsMargins(20, 18, 20, 18);
        cLayout->setSpacing(8);

        auto *topRow = new QHBoxLayout();
        topRow->setSpacing(10);
        auto *iconLbl = new QLabel(icon);
        iconLbl->setFixedSize(40, 40);
        iconLbl->setAlignment(Qt::AlignCenter);
        iconLbl->setStyleSheet(QString("background: %1; border-radius: 10px; font-size: 18px; border: none;").arg(iconBg));
        topRow->addWidget(iconLbl);

        auto *titleLbl = new QLabel(title);
        titleLbl->setStyleSheet("font-size: 11.5px; font-weight: 700; color: #64748b; text-transform: uppercase; letter-spacing: 0.4px;");
        topRow->addWidget(titleLbl, 1);
        cLayout->addLayout(topRow);

        valLbl = new QLabel(defaultVal);
        valLbl->setStyleSheet(
            "font-size: 24px; font-weight: 800; color: #0f172a; "
            "font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;");
        cLayout->addWidget(valLbl);

        subLbl = new QLabel(defaultSub);
        subLbl->setStyleSheet("font-size: 12px; color: #94a3b8; font-weight: 500;");
        cLayout->addWidget(subLbl);

        return card;
    };

    QLabel *subDummy = nullptr;
    kpiRow->addWidget(createKpiCard("👥", "Chrome Profiles", m_lblTotalProfiles, "6 Profiles",
                                    m_lblRunningProfiles, "🟢 Đang chạy: 0 tab", "#eff6ff"));
    kpiRow->addWidget(createKpiCard("🌐", "Proxy Pool", m_lblTotalProxies, "1 Proxies",
                                    m_lblLiveProxies, "⚡ Live: 0 | Sẵn sàng", "#f0fdf4"));
    kpiRow->addWidget(createKpiCard("⚡", "Tài Khoản Đã Lọc", subDummy, "125,400+",
                                    subDummy, "Độ chính xác 99.9%", "#fff7ed"));
    kpiRow->addWidget(createKpiCard("👑", "Bản Quyền", m_lblLicenseStatus, "Bản Dùng Thử",
                                    m_lblLicenseDays, "Hạn dùng: Còn 7 ngày", "#fdf4ff"));

    layout->addLayout(kpiRow);

    // 4. CLEAN TOOL SHOWCASE CARDS (2x2 GRID)
    auto *featuresGrid = new QGridLayout();
    featuresGrid->setSpacing(16);

    auto createFeatureCard = [this](const QString &icon, const QString &tag, const QString &tagColor, const QString &tagBg,
                                    const QString &title, const QString &desc, const QString &btnText,
                                    const QString &btnBg, const QString &iconBg, int navTarget) {
        auto *card = new QWidget();
        card->setObjectName("featCard");
        card->setStyleSheet(
            "QWidget#featCard { background: #ffffff; border: 1px solid #e2e8f0; border-radius: 14px; } "
            "QWidget#featCard:hover { border-color: #cbd5e1; } "
            "QLabel { border: none; background: transparent; }");
        auto *cLayout = new QVBoxLayout(card);
        cLayout->setContentsMargins(20, 18, 20, 18);
        cLayout->setSpacing(12);

        auto *topRow = new QHBoxLayout();
        topRow->setSpacing(12);

        auto *iconLbl = new QLabel(icon);
        iconLbl->setFixedSize(44, 44);
        iconLbl->setAlignment(Qt::AlignCenter);
        iconLbl->setStyleSheet(QString("background: %1; border-radius: 12px; font-size: 20px; border: none;").arg(iconBg));
        topRow->addWidget(iconLbl);

        auto *tCol = new QVBoxLayout();
        tCol->setSpacing(2);
        auto *tagLbl = new QLabel(tag);
        tagLbl->setStyleSheet(QString("color: %1; background: %2; font-size: 10px; font-weight: 800; border-radius: 4px; padding: 2px 6px;").arg(tagColor, tagBg));
        auto *titleLbl = new QLabel(title);
        titleLbl->setStyleSheet("font-size: 15px; font-weight: 800; color: #0f172a;");
        tCol->addWidget(tagLbl);
        tCol->addWidget(titleLbl);
        topRow->addLayout(tCol, 1);
        cLayout->addLayout(topRow);

        auto *descLbl = new QLabel(desc);
        descLbl->setWordWrap(true);
        descLbl->setStyleSheet("color: #64748b; font-size: 12.5px; line-height: 1.4;");
        cLayout->addWidget(descLbl, 1);

        auto *btn = new QPushButton(btnText);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFixedHeight(36);
        btn->setStyleSheet(QString(
            "QPushButton { background: %1; color: #ffffff; font-weight: 700; border-radius: 8px; font-size: 12px; border: none; } "
            "QPushButton:hover { opacity: 0.9; }").arg(btnBg));
        connect(btn, &QPushButton::clicked, this, [this, navTarget]() {
            emit requestNavigate(navTarget);
        });
        cLayout->addWidget(btn);

        return card;
    };

    featuresGrid->addWidget(createFeatureCard("👥", "ANTI-DETECT", "#2563eb", "#eff6ff",
                                              "Chrome Profiles",
                                              "Tạo và quản lý đa trình duyệt Chrome riêng biệt dấu vân tay, Canvas, WebGL, Audio.",
                                              "Mở Profiles →", "#2563eb", "#eff6ff", 2), 0, 0);

    featuresGrid->addWidget(createFeatureCard("⚡", "ĐỒNG BỘ 10X", "#ea580c", "#fff7ed",
                                              "Siêu Đồng Bộ Master - Worker",
                                              "Thao tác chuột, bàn phím và scroll trên profile Master sẽ nhân bản sang các profile con.",
                                              "Đồng Bộ →", "#ea580c", "#fff7ed", 2), 0, 1);

    featuresGrid->addWidget(createFeatureCard("🌐", "PROXY POOL", "#059669", "#f0fdf4",
                                              "Proxy Pool Tự Động",
                                              "Tự động quét và kiểm tra độ trễ (ping), IP, quốc gia của Proxy. Thay thế proxy die tức thì.",
                                              "Check Proxy →", "#059669", "#f0fdf4", 3), 1, 0);

    featuresGrid->addWidget(createFeatureCard("📑", "TÁCH FILE", "#7c3aed", "#f5f3ff",
                                              "Lọc & Tách Tài Khoản MMO",
                                              "Nhập danh sách tài khoản theo định dạng tuỳ biến, tự động loại bỏ trùng lặp và xuất Excel.",
                                              "Lọc File →", "#7c3aed", "#f5f3ff", 1), 1, 1);

    layout->addLayout(featuresGrid);

    // 5. COMPACT VIP PROMO STRIP
    auto *promoCard = new QWidget();
    promoCard->setObjectName("dashPromo");
    promoCard->setFixedHeight(52);
    promoCard->setStyleSheet(
        "QWidget#dashPromo { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #ea580c, stop:1 #f97316); "
        "border-radius: 10px; } QLabel { border: none; background: transparent; }");
    auto *promoLayout = new QHBoxLayout(promoCard);
    promoLayout->setContentsMargins(20, 0, 16, 0);
    promoLayout->setSpacing(14);

    auto *promoText = new QLabel("🔥 Nâng cấp gói 1 Năm hoặc Vĩnh Viễn để mở khóa không giới hạn Profiles & Luồng chạy!");
    promoText->setStyleSheet("color: #ffffff; font-size: 13px; font-weight: 700;");
    promoLayout->addWidget(promoText, 1);

    auto *btnPromo = new QPushButton("Quét Mã VietQR 💳");
    btnPromo->setCursor(Qt::PointingHandCursor);
    btnPromo->setFixedHeight(32);
    btnPromo->setStyleSheet(
        "QPushButton { background: #ffffff; color: #ea580c; font-weight: 800; font-size: 11.5px; border-radius: 6px; padding: 0 14px; border: none; } "
        "QPushButton:hover { background: #fff7ed; }");
    connect(btnPromo, &QPushButton::clicked, this, [this]() { emit requestNavigate(5); });
    promoLayout->addWidget(btnPromo);

    layout->addWidget(promoCard);

    // 6. MINIMAL CLEAN FOOTER
    auto *footerRow = new QHBoxLayout();
    footerRow->setContentsMargins(4, 0, 4, 0);
    auto *footLeft = new QLabel("© 2026 TunnBit Platform. All rights reserved.");
    footLeft->setStyleSheet("color: #94a3b8; font-size: 11.5px; font-weight: 500; border: none; background: transparent;");
    auto *footRight = new QLabel("Hỗ trợ kỹ thuật 24/7: Zalo / Telegram @tunnnguyen");
    footRight->setStyleSheet("color: #64748b; font-size: 11.5px; font-weight: 600; border: none; background: transparent;");
    footerRow->addWidget(footLeft);
    footerRow->addStretch();
    footerRow->addWidget(footRight);
    layout->addLayout(footerRow);

    scrollArea->setWidget(contentWidget);
    rootLayout->addWidget(scrollArea);
}

void DashboardPage::updateMetrics(int totalProfiles, int runningProfiles,
                                 int totalProxies, int liveProxies,
                                 const QString &licenseStatus, const QString &licenseDays) {
    if (m_lblTotalProfiles) {
        m_lblTotalProfiles->setText(QString("%1 Profiles").arg(totalProfiles));
    }
    if (m_lblRunningProfiles) {
        m_lblRunningProfiles->setText(QString("🟢 Đang chạy: %1 tab").arg(runningProfiles));
    }
    if (m_lblTotalProxies) {
        m_lblTotalProxies->setText(QString("%1 Proxies").arg(totalProxies));
    }
    if (m_lblLiveProxies) {
        m_lblLiveProxies->setText(QString("⚡ Live: %1 | Sẵn sàng").arg(liveProxies));
    }
    if (m_lblLicenseStatus) {
        m_lblLicenseStatus->setText(licenseStatus);
    }
    if (m_lblLicenseDays) {
        m_lblLicenseDays->setText(licenseDays);
    }
}
