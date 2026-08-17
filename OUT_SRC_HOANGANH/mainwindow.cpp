#include "mainwindow.h"
#include "VirtualKeyboard.h"

#include <algorithm>
#include <limits>

#include <QApplication>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QValueAxis>
#include <QCheckBox>
#include <QComboBox>
#include <QDate>
#include <QDateTime>
#include <QDialog>
#include <QFormLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QJsonDocument>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QNetworkReply>
#include <QPainter>
#include <QPushButton>
#include <QScrollArea>
#include <QStackedWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTimer>
#include <QUrlQuery>
#include <QVBoxLayout>

namespace {
QString friendlyMetricTitle(const QString &key)
{
    if (key == "temperature_c") return QStringLiteral("🌡 Nhiệt độ (°C)");
    if (key == "sound_vpp") return QStringLiteral("🔊 Âm thanh (Vpp)");
    if (key == "pressure_hpa") return QStringLiteral("💨 Áp suất (hPa)");
    if (key == "uv_index") return QStringLiteral("☀ Chỉ số UV");
    if (key == "lux") return QStringLiteral("💡 Cường độ sáng (Lux)");
    if (key == "voltage_v") return QStringLiteral("🔌 Điện áp (V)");
    if (key == "current_a") return QStringLiteral("⚡ Dòng điện (A)");
    if (key == "flow_l_min") return QStringLiteral("🚰 Lưu lượng (L/m)");
    if (key == "total_liters") return QStringLiteral("💧 Tổng nước (L)");
    if (key == "ir_detected") return QStringLiteral("🚨 Cảm biến IR");
    return key;
}

QColor metricChartColor(const QString &key)
{
    if (key == "temperature_c") return QColor("#ff7043");
    if (key == "sound_vpp") return QColor("#ffca28");
    if (key == "pressure_hpa") return QColor("#29b6f6");
    if (key == "uv_index") return QColor("#ab47bc");
    if (key == "lux") return QColor("#ffd54f");
    if (key == "voltage_v") return QColor("#ffee58");
    if (key == "current_a") return QColor("#26a69a");
    if (key == "flow_l_min") return QColor("#26c6da");
    if (key == "total_liters") return QColor("#42a5f5");
    return QColor("#7c5cff");
}

QFrame *panel(const QString &name)
{
    auto *f = new QFrame;
    f->setObjectName(name);
    return f;
}
QLabel *label(const QString &text, const QString &name = {})
{
    auto *l = new QLabel(text);
    if (!name.isEmpty()) l->setObjectName(name);
    l->setWordWrap(true);
    return l;
}
QPushButton *button(const QString &text, const QString &name = {})
{
    auto *b = new QPushButton(text);
    if (!name.isEmpty()) b->setObjectName(name);
    b->setCursor(Qt::PointingHandCursor);
    return b;
}
QString firstMetric(const QJsonObject &device, const QString &key, const QString &unit)
{
    const auto m = device.value("metrics").toObject();
    if (!m.value(key).isDouble()) return QStringLiteral("--");
    return QStringLiteral("%1 %2").arg(m.value(key).toDouble(), 0, 'f', key == "pressure_hpa" ? 0 : 1).arg(unit);
}

bool isDeviceOnline(const QJsonObject &device)
{
    if (!device.value("online").toBool(false)) return false;

    const QString raw = device.value("last_seen_at").toString();
    if (raw.isEmpty()) return true;

    QDateTime last = QDateTime::fromString(raw, Qt::ISODateWithMs);
    if (!last.isValid()) last = QDateTime::fromString(raw, Qt::ISODate);
    if (!last.isValid()) return device.value("online").toBool(false);
    if (last.timeSpec() == Qt::LocalTime) last.setTimeSpec(Qt::UTC);

    return last.secsTo(QDateTime::currentDateTimeUtc()) <= 35;
}

}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("HoangAnh IoT Command Center (7-Inch Edition)"));
    resize(800, 480);
    setMinimumSize(800, 480);

    m_root = new QStackedWidget(this);
    setCentralWidget(m_root);

    setStyleSheet(R"QSS(
        QMainWindow, QWidget#shell { background-color: #070a1e; color: #ecf2ff; }
        QWidget { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, "Helvetica Neue", sans-serif; }
        QLabel { color: #ecf2ff; background: transparent; }
        
        /* === LOGIN PAGE === */
        QFrame#loginWrap { background: qradialgradient(cx:0.2, cy:0.2, radius:1.1, stop:0 #1a1b4b, stop:0.5 #0b112c, stop:1 #060919); }
        QFrame#loginCard { background: #141c42; border: 1.5px solid #4a62b3; border-radius: 14px; }
        QLabel#loginHero { color: #ffffff; font-size: 20px; font-weight: 900; }
        QLabel#loginSub { color: #8fa0dd; font-size: 11px; font-weight: 600; line-height: 14px; }
        QLabel#loginTag { background: rgba(124, 92, 255, 0.25); color: #b794f6; border-radius: 6px; padding: 2px 8px; font-size: 10px; font-weight: 800; }
        QLabel#loginTitle { color: #ffffff; font-size: 16px; font-weight: 900; }
        QLabel#loginHint { color: #8292c2; font-size: 10px; font-weight: 600; }
        
        /* === INPUTS & CONTROLS === */
        QLineEdit, QComboBox { background: #0e1538; color: #ffffff; border: 1px solid #283670; border-radius: 8px; padding: 5px 10px; min-height: 26px; font-size: 12px; font-weight: 700; }
        QLineEdit:focus, QComboBox:focus { border: 2px solid #7c5cff; background: #131d4d; }
        QPushButton { background: #7c5cff; border: none; border-radius: 8px; color: white; padding: 6px 12px; font-weight: 900; font-size: 12px; }
        QPushButton:hover { background: #6945f8; }
        QPushButton:pressed { background: #5633e0; }
        QPushButton#primaryBtn { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #ff7e40, stop:1 #7c5cff); }
        QPushButton#primaryBtn:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #ff6b22, stop:1 #6742f5); }
        QPushButton#ghost { background: rgba(255,255,255,14); border: 1px solid rgba(255,255,255,30); color: #d4e0ff; }
        QPushButton#ghost:hover { background: rgba(255,255,255,24); }
        QPushButton#danger { background: #e63956; }
        QPushButton#danger:hover { background: #cf2743; }
        QPushButton#kbToggleBtn { background: rgba(124, 92, 255, 0.2); border: 1px solid rgba(124, 92, 255, 0.4); color: #bca4ff; font-size: 11px; padding: 3px 8px; border-radius: 6px; }

        /* === VIRTUAL KEYBOARD === */
        QWidget#virtualKeyboard { background: rgba(10, 14, 38, 0.98); border: 1px solid rgba(124, 92, 255, 0.5); border-radius: 12px; }
        QPushButton#kbKey { background: #182250; color: #ffffff; border: 1px solid #2d3b7e; border-radius: 6px; font-size: 13px; font-weight: 800; }
        QPushButton#kbKey:hover { background: #2a3880; border-color: #7c5cff; }
        QPushButton#kbKey:pressed { background: #7c5cff; color: #ffffff; }
        QPushButton#kbKeyNum { background: #131a40; color: #7fffd4; border: 1px solid #24306b; border-radius: 6px; font-size: 12px; font-weight: 800; }
        QPushButton#kbKeyNum:hover { background: #222d69; }
        QPushButton#kbKeyAction { background: #242d5c; color: #ffbe55; border: 1px solid #3c4885; border-radius: 6px; font-size: 12px; font-weight: 800; }
        QPushButton#kbKeyAction:hover { background: #333f7d; }
        QPushButton#kbKeyShift { background: #242d5c; color: #ecf2ff; border: 1px solid #3c4885; border-radius: 6px; font-size: 13px; font-weight: 800; }
        QPushButton#kbKeyShiftActive { background: #ff8a3d; color: #ffffff; border: 1px solid #ffaa6e; border-radius: 6px; font-size: 13px; font-weight: 800; }
        QPushButton#kbKeyMode { background: #1e2857; color: #b9caff; border: 1px solid #37478d; border-radius: 6px; font-size: 12px; font-weight: 900; }
        QPushButton#kbKeySpace { background: #1b2554; border: 1px solid #2c3a7a; border-radius: 6px; }
        QPushButton#kbKeySpace:hover { background: #283677; }
        QPushButton#kbKeyEnter { background: #20c96b; color: #ffffff; border: 1px solid #2ddc7b; border-radius: 6px; font-size: 13px; font-weight: 900; }
        QPushButton#kbKeyEnter:hover { background: #1ab05b; }

        /* === SHELL & NAVIGATION === */
        QFrame#shell { background: #070a1e; }
        QFrame#topbar { background: #0e1434; border: 1px solid #2d3c74; border-radius: 10px; }
        QLabel#brand { font-size: 13px; font-weight: 900; color: #ffffff; }
        QLabel#muted { color: #8292c2; font-size: 11px; font-weight: 600; }
        QPushButton#nav { background: transparent; color: #9ab0e6; border-radius: 8px; padding: 5px 9px; text-align: center; font-size: 11px; font-weight: 800; }
        QPushButton#nav:hover { background: rgba(255,255,255,10); color: #ffffff; }
        QPushButton#nav:checked { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #ff8a3d, stop:1 #7c5cff); color: white; font-weight: 900; }
        QPushButton#logoutBtn { background: rgba(239, 71, 111, 0.2); border: 1px solid rgba(239, 71, 111, 0.4); color: #ff859f; font-size: 11px; padding: 4px 8px; border-radius: 6px; }
        QPushButton#logoutBtn:hover { background: #ef476f; color: white; }

        /* === CARDS & TILES === */
        QFrame#deviceCard {
            background-color: #0f1938;
            border: 2px solid #3c54a4;
            border-radius: 12px;
        }
        QFrame#deviceCard:hover {
            background-color: #14224c;
            border: 2px solid #7c5cff;
        }
        QFrame#availableCard {
            background-color: #0c202d;
            border: 2px solid #20c96b;
            border-radius: 12px;
        }
        QFrame#pageCard, QFrame#metricCard, QFrame#chartCard {
            background-color: #0f1938;
            border: 1.5px solid #2d3c74;
            border-radius: 12px;
        }
        
        /* === BADGES & LABELS === */
        QLabel#pageTitle { font-size: 13px; font-weight: 800; color: #ffbe55; background: transparent; }
        QLabel#deviceName { font-size: 13px; font-weight: 800; color: #ffffff; background: transparent; }
        QLabel#cardIcon { background: #1c264e; color: #ffbe55; border: 1px solid #334478; border-radius: 8px; min-width: 30px; max-width: 30px; min-height: 30px; max-height: 30px; font-size: 14px; font-weight: 900; qproperty-alignment: AlignCenter; }
        QLabel#bigMetric { font-size: 12px; font-weight: 800; color: #7fffd4; background: #080d24; border: 1px solid #283770; border-radius: 6px; padding: 4px 8px; }
        QLabel#kpiVal { font-size: 18px; font-weight: 900; color: #ffffff; }
        QLabel#kpiTitle { font-size: 10px; font-weight: 700; color: #ffbe55; text-transform: uppercase; }
        QLabel#orange { color: #ffbe55; font-weight: 800; }
        QLabel#onlineBadge { background: #20c96b; color: #ffffff; border-radius: 6px; padding: 2px 7px; font-size: 10px; font-weight: 900; }
        QLabel#offlineBadge { background: #555f75; color: #ffffff; border-radius: 6px; padding: 2px 7px; font-size: 10px; font-weight: 900; }
        QLabel#irSafeBadge { background: rgba(32, 201, 107, 0.85); color: #ffffff; border-radius: 8px; padding: 3px 8px; font-size: 10px; font-weight: 800; }
        QLabel#irAlertBadge { background: #ef3f46; color: #ffffff; border-radius: 8px; padding: 3px 8px; font-size: 10px; font-weight: 900; }
        QPushButton#readyBadge { background: #20c96b; color: #ffffff; border-radius: 8px; padding: 5px 10px; font-weight: 900; font-size: 11px; }
        QPushButton#readyBadge:hover { background: #17ad5a; }

        /* === TABLES & CHARTS === */
        QTableWidget { background: #0e1434; color: #ecf2ff; border: 1px solid #2d3c74; border-radius: 10px; gridline-color: rgba(255, 255, 255, 18); selection-background-color: #7c5cff; font-size: 11px; }
        QHeaderView::section { background: #0e163b; color: #b7c7f5; border: none; padding: 6px; font-weight: 900; font-size: 11px; }
        QFrame#miniChartCard { background: #0f1938; border: 1px solid #2d3c74; border-radius: 12px; }
        QLabel#chartTitle { font-size: 12px; font-weight: 900; color: #ffffff; }
        QLabel#chartValueBadge { background: rgba(124, 92, 255, 0.25); color: #7fffd4; border: 1px solid rgba(124, 92, 255, 0.4); border-radius: 4px; padding: 1px 4px; font-size: 10px; font-weight: 800; }
        QPushButton#toggleTab { background: rgba(255, 255, 255, 0.06); border: 1px solid rgba(124, 92, 255, 0.3); border-radius: 7px; padding: 4px 10px; font-size: 11px; font-weight: 800; color: #9ab0e6; }
        QPushButton#toggleTab:hover { background: rgba(255, 255, 255, 0.12); color: #ffffff; }
        QPushButton#toggleTab:checked { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #ff8a3d, stop:1 #7c5cff); color: white; font-weight: 900; border: none; }
        QChartView { background: transparent; border: none; }
        QScrollArea { border: none; background: transparent; }
    )QSS");

    buildLogin();
    buildShell();
    m_root->addWidget(m_loginPage);
    m_root->addWidget(m_shellPage);

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &MainWindow::refreshAll);
}

QNetworkRequest MainWindow::request(const QString &path) const
{
    QNetworkRequest req(QUrl(m_baseUrl + path));
    req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    req.setTransferTimeout(3500);
    if (!m_token.isEmpty()) req.setRawHeader("Authorization", "Bearer " + m_token.toUtf8());
    return req;
}

void MainWindow::get(const QString &path, std::function<void(QJsonObject)> ok)
{
    auto *reply = m_net.get(request(path));
    connect(reply, &QNetworkReply::finished, this, [this, reply, ok] {
        const auto body = reply->readAll();
        if (reply->error() == QNetworkReply::NoError) ok(QJsonDocument::fromJson(body).object());
        else if (m_status) m_status->setText(reply->errorString());
        reply->deleteLater();
    });
}

void MainWindow::post(const QString &path, const QJsonObject &body, std::function<void(QJsonObject)> ok)
{
    auto *reply = m_net.post(request(path), QJsonDocument(body).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply, ok] {
        const auto body = reply->readAll();
        if (reply->error() == QNetworkReply::NoError) { if (ok) ok(QJsonDocument::fromJson(body).object()); }
        else QMessageBox::warning(this, tr("Lỗi"), reply->errorString());
        reply->deleteLater();
    });
}

void MainWindow::del(const QString &path, std::function<void(QJsonObject)> ok)
{
    auto *reply = m_net.deleteResource(request(path));
    connect(reply, &QNetworkReply::finished, this, [this, reply, ok] {
        const auto body = reply->readAll();
        if (reply->error() == QNetworkReply::NoError) { if (ok) ok(QJsonDocument::fromJson(body).object()); }
        else QMessageBox::warning(this, tr("Lỗi"), reply->errorString());
        reply->deleteLater();
    });
}

void MainWindow::logout()
{
    m_timer->stop();
    m_token.clear();
    m_role.clear();
    m_username.clear();
    m_root->setCurrentWidget(m_loginPage);
    if (m_loginKeyboard) m_loginKeyboard->show();
}

void MainWindow::buildLogin()
{
    m_loginPage = panel("loginWrap");
    auto *mainVBox = new QVBoxLayout(m_loginPage);
    mainVBox->setContentsMargins(12, 10, 12, 8);
    mainVBox->setSpacing(6);

    // ---- TOP SECTION: Brand banner (Left) & Compact Login Card (Right) ----
    auto *topRow = new QHBoxLayout;
    topRow->setSpacing(12);

    // Left Banner
    auto *heroCard = panel("loginCard");
    auto *heroL = new QVBoxLayout(heroCard);
    heroL->setContentsMargins(16, 12, 16, 12);
    heroL->setSpacing(6);
    
    auto *tagRow = new QHBoxLayout;
    tagRow->addWidget(label("RASPBERRY PI 7\"", "loginTag"));
    tagRow->addWidget(label("MQTT ENGINE", "loginTag"));
    tagRow->addStretch();
    heroL->addLayout(tagRow);

    heroL->addWidget(label("HOANGANH\nIoT COMMAND CENTER", "loginHero"));
    heroL->addWidget(label("Hệ thống quản lý, giám sát và điều khiển thiết bị thông minh qua WiFi & MQTT.", "loginSub"));
    heroL->addStretch();

    // Right Card: Login Form
    auto *card = panel("loginCard");
    card->setFixedWidth(340);
    auto *cl = new QVBoxLayout(card);
    cl->setContentsMargins(16, 10, 16, 10);
    cl->setSpacing(6);

    auto *cardHead = new QHBoxLayout;
    cardHead->addWidget(label("ĐĂNG NHẬP", "loginTitle"));
    cardHead->addStretch();
    auto *kbToggle = button("⌨ Bàn phím", "kbToggleBtn");
    cardHead->addWidget(kbToggle);
    cl->addLayout(cardHead);

    auto *u = new QLineEdit;
    u->setPlaceholderText("Tài khoản (username)");
    u->setText("admin"); // Default convenience for testing

    auto *p = new QLineEdit;
    p->setPlaceholderText("Mật khẩu (password)");
    p->setEchoMode(QLineEdit::Password);
    p->setText("admin");

    auto *passRow = new QHBoxLayout;
    passRow->setSpacing(4);
    passRow->addWidget(p, 1);
    auto *eyeBtn = button("👁", "ghost");
    eyeBtn->setFixedSize(30, 28);
    eyeBtn->setToolTip(tr("Xem/Ẩn mật khẩu"));
    connect(eyeBtn, &QPushButton::clicked, this, [p] {
        p->setEchoMode(p->echoMode() == QLineEdit::Password ? QLineEdit::Normal : QLineEdit::Password);
    });
    passRow->addWidget(eyeBtn);

    auto *loginBtn = button("VÀO HỆ THỐNG ➔", "primaryBtn");
    loginBtn->setFixedHeight(32);

    cl->addWidget(u);
    cl->addLayout(passRow);
    cl->addWidget(loginBtn);

    topRow->addWidget(heroCard, 1);
    topRow->addWidget(card);
    mainVBox->addLayout(topRow, 1);

    // ---- BOTTOM SECTION: Built-in Touchscreen Virtual Keyboard ----
    m_loginKeyboard = new VirtualKeyboard(m_loginPage);
    m_loginKeyboard->attachTo(u); // Default target is username
    mainVBox->addWidget(m_loginKeyboard, 0);

    // Toggle keyboard visibility
    connect(kbToggle, &QPushButton::clicked, this, [this] {
        m_loginKeyboard->setVisible(!m_loginKeyboard->isVisible());
    });

    // Auto-focus listener: switch keyboard target when input is tapped/clicked
    connect(qApp, &QApplication::focusChanged, this, [this, u, p](QWidget *, QWidget *now) {
        if (now == u) {
            m_loginKeyboard->attachTo(u);
            m_loginKeyboard->show();
        } else if (now == p) {
            m_loginKeyboard->attachTo(p);
            m_loginKeyboard->show();
        }
    });

    // Submit handler
    auto doLogin = [=] {
        post("/api/auth/login", {{"username", u->text().trimmed()}, {"password", p->text()}}, [=](QJsonObject obj) {
            m_token = obj.value("token").toString();
            const auto user = obj.value("user").toObject();
            m_role = user.value("role").toString();
            m_username = user.value("username").toString(u->text());
            const bool isAdmin = (m_role == "admin");
            const auto navButtons = m_shellPage->findChildren<QPushButton*>("nav");
            for (auto *nav : navButtons) {
                if (nav->text().contains(QStringLiteral("Tài khoản"))) {
                    nav->setVisible(isAdmin);
                    nav->setEnabled(isAdmin);
                    nav->setChecked(false);
                } else if (nav->text().contains(QStringLiteral("Tổng quan"))) {
                    nav->setChecked(true);
                } else {
                    nav->setChecked(false);
                }
            }
            setPage(0);
            m_root->setCurrentWidget(m_shellPage);
            m_timer->start(2500);
            refreshAll();
        });
    };

    connect(loginBtn, &QPushButton::clicked, this, doLogin);
    connect(p, &QLineEdit::returnPressed, this, doLogin);
    connect(m_loginKeyboard, &VirtualKeyboard::enterPressed, this, doLogin);
}

void MainWindow::buildShell()
{
    m_shellPage = panel("shell");
    auto *root = new QVBoxLayout(m_shellPage);
    root->setContentsMargins(10, 8, 10, 8);
    root->setSpacing(8);

    // Slim Top Header Bar (Height ~40px)
    auto *top = panel("topbar");
    auto *tl = new QHBoxLayout(top);
    tl->setContentsMargins(12, 4, 12, 4);
    tl->setSpacing(8);

    tl->addWidget(label("HA COMMAND", "brand"));

    const QStringList names{"◆ Tổng quan", "▣ Thiết bị", "▥ Thống kê", "♙ Tài khoản"};
    for (int i = 0; i < names.size(); ++i) {
        auto *b = button(names[i], "nav");
        b->setCheckable(true);
        if (i == 0) b->setChecked(true);
        connect(b, &QPushButton::clicked, this, [this, i, b] {
            setPage(i);
            b->setChecked(true);
        });
        tl->addWidget(b);
    }
    tl->addStretch();

    m_status = label("offline", "muted");
    tl->addWidget(m_status);

    auto *outBtn = button("⎋ Thoát", "logoutBtn");
    connect(outBtn, &QPushButton::clicked, this, &MainWindow::logout);
    tl->addWidget(outBtn);

    root->addWidget(top);

    m_pages = new QStackedWidget;
    buildHome();
    buildDevices();
    buildHistory();
    buildUsers();
    root->addWidget(m_pages, 1);
}

void MainWindow::setPage(int index)
{
    m_pages->setCurrentIndex(index);
    refreshAll();
}

void MainWindow::buildHome()
{
    auto *page = new QWidget;
    auto *root = new QVBoxLayout(page);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(8);

    // Row 1: KPI Stats Bar
    auto *kpiRow = new QHBoxLayout;
    kpiRow->setSpacing(8);

    auto makeKpi = [](const QString &t, const QString &icon) {
        auto *f = panel("metricCard");
        f->setFixedHeight(54);
        auto *h = new QHBoxLayout(f);
        h->setContentsMargins(10, 4, 10, 4);
        h->setSpacing(8);
        h->addWidget(label(icon, "cardIcon"));
        auto *v = new QVBoxLayout;
        v->setSpacing(0);
        v->addWidget(label(t, "kpiTitle"));
        auto *val = label("--", "kpiVal");
        v->addWidget(val);
        h->addLayout(v, 1);
        return qMakePair(f, val);
    };

    auto a = makeKpi("TỔNG THIẾT BỊ", "⊞");
    auto b = makeKpi("ONLINE", "●");
    auto c = makeKpi("LOẠI CẢM BIẾN", "◈");
    m_kpiDevices = a.second;
    m_kpiOnline = b.second;
    m_kpiType = c.second;

    kpiRow->addWidget(a.first);
    kpiRow->addWidget(b.first);
    kpiRow->addWidget(c.first);
    root->addLayout(kpiRow);

    // Row 2: Live Device Console Card with Scroll Area
    auto *hero = panel("pageCard");
    auto *hl = new QVBoxLayout(hero);
    hl->setContentsMargins(10, 8, 10, 8);
    hl->setSpacing(6);

    auto *head = new QHBoxLayout;
    m_homeTitle = label("Trạng thái thiết bị trực tiếp", "pageTitle");
    head->addWidget(m_homeTitle);
    head->addStretch();
    auto *refBtn = button("⟳ Làm mới", "ghost");
    connect(refBtn, &QPushButton::clicked, this, &MainWindow::refreshDevices);
    head->addWidget(refBtn);
    hl->addLayout(head);

    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    auto *wrap = new QWidget;
    m_homeDeviceGrid = new QGridLayout(wrap);
    m_homeDeviceGrid->setSpacing(8);
    m_homeDeviceGrid->setContentsMargins(2, 2, 2, 2);
    scroll->setWidget(wrap);

    hl->addWidget(scroll, 1);
    root->addWidget(hero, 1);

    m_pages->addWidget(page);
}

void MainWindow::buildDevices()
{
    auto *page = new QWidget;
    page->setStyleSheet("background-color: #070a1e;");
    auto *root = new QVBoxLayout(page);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(8);

    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet("background-color: #070a1e; border: none;");

    auto *wrap = new QWidget;
    wrap->setStyleSheet("background-color: #070a1e;");
    auto *vl = new QVBoxLayout(wrap);
    vl->setContentsMargins(4, 4, 4, 4);
    vl->setSpacing(8);

    vl->addWidget(label("Danh sách thiết bị đã ghép nối", "pageTitle"));
    m_deviceGrid = new QGridLayout;
    m_deviceGrid->setSpacing(8);
    vl->addLayout(m_deviceGrid);

    vl->addWidget(label("Thiết bị mới phát hiện (chưa liên kết)", "pageTitle"));
    m_availableGrid = new QGridLayout;
    m_availableGrid->setSpacing(8);
    vl->addLayout(m_availableGrid);
    vl->addStretch();

    scroll->setWidget(wrap);
    root->addWidget(scroll, 1);
    m_pages->addWidget(page);
}

void MainWindow::buildHistory()
{
    auto *page = new QWidget;
    page->setStyleSheet("background-color: #070a1e;");
    auto *root = new QVBoxLayout(page);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(6);

    // Top Controls Bar
    auto *bar = new QHBoxLayout;
    bar->addWidget(label("Phòng Phân Tích & Thống Kê", "pageTitle"));
    bar->addStretch();

    m_historyDevice = new QComboBox;
    m_historyDevice->setMinimumWidth(160);

    m_historyPeriod = new QComboBox;
    m_historyPeriod->addItem("Theo Ngày", "day");
    m_historyPeriod->addItem("Theo Tháng", "month");
    m_historyPeriod->addItem("Theo Năm", "year");

    auto *search = button("⟳ Quét dữ liệu", "primaryBtn");
    search->setFixedHeight(28);

    // View Mode Toggle (Charts vs Table)
    auto *chartToggle = button("📈 Biểu đồ", "toggleTab");
    auto *tableToggle = button("📋 Bảng số liệu", "toggleTab");
    chartToggle->setCheckable(true);
    tableToggle->setCheckable(true);
    chartToggle->setChecked(true);

    bar->addWidget(m_historyDevice);
    bar->addWidget(m_historyPeriod);
    bar->addWidget(search);
    bar->addSpacing(6);
    bar->addWidget(chartToggle);
    bar->addWidget(tableToggle);
    root->addLayout(bar);

    // Main Stack: Page 0 = Scrollable Charts Grid, Page 1 = Full Data Table
    m_historyStack = new QStackedWidget;
    m_historyStack->setStyleSheet("background-color: #070a1e;");

    // Page 0: Charts Grid with Scroll Area
    auto *chartsScroll = new QScrollArea;
    chartsScroll->setWidgetResizable(true);
    chartsScroll->setFrameShape(QFrame::NoFrame);
    chartsScroll->setStyleSheet("background-color: #070a1e; border: none;");
    auto *chartsWrap = new QWidget;
    chartsWrap->setStyleSheet("background-color: #070a1e;");
    m_historyCharts = new QGridLayout(chartsWrap);
    m_historyCharts->setSpacing(8);
    m_historyCharts->setContentsMargins(2, 2, 2, 2);
    chartsScroll->setWidget(chartsWrap);
    m_historyStack->addWidget(chartsScroll);

    // Page 1: Data Table
    m_historyTable = new QTableWidget;
    m_historyTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_historyTable->verticalHeader()->hide();
    m_historyStack->addWidget(m_historyTable);

    root->addWidget(m_historyStack, 1);

    connect(chartToggle, &QPushButton::clicked, this, [=] {
        chartToggle->setChecked(true);
        tableToggle->setChecked(false);
        m_historyStack->setCurrentIndex(0);
    });

    connect(tableToggle, &QPushButton::clicked, this, [=] {
        tableToggle->setChecked(true);
        chartToggle->setChecked(false);
        m_historyStack->setCurrentIndex(1);
    });

    connect(search, &QPushButton::clicked, this, &MainWindow::refreshHistory);
    connect(m_historyDevice, &QComboBox::currentIndexChanged, this, [this](int) { refreshHistory(); });

    m_pages->addWidget(page);
}

void MainWindow::buildUsers()
{
    auto *page = new QWidget;
    page->setStyleSheet("background-color: #070a1e;");
    auto *root = new QVBoxLayout(page);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(8);

    auto *top = new QHBoxLayout;
    top->addWidget(label("Quản lý tài khoản người dùng", "pageTitle"));
    top->addStretch();
    auto *add = button("+ Tạo người dùng", "primaryBtn");
    top->addWidget(add);
    root->addLayout(top);

    m_usersTable = new QTableWidget(0, 4);
    m_usersTable->setHorizontalHeaderLabels({"Tài khoản", "Quyền", "Thiết bị", "Trạng thái"});
    m_usersTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_usersTable->verticalHeader()->hide();
    root->addWidget(m_usersTable);

    connect(add, &QPushButton::clicked, this, &MainWindow::createUserDialog);
    connect(m_usersTable, &QTableWidget::cellClicked, this, [this](int row, int) {
        if (row >= 0 && row < m_users.size()) editUserDialog(m_users.at(row).toObject());
    });

    m_pages->addWidget(page);
}

QString MainWindow::deviceIcon(const QString &type)
{
    if (type == "temperature_sound") return "♪";
    if (type == "weather_pressure") return "☁";
    if (type == "uv_pressure") return "☼";
    if (type == "electric_power") return "⚡";
    if (type == "water_flow_pump") return "≈";
    return "◆";
}

QString MainWindow::deviceTypeName(const QString &type)
{
    if (type == "temperature_sound") return "Nhiệt độ & Âm thanh";
    if (type == "weather_pressure") return "Môi trường & Áp suất";
    if (type == "uv_pressure") return "UV & Áp suất";
    if (type == "electric_power") return "Đo điện";
    if (type == "water_flow_pump") return "Bơm & Lưu lượng";
    return "Thiết bị IoT";
}

QString MainWindow::metricText(const QJsonObject &d)
{
    const QString type = d.value("device_type").toString();
    const auto m = d.value("metrics").toObject();
    QStringList items;

    if (type == "temperature_sound") {
        if (m.value("temperature_c").isDouble())
            items << QStringLiteral("Nhiệt độ: %1°C").arg(m.value("temperature_c").toDouble(), 0, 'f', 1);
        if (m.value("sound_vpp").isDouble())
            items << QStringLiteral("Âm thanh: %1 V").arg(m.value("sound_vpp").toDouble(), 0, 'f', 2);
    } else if (type == "weather_pressure") {
        if (m.value("temperature_c").isDouble())
            items << QStringLiteral("Nhiệt độ: %1°C").arg(m.value("temperature_c").toDouble(), 0, 'f', 1);
        if (m.value("pressure_hpa").isDouble())
            items << QStringLiteral("Áp suất: %1 hPa").arg(m.value("pressure_hpa").toDouble(), 0, 'f', 0);
        if (m.contains("ir_detected")) {
            const bool detected = m.value("ir_detected").toDouble() >= 0.5;
            items << (detected ? QStringLiteral("Vật cản: [CÓ VẬT]") : QStringLiteral("Vật cản: [Không]"));
        }
        if (m.value("lux").isDouble())
            items << QStringLiteral("Ánh sáng: %1 Lux").arg(m.value("lux").toDouble(), 0, 'f', 0);
    } else if (type == "uv_pressure") {
        if (m.value("uv_index").isDouble())
            items << QStringLiteral("UV: %1").arg(m.value("uv_index").toDouble(), 0, 'f', 1);
        if (m.value("lux").isDouble())
            items << QStringLiteral("Ánh sáng: %1 Lux").arg(m.value("lux").toDouble(), 0, 'f', 0);
        if (m.value("pressure_hpa").isDouble())
            items << QStringLiteral("Áp suất: %1 hPa").arg(m.value("pressure_hpa").toDouble(), 0, 'f', 0);
    } else if (type == "electric_power") {
        if (m.value("voltage_v").isDouble())
            items << QStringLiteral("Điện áp: %1 V").arg(m.value("voltage_v").toDouble(), 0, 'f', 1);
        if (m.value("current_a").isDouble())
            items << QStringLiteral("Dòng điện: %1 A").arg(m.value("current_a").toDouble(), 0, 'f', 2);
    } else if (type == "water_flow_pump") {
        if (m.value("flow_l_min").isDouble())
            items << QStringLiteral("Lưu lượng: %1 L/m").arg(m.value("flow_l_min").toDouble(), 0, 'f', 1);
        if (m.value("total_liters").isDouble())
            items << QStringLiteral("Tổng: %1 L").arg(m.value("total_liters").toDouble(), 0, 'f', 1);
    }

    if (m.contains("ir_detected") && type != "weather_pressure") {
        const bool detected = m.value("ir_detected").toDouble() >= 0.5;
        items << (detected ? QStringLiteral("Vật cản: [CÓ VẬT]") : QStringLiteral("Vật cản: [Không]"));
    }

    if (items.isEmpty()) {
        for (auto it = m.begin(); it != m.end(); ++it) {
            if (it.value().isDouble() && it.key() != "ir_detected") {
                items << QStringLiteral("%1: %2").arg(it.key()).arg(it.value().toDouble(), 0, 'f', 1);
            }
        }
    }

    return items.isEmpty() ? QStringLiteral("--") : items.join("   |   ");
}

void MainWindow::refreshAll()
{
    refreshDevices();
    if (m_role == "admin" && m_pages && m_pages->currentIndex() == 3) refreshUsers();
    if (m_pages && m_pages->currentIndex() == 2) refreshHistory();
}

void MainWindow::refreshDevices()
{
    get("/api/devices/me", [this](QJsonObject o) {
        m_devices = o.value("data").toArray();
        renderDevices();
        refreshAvailable();
    });
}

void MainWindow::refreshAvailable()
{
    get("/api/devices/available", [this](QJsonObject o) {
        m_available = o.value("data").toArray();
        renderAvailable();
    });
}

void MainWindow::refreshUsers()
{
    if (m_role != "admin") return;
    get("/api/admin/users", [this](QJsonObject o) {
        m_users = o.value("data").toArray();
        renderUsers();
    });
}

void MainWindow::renderDevices()
{
    m_homeTitle->setText("Thiết bị của " + m_username.toUpper());
    m_kpiDevices->setText(QString::number(m_devices.size()));
    int online = 0;
    QSet<QString> types;

    QStringList currentIds;
    for (const auto &v : m_devices) {
        auto d = v.toObject();
        currentIds << d.value("device_id").toString();
        if (isDeviceOnline(d)) online++;
        types.insert(d.value("device_type").toString());
    }

    m_kpiOnline->setText(QString::number(online));
    m_kpiType->setText(QString::number(types.size()));
    m_status->setText("Đồng bộ: " + QTime::currentTime().toString("HH:mm:ss"));

    // Check if we can do an in-place update (prevents screen flicker)
    bool canUpdateInPlace = (m_deviceGrid->count() == m_devices.size());
    if (canUpdateInPlace) {
        for (int idx = 0; idx < m_devices.size(); ++idx) {
            auto *item = m_deviceGrid->itemAt(idx);
            if (!item || !item->widget() || item->widget()->property("deviceId").toString() != currentIds[idx]) {
                canUpdateInPlace = false;
                break;
            }
        }
    }

    if (canUpdateInPlace) {
        // Fast in-place update with ZERO flickering
        for (int idx = 0; idx < m_devices.size(); ++idx) {
            auto d = m_devices[idx].toObject();
            const bool onlineNow = isDeviceOnline(d);

            for (auto *grid : {m_deviceGrid, m_homeDeviceGrid}) {
                if (!grid) continue;
                auto *item = grid->itemAt(idx);
                if (!item || !item->widget()) continue;
                auto *card = item->widget();

                auto *metricLbl = card->findChild<QLabel*>("metricVal");
                if (metricLbl) metricLbl->setText(metricText(d));

                auto *badgeLbl = card->findChild<QLabel*>("statusVal");
                if (badgeLbl) {
                    badgeLbl->setText(onlineNow ? "ONLINE" : "OFFLINE");
                    badgeLbl->setObjectName(onlineNow ? "onlineBadge" : "offlineBadge");
                    badgeLbl->style()->unpolish(badgeLbl);
                    badgeLbl->style()->polish(badgeLbl);
                }

                auto *relayBtn = card->findChild<QPushButton*>("relayBtn");
                if (relayBtn) {
                    const bool relayOn = d.value("state").toObject().value("relay").toBool(false);
                    const bool isPump = (d.value("device_type").toString() == "water_flow_pump");
                    relayBtn->setText(isPump ? (relayOn ? "Bơm: BẬT" : "Bơm: TẮT") : (relayOn ? "Relay: BẬT" : "Relay: TẮT"));
                    relayBtn->setEnabled(onlineNow);
                }
            }
        }
        return;
    }

    // Full rebuild only when device count or IDs actually change
    while (auto *it = m_deviceGrid->takeAt(0)) {
        if (it->widget()) it->widget()->deleteLater();
        delete it;
    }
    if (m_homeDeviceGrid) {
        while (auto *it = m_homeDeviceGrid->takeAt(0)) {
            if (it->widget()) it->widget()->deleteLater();
            delete it;
        }
    }

    const QString selectedHistoryDevice = m_historyDevice->currentData().toString();
    m_historyDevice->blockSignals(true);
    m_historyDevice->clear();

    auto makeDeviceCard = [this](const QJsonObject &d) {
        auto *card = panel("deviceCard");
        card->setStyleSheet("QFrame#deviceCard { background-color: #0f1938; border: 1.5px solid #3c54a4; border-radius: 10px; } "
                            "QFrame#deviceCard:hover { border: 1.5px solid #7c5cff; background-color: #14224c; }");
        card->setProperty("deviceId", d.value("device_id").toString());
        auto *l = new QVBoxLayout(card);
        l->setContentsMargins(10, 8, 10, 8);
        l->setSpacing(6);

        auto *top = new QHBoxLayout;
        top->addWidget(label(deviceIcon(d.value("device_type").toString()), "cardIcon"));
        
        auto *titleBox = new QVBoxLayout;
        titleBox->setSpacing(1);
        titleBox->addWidget(label(d.value("name").toString(d.value("device_id").toString()), "deviceName"));
        titleBox->addWidget(label(deviceTypeName(d.value("device_type").toString()) + " · " + d.value("device_id").toString(), "muted"));
        top->addLayout(titleBox, 1);

        const bool onlineNow = isDeviceOnline(d);
        auto *statusBadge = label(onlineNow ? "ONLINE" : "OFFLINE", onlineNow ? "onlineBadge" : "offlineBadge");
        statusBadge->setObjectName("statusVal");
        top->addWidget(statusBadge);
        l->addLayout(top);

        auto *metricLbl = label(metricText(d), "bigMetric");
        metricLbl->setObjectName("metricVal");
        l->addWidget(metricLbl);

        auto *row = new QHBoxLayout;
        auto *cfg = button("⚙ Ngưỡng", "ghost");
        cfg->setFixedHeight(26);
        auto *rel = button("Gỡ", "danger");
        rel->setFixedHeight(26);
        row->addWidget(cfg);
        row->addWidget(rel);

        bool hasRelay = false;
        for (const auto &cap : d.value("capabilities").toArray()) {
            if (cap.toString() == "relay") hasRelay = true;
        }
        if (hasRelay) {
            const bool relayOn = d.value("state").toObject().value("relay").toBool(false);
            auto *r = button((d.value("device_type").toString() == "water_flow_pump") ? (relayOn ? "Bơm: BẬT" : "Bơm: TẮT") : (relayOn ? "Relay: BẬT" : "Relay: TẮT"));
            r->setObjectName("relayBtn");
            r->setFixedHeight(26);
            r->setEnabled(onlineNow);
            row->addWidget(r);
            connect(r, &QPushButton::clicked, this, [=] {
                if (onlineNow) toggleRelay(d.value("device_id").toString(), !relayOn);
            });
        }
        row->addStretch();
        l->addLayout(row);

        connect(cfg, &QPushButton::clicked, this, [=] { openDeviceConfigDialog(d); });
        connect(rel, &QPushButton::clicked, this, [=] { releaseDevice(d.value("device_id").toString()); });
        return card;
    };

    int i = 0;
    for (const auto &v : m_devices) {
        auto d = v.toObject();
        m_historyDevice->addItem(d.value("name").toString(d.value("device_id").toString()) + " · " + d.value("device_id").toString(), d.value("device_id").toString());
        m_deviceGrid->addWidget(makeDeviceCard(d), i / 2, i % 2);
        if (m_homeDeviceGrid) m_homeDeviceGrid->addWidget(makeDeviceCard(d), i / 2, i % 2);
        i++;
    }

    if (!selectedHistoryDevice.isEmpty()) {
        const int keepIndex = m_historyDevice->findData(selectedHistoryDevice);
        if (keepIndex >= 0) m_historyDevice->setCurrentIndex(keepIndex);
    }
    m_historyDevice->blockSignals(false);
}

void MainWindow::renderAvailable()
{
    // Check if available devices changed
    QStringList currentAvailIds;
    for (const auto &v : m_available) {
        currentAvailIds << v.toObject().value("device_id").toString();
    }

    bool canSkip = (m_availableGrid->count() == m_available.size());
    if (canSkip) {
        for (int idx = 0; idx < m_available.size(); ++idx) {
            auto *item = m_availableGrid->itemAt(idx);
            if (!item || !item->widget() || item->widget()->property("deviceId").toString() != currentAvailIds[idx]) {
                canSkip = false;
                break;
            }
        }
    }
    if (canSkip) return;

    while (auto *it = m_availableGrid->takeAt(0)) {
        if (it->widget()) it->widget()->deleteLater();
        delete it;
    }

    int i = 0;
    for (const auto &v : m_available) {
        const auto d = v.toObject();
        auto *card = panel("availableCard");
        card->setStyleSheet("QFrame#availableCard { background-color: #0c202d; border: 1.5px solid #20c96b; border-radius: 10px; }");
        card->setProperty("deviceId", d.value("device_id").toString());
        card->setMinimumHeight(80);

        auto *row = new QHBoxLayout(card);
        row->setContentsMargins(10, 8, 10, 8);
        row->setSpacing(10);

        row->addWidget(label(deviceIcon(d.value("device_type").toString()), "cardIcon"));

        auto *info = new QVBoxLayout;
        info->setSpacing(2);
        info->addWidget(label(d.value("device_id").toString(), "deviceName"));
        info->addWidget(label(deviceTypeName(d.value("device_type").toString()), "muted"));
        row->addLayout(info, 1);

        auto *add = button("+ Ghép nối", "readyBadge");
        add->setFixedHeight(30);
        row->addWidget(add);

        auto openClaim = [this, d] {
            openClaimDeviceDialog(d);
        };
        connect(add, &QPushButton::clicked, this, openClaim);

        m_availableGrid->addWidget(card, i / 2, i % 2);
        i++;
    }
}

void MainWindow::claimDevice(const QString &id, const QString &name)
{
    post("/api/devices/claim", {{"device_id", id}, {"name", name}}, [this](QJsonObject) {
        refreshDevices();
        QTimer::singleShot(350, this, &MainWindow::refreshDevices);
    });
}

void MainWindow::releaseDevice(const QString &id)
{
    post("/api/devices/release", {{"device_id", id}}, [this](QJsonObject) { refreshDevices(); });
}

void MainWindow::toggleRelay(const QString &id, bool state)
{
    post("/api/devices/relay", {{"device_id", id}, {"state", state}}, [this](QJsonObject) { refreshDevices(); });
}

void MainWindow::refreshHistory()
{
    const QString id = m_historyDevice->currentData().toString();
    if (id.isEmpty()) return;
    QUrlQuery q;
    q.addQueryItem("device_id", id);
    q.addQueryItem("period", m_historyPeriod->currentData().toString());
    q.addQueryItem("date", QDate::currentDate().toString(Qt::ISODate));
    q.addQueryItem("limit", "160");
    get("/api/devices/history?" + q.toString(QUrl::FullyEncoded), [this](QJsonObject o) { renderHistory(o); });
}

void MainWindow::renderHistory(const QJsonObject &h)
{
    m_lastHistory = h;
    while (auto *it = m_historyCharts->takeAt(0)) {
        if (it->widget()) it->widget()->deleteLater();
        delete it;
    }
    const auto keys = h.value("metric_keys").toArray(), rows = h.value("data").toArray();
    QStringList headers{"Thời gian"};
    for (auto k : keys) headers << friendlyMetricTitle(k.toString());
    m_historyTable->setColumnCount(headers.size());
    m_historyTable->setHorizontalHeaderLabels(headers);
    m_historyTable->setRowCount(rows.size());

    for (int r = 0; r < rows.size(); ++r) {
        auto e = rows[r].toObject();
        QString timeStr = QDateTime::fromString(e.value("recorded_at").toString(), Qt::ISODateWithMs).toLocalTime().toString("HH:mm:ss dd/MM");
        if (timeStr.isEmpty()) timeStr = e.value("recorded_at").toString();
        m_historyTable->setItem(r, 0, new QTableWidgetItem(timeStr));
        auto m = e.value("metrics").toObject();
        for (int c = 0; c < keys.size(); ++c)
            m_historyTable->setItem(r, c + 1, new QTableWidgetItem(QString::number(m.value(keys[c].toString()).toDouble(), 'f', 2)));
    }

    for (auto k : keys) {
        QString key = k.toString();
        if (key == "ir_detected") continue; // IR is boolean, no need for bar chart

        QStringList cats;
        double minValue = std::numeric_limits<double>::max();
        double maxValue = std::numeric_limits<double>::lowest();
        double latestValue = 0.0;
        bool hasLatest = false;

        const int maxBars = 18;
        const int totalPoints = rows.size();
        const int startIdx = std::max(0, totalPoints - maxBars);

        QVector<double> values;
        for (int r = startIdx; r < totalPoints; ++r) {
            auto e = rows[r].toObject();
            QString timeLabel = QDateTime::fromString(e.value("recorded_at").toString(), Qt::ISODateWithMs).toLocalTime().toString("HH:mm:ss");
            if (timeLabel.isEmpty()) timeLabel = e.value("recorded_at").toString();
            cats << timeLabel;

            const double value = e.value("metrics").toObject().value(key).toDouble();
            latestValue = value;
            hasLatest = true;
            values.append(value);
            minValue = std::min(minValue, value);
            maxValue = std::max(maxValue, value);
        }

        // Check if card for this metric key already exists (in-place update = ZERO flicker)
        QWidget *existingCard = nullptr;
        for (int i = 0; i < m_historyCharts->count(); ++i) {
            auto *item = m_historyCharts->itemAt(i);
            if (item && item->widget() && item->widget()->property("metricKey").toString() == key) {
                existingCard = item->widget();
                break;
            }
        }

        if (existingCard) {
            auto *valBadge = existingCard->findChild<QLabel*>("valBadge");
            if (valBadge && hasLatest) {
                valBadge->setText(QString::number(latestValue, 'f', 1));
            }

            auto *view = existingCard->findChild<QChartView*>();
            if (view && view->chart()) {
                auto *chart = view->chart();
                if (!chart->series().isEmpty()) {
                    auto *series = qobject_cast<QBarSeries*>(chart->series().first());
                    if (series && !series->barSets().isEmpty()) {
                        auto *set = series->barSets().first();
                        set->remove(0, set->count());
                        for (double v : values) *set << v;
                    }
                }
                const auto axes = chart->axes();
                for (auto *axObj : axes) {
                    if (auto *ax = qobject_cast<QBarCategoryAxis*>(axObj)) {
                        ax->setCategories(cats);
                    } else if (auto *ay = qobject_cast<QValueAxis*>(axObj)) {
                        if (key == "pressure_hpa") {
                            ay->setRange(990.0, std::max(1025.0, maxValue + 2.0));
                        } else if (key.contains("uv")) {
                            ay->setRange(0.0, std::max(10.0, maxValue + 1.0));
                        } else if (minValue != std::numeric_limits<double>::max() && maxValue > minValue) {
                            const double pad = std::max(0.5, (maxValue - minValue) * 0.2);
                            ay->setRange(std::max(0.0, minValue - pad), maxValue + pad);
                        }
                    }
                }
            }
            continue;
        }

        // Create new chart card on first appearance
        auto *set = new QBarSet(key);
        set->setColor(metricChartColor(key));
        set->setBorderColor(Qt::transparent);
        for (double v : values) *set << v;

        auto *series = new QBarSeries;
        series->setBarWidth(0.55);
        series->append(set);

        auto *chart = new QChart;
        chart->legend()->hide();
        chart->addSeries(series);
        chart->setBackgroundVisible(false);
        chart->setMargins(QMargins(2, 2, 2, 2));

        auto *ax = new QBarCategoryAxis;
        ax->append(cats);
        ax->setLabelsColor(QColor("#a6b8e8"));
        ax->setLabelsAngle(-35);
        QFont axisFont = ax->labelsFont();
        axisFont.setPointSize(8);
        axisFont.setBold(true);
        ax->setLabelsFont(axisFont);
        ax->setLinePen(QPen(QColor("#2c3868"), 1));
        ax->setGridLinePen(QPen(QColor("#18224c"), 1, Qt::DotLine));

        auto *ay = new QValueAxis;
        ay->setLabelsColor(QColor("#a6b8e8"));
        ay->setLabelsFont(axisFont);
        ay->setLinePen(QPen(QColor("#2c3868"), 1));
        ay->setGridLinePen(QPen(QColor("#18224c"), 1, Qt::DotLine));

        if (key == "pressure_hpa") {
            ay->setRange(990.0, std::max(1025.0, maxValue + 2.0));
        } else if (key.contains("uv")) {
            ay->setRange(0.0, std::max(10.0, maxValue + 1.0));
        } else if (minValue != std::numeric_limits<double>::max() && maxValue > minValue) {
            const double pad = std::max(0.5, (maxValue - minValue) * 0.2);
            ay->setRange(std::max(0.0, minValue - pad), maxValue + pad);
        } else {
            ay->setRange(0.0, std::max(10.0, maxValue + 2.0));
        }

        chart->addAxis(ax, Qt::AlignBottom);
        chart->addAxis(ay, Qt::AlignLeft);
        series->attachAxis(ax);
        series->attachAxis(ay);

        auto *mini = panel("miniChartCard");
        mini->setProperty("metricKey", key);
        mini->setMinimumHeight(175);
        mini->setCursor(Qt::PointingHandCursor);
        mini->setToolTip(tr("Bấm để phóng to biểu đồ này"));
        auto *miniLayout = new QVBoxLayout(mini);
        miniLayout->setContentsMargins(10, 8, 10, 6);
        miniLayout->setSpacing(4);

        auto *cardTop = new QHBoxLayout;
        cardTop->addWidget(label(friendlyMetricTitle(key), "chartTitle"));
        cardTop->addStretch();
        if (hasLatest) {
            auto *valBadge = label(QString::number(latestValue, 'f', 1), "chartValueBadge");
            valBadge->setObjectName("valBadge");
            cardTop->addWidget(valBadge);
        }
        auto *zoomBtn = new QPushButton(QStringLiteral("⛶ Phóng to"), mini);
        zoomBtn->setCursor(Qt::PointingHandCursor);
        zoomBtn->setStyleSheet(QStringLiteral("QPushButton { background: #1c2652; border: 1px solid #3d4f8f; border-radius: 6px; color: #a6b8e8; font-size: 11px; font-weight: bold; padding: 2px 8px; } QPushButton:hover { background: #7c5cff; color: #ffffff; }"));
        connect(zoomBtn, &QPushButton::clicked, this, [this, key] {
            showChartZoomDialog(key);
        });
        cardTop->addWidget(zoomBtn);
        miniLayout->addLayout(cardTop);

        auto *view = new QChartView(chart);
        view->setRenderHint(QPainter::Antialiasing);
        view->setFixedHeight(135);
        view->setCursor(Qt::PointingHandCursor);
        miniLayout->addWidget(view);

        int count = m_historyCharts->count();
        m_historyCharts->addWidget(mini, count / 2, count % 2);
    }
}

void MainWindow::showChartZoomDialog(const QString &key)
{
    if (m_lastHistory.isEmpty()) return;
    const auto rows = m_lastHistory.value("data").toArray();
    if (rows.isEmpty()) return;

    QDialog dialog(this);
    dialog.setWindowTitle(tr("Phóng to biểu đồ"));
    dialog.setModal(true);
    dialog.resize(qMax(400, width() - 30), qMax(300, height() - 30));
    dialog.setStyleSheet(QStringLiteral("QDialog { background-color: #0f1636; border: 2px solid #7c5cff; border-radius: 12px; }"));

    auto *root = new QVBoxLayout(&dialog);
    root->setContentsMargins(14, 12, 14, 12);
    root->setSpacing(10);

    auto *head = new QHBoxLayout;
    auto *title = label(friendlyMetricTitle(key), "chartZoomTitle");
    title->setStyleSheet(QStringLiteral("color: #ffffff; font-size: 17px; font-weight: bold;"));
    head->addWidget(title);
    head->addStretch();
    auto *closeBtn = new QPushButton(QStringLiteral("✕"), &dialog);
    closeBtn->setFixedSize(34, 34);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(QStringLiteral("QPushButton { background: #1c2652; border: 1px solid #3d4f8f; border-radius: 17px; color: #ffffff; font-weight: bold; } QPushButton:hover { background: #ff4d4f; }"));
    head->addWidget(closeBtn);
    root->addLayout(head);

    QStringList cats;
    QList<double> values;
    double minValue = std::numeric_limits<double>::max();
    double maxValue = std::numeric_limits<double>::lowest();
    double sum = 0.0;

    for (int r = rows.size() - 1; r >= 0; --r) {
        auto e = rows[r].toObject();
        QString timeLabel = QDateTime::fromString(e.value("recorded_at").toString(), Qt::ISODateWithMs).toLocalTime().toString("HH:mm:ss");
        if (timeLabel.isEmpty()) timeLabel = e.value("recorded_at").toString();
        cats << timeLabel;

        const double value = e.value("metrics").toObject().value(key).toDouble();
        values.append(value);
        minValue = std::min(minValue, value);
        maxValue = std::max(maxValue, value);
        sum += value;
    }

    auto *set = new QBarSet(key);
    set->setColor(metricChartColor(key));
    set->setBorderColor(Qt::transparent);
    for (double v : values) *set << v;

    auto *series = new QBarSeries;
    series->setBarWidth(0.55);
    series->append(set);

    auto *chart = new QChart;
    chart->legend()->hide();
    chart->addSeries(series);
    chart->setBackgroundVisible(false);
    chart->setMargins(QMargins(4, 4, 4, 4));

    auto *ax = new QBarCategoryAxis;
    ax->append(cats);
    ax->setLabelsColor(QColor("#a6b8e8"));
    ax->setLabelsAngle(-35);
    QFont axisFont = ax->labelsFont();
    axisFont.setPointSize(9);
    axisFont.setBold(true);
    ax->setLabelsFont(axisFont);
    ax->setLinePen(QPen(QColor("#2c3868"), 1));
    ax->setGridLinePen(QPen(QColor("#18224c"), 1, Qt::DotLine));

    auto *ay = new QValueAxis;
    ay->setLabelsColor(QColor("#a6b8e8"));
    ay->setLabelsFont(axisFont);
    ay->setLinePen(QPen(QColor("#2c3868"), 1));
    ay->setGridLinePen(QPen(QColor("#18224c"), 1, Qt::DotLine));

    if (key == "pressure_hpa") {
        ay->setRange(990.0, std::max(1025.0, maxValue + 2.0));
    } else if (key.contains("uv")) {
        ay->setRange(0.0, std::max(10.0, maxValue + 1.0));
    } else if (minValue != std::numeric_limits<double>::max() && maxValue > minValue) {
        const double pad = std::max(0.5, (maxValue - minValue) * 0.2);
        ay->setRange(std::max(0.0, minValue - pad), maxValue + pad);
    } else {
        ay->setRange(0.0, std::max(10.0, maxValue + 2.0));
    }

    chart->addAxis(ax, Qt::AlignBottom);
    chart->addAxis(ay, Qt::AlignLeft);
    series->attachAxis(ax);
    series->attachAxis(ay);

    auto *view = new QChartView(chart);
    view->setRenderHint(QPainter::Antialiasing);
    view->setStyleSheet(QStringLiteral("background: transparent; border: none;"));
    root->addWidget(view, 1);

    auto *bottom = new QHBoxLayout;
    auto *statBadge = label(tr("Tổng: %1 | Min: %2 | Max: %3 | TB: %4")
        .arg(values.size())
        .arg(QString::number(minValue, 'f', 2))
        .arg(QString::number(maxValue, 'f', 2))
        .arg(QString::number(values.isEmpty() ? 0.0 : sum / values.size(), 'f', 2)), "chartZoomStat");
    statBadge->setStyleSheet(QStringLiteral("background: #1c2652; border: 1px solid #3d4f8f; border-radius: 8px; color: #a6b8e8; font-size: 11px; font-weight: bold; padding: 6px 12px;"));
    bottom->addWidget(statBadge);
    bottom->addStretch();
    auto *btn = new QPushButton(tr("Đóng"), &dialog);
    btn->setStyleSheet(QStringLiteral("QPushButton { background: #7c5cff; border: none; border-radius: 8px; color: #ffffff; font-weight: bold; min-height: 32px; padding: 0 16px; } QPushButton:hover { background: #6945f0; }"));
    bottom->addWidget(btn);
    root->addLayout(bottom);

    connect(closeBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(btn, &QPushButton::clicked, &dialog, &QDialog::accept);
    dialog.exec();
}

void MainWindow::renderUsers()
{
    m_usersTable->setRowCount(m_users.size());
    for (int i = 0; i < m_users.size(); ++i) {
        auto u = m_users[i].toObject();
        m_usersTable->setItem(i, 0, new QTableWidgetItem(u.value("username").toString()));
        m_usersTable->setItem(i, 1, new QTableWidgetItem(u.value("role").toString()));
        auto ids = u.value("device_ids").toArray();
        QStringList s;
        for (auto id : ids) s << id.toString();
        m_usersTable->setItem(i, 2, new QTableWidgetItem(s.join(", ")));
        m_usersTable->setItem(i, 3, new QTableWidgetItem(u.value("enabled").toBool() ? "Đang bật" : "Bị khóa"));
    }
}

void MainWindow::updateDeviceConfig(const QString &deviceId, const QJsonObject &config)
{
    post("/api/devices/config", {{"device_id", deviceId}, {"config", config}}, [this](QJsonObject) {
        refreshDevices();
        QMessageBox::information(this, tr("Đã gửi"), tr("Đã gửi cấu hình xuống thiết bị thành công."));
    });
}

void MainWindow::openDeviceConfigDialog(const QJsonObject &device)
{
    const QString deviceId = device.value("device_id").toString();
    const QString type = device.value("device_type").toString();
    const QJsonObject current = device.value("config").toObject();

    QDialog dialog(this);
    dialog.setObjectName("configDialog");
    dialog.setWindowTitle(tr("Chỉnh ngưỡng thiết bị"));
    dialog.setFixedSize(480, 380);
    dialog.setStyleSheet(R"QSS(
        QDialog { background: #0c1230; border: 1px solid #2d3b7e; border-radius: 12px; }
        QLabel { color: #ecf2ff; font-weight: 700; font-size: 12px; }
        QLineEdit { background: #131b45; color: #ffffff; border: 1px solid #2d3b7e; border-radius: 6px; padding: 4px 8px; min-height: 22px; font-weight: 800; }
        QPushButton { background: #7c5cff; color: white; border: none; border-radius: 6px; padding: 6px 14px; font-weight: 900; }
        QPushButton#cancel { background: #263154; color: #dce7ff; }
    )QSS");

    auto *root = new QVBoxLayout(&dialog);
    root->setContentsMargins(14, 12, 14, 12);
    root->setSpacing(8);

    root->addWidget(label(tr("Cấu hình · %1 (%2)").arg(device.value("name").toString(deviceId), deviceId), "pageTitle"));

    auto *form = new QFormLayout;
    form->setHorizontalSpacing(10);
    form->setVerticalSpacing(6);
    QVector<QPair<QString, QLineEdit*>> inputs;
    auto addInput = [&](const QString &key, const QString &title, double fallback) {
        auto *edit = new QLineEdit(QString::number(current.value(key).toDouble(fallback), 'f', 2), &dialog);
        form->addRow(title, edit);
        inputs.append({key, edit});
    };

    if (type == "temperature_sound") {
        addInput("temperature_warn_c", tr("Nhiệt độ cảnh báo (°C)"), 40);
        addInput("temperature_danger_c", tr("Nhiệt độ nguy hiểm (°C)"), 50);
        addInput("sound_warn_vpp", tr("Âm thanh cảnh báo (Vpp)"), 1.5);
        addInput("sampling_interval_seconds", tr("Chu kỳ gửi (giây)"), 2);
    } else if (type == "weather_pressure") {
        addInput("temperature_warn_c", tr("Nhiệt độ cảnh báo (°C)"), 40);
        addInput("pressure_min_hpa", tr("Áp suất min (hPa)"), 990);
        addInput("pressure_max_hpa", tr("Áp suất max (hPa)"), 1030);
        addInput("ir_alarm_seconds", tr("Báo động IR (giây)"), 1);
        addInput("sampling_interval_seconds", tr("Chu kỳ gửi (giây)"), 3);
    } else if (type == "uv_pressure") {
        addInput("uv_warn_index", tr("UV cảnh báo"), 6);
        addInput("uv_danger_index", tr("UV nguy hiểm"), 8);
        addInput("pressure_min_hpa", tr("Áp suất min (hPa)"), 990);
        addInput("pressure_max_hpa", tr("Áp suất max (hPa)"), 1030);
        addInput("sampling_interval_seconds", tr("Chu kỳ gửi (giây)"), 5);
    } else {
        addInput("sampling_interval_seconds", tr("Chu kỳ gửi (giây)"), 5);
    }
    root->addLayout(form);

    auto *actions = new QHBoxLayout;
    auto *cancel = button(tr("Hủy"), "cancel");
    auto *save = button(tr("Lưu cấu hình"));
    actions->addWidget(cancel);
    actions->addWidget(save);
    root->addLayout(actions);

    connect(cancel, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(save, &QPushButton::clicked, &dialog, &QDialog::accept);

    if (dialog.exec() != QDialog::Accepted) return;

    QJsonObject config;
    for (const auto &item : inputs)
        config.insert(item.first, item.second->text().replace(',', '.').toDouble());
    updateDeviceConfig(deviceId, config);
}

void MainWindow::openClaimDeviceDialog(const QJsonObject &device)
{
    const QString deviceId = device.value("device_id").toString();
    const QString type = device.value("device_type").toString();
    const QString defaultName = deviceTypeName(type);

    QDialog dialog(this);
    dialog.setWindowTitle(tr("Ghép nối thiết bị"));
    dialog.setFixedSize(500, 380);
    dialog.setStyleSheet(R"QSS(
        QDialog { background: #0c1230; border: 1px solid #2d3b7e; border-radius: 12px; }
        QLabel { color: #ecf2ff; font-weight: 700; font-size: 12px; }
        QLabel#dialogTitle { color: #ffffff; font-size: 15px; font-weight: 900; }
        QLabel#infoBadge { background: #141c46; color: #7fffd4; border: 1px solid #283770; border-radius: 6px; padding: 4px 8px; font-weight: 800; font-size: 11px; }
        QLineEdit { background: #131b45; color: #ffffff; border: 1px solid #2d3b7e; border-radius: 6px; padding: 5px 10px; min-height: 24px; font-weight: 800; font-size: 12px; }
        QLineEdit:focus { border: 2px solid #7c5cff; background: #172152; }
        QPushButton { background: #7c5cff; color: white; border: none; border-radius: 6px; padding: 6px 14px; font-weight: 900; }
        QPushButton#primaryBtn { background: #20c96b; color: #ffffff; }
        QPushButton#primaryBtn:hover { background: #17ad5a; }
        QPushButton#cancel { background: #263154; color: #dce7ff; }
    )QSS");

    auto *root = new QVBoxLayout(&dialog);
    root->setContentsMargins(14, 10, 14, 10);
    root->setSpacing(6);

    root->addWidget(label("Ghép nối thiết bị vào hệ thống", "dialogTitle"));

    auto *form = new QFormLayout;
    form->setHorizontalSpacing(10);
    form->setVerticalSpacing(6);

    auto *idLabel = label(deviceId, "infoBadge");
    auto *typeLabel = label(deviceTypeName(type) + " (" + type + ")", "infoBadge");
    auto *nameInput = new QLineEdit(&dialog);
    nameInput->setText(defaultName);
    nameInput->setPlaceholderText("Đặt tên thiết bị (ví dụ: Bơm tưới vườn, Cảm biến phòng khách)");
    nameInput->selectAll();

    form->addRow("Mã thiết bị (ID):", idLabel);
    form->addRow("Loại thiết bị:", typeLabel);
    form->addRow("Tên hiển thị:", nameInput);
    root->addLayout(form);

    // Integrated on-screen virtual keyboard for quick touchscreen typing
    auto *kb = new VirtualKeyboard(&dialog);
    kb->attachTo(nameInput);
    kb->setFixedHeight(160);
    root->addWidget(kb);

    auto *actions = new QHBoxLayout;
    auto *cancel = button("Hủy", "cancel");
    auto *save = button("✔ Xác nhận ghép nối", "primaryBtn");
    actions->addWidget(cancel);
    actions->addWidget(save);
    root->addLayout(actions);

    connect(cancel, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(save, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(nameInput, &QLineEdit::returnPressed, &dialog, &QDialog::accept);
    connect(kb, &VirtualKeyboard::enterPressed, &dialog, &QDialog::accept);

    if (dialog.exec() == QDialog::Accepted) {
        QString customName = nameInput->text().trimmed();
        if (customName.isEmpty()) customName = defaultName;
        claimDevice(deviceId, customName);
    }
}

void MainWindow::createUserDialog()
{
    QDialog d(this);
    d.setWindowTitle("Tạo tài khoản");
    d.setFixedSize(420, 260);
    d.setStyleSheet(R"QSS(
        QDialog { background: #0c1230; border: 1px solid #2d3b7e; border-radius: 12px; }
        QLabel { color: #ecf2ff; font-weight: 700; font-size: 12px; }
        QLineEdit, QComboBox { background: #131b45; color: #ffffff; border: 1px solid #2d3b7e; border-radius: 6px; padding: 4px 8px; min-height: 22px; font-weight: 800; }
        QPushButton { background: #7c5cff; color: white; border: none; border-radius: 6px; padding: 6px 14px; font-weight: 900; }
        QPushButton#cancel { background: #263154; color: #dce7ff; }
    )QSS");

    auto *root = new QVBoxLayout(&d);
    root->setContentsMargins(14, 12, 14, 12);
    root->setSpacing(8);

    root->addWidget(label("Tạo tài khoản mới", "pageTitle"));

    auto *form = new QFormLayout;
    QLineEdit u, p;
    p.setEchoMode(QLineEdit::Password);
    QComboBox role;
    role.addItem("Người dùng", "viewer");
    role.addItem("Admin", "admin");

    form->addRow("Tài khoản", &u);
    form->addRow("Mật khẩu", &p);
    form->addRow("Quyền", &role);
    root->addLayout(form);

    auto *actions = new QHBoxLayout;
    auto *cancel = button("Hủy", "cancel");
    auto *ok = button("Tạo");
    actions->addWidget(cancel);
    actions->addWidget(ok);
    root->addLayout(actions);

    connect(cancel, &QPushButton::clicked, &d, &QDialog::reject);
    connect(ok, &QPushButton::clicked, &d, &QDialog::accept);

    if (d.exec() == QDialog::Accepted) {
        post("/api/admin/users", {{"username", u.text().trimmed()}, {"password", p.text()}, {"role", role.currentData().toString()}},
             [this](QJsonObject) { refreshUsers(); });
    }
}

void MainWindow::editUserDialog(const QJsonObject &user)
{
    const QString oldUsername = user.value("username").toString();
    QDialog d(this);
    d.setWindowTitle("Sửa tài khoản");
    d.setFixedSize(440, 310);
    d.setStyleSheet(R"QSS(
        QDialog { background: #0c1230; border: 1px solid #2d3b7e; border-radius: 12px; }
        QLabel { color: #ecf2ff; font-weight: 700; font-size: 12px; }
        QLineEdit, QComboBox { background: #131b45; color: #ffffff; border: 1px solid #2d3b7e; border-radius: 6px; padding: 4px 8px; min-height: 22px; font-weight: 800; }
        QPushButton { background: #7c5cff; color: white; border: none; border-radius: 6px; padding: 6px 14px; font-weight: 900; }
        QPushButton#cancel { background: #263154; color: #dce7ff; }
        QPushButton#delete { background: #e63956; color: white; }
    )QSS");

    auto *root = new QVBoxLayout(&d);
    root->setContentsMargins(14, 12, 14, 12);
    root->setSpacing(8);

    root->addWidget(label("Sửa tài khoản: " + oldUsername, "pageTitle"));

    auto *form = new QFormLayout;
    QLineEdit username(oldUsername), password;
    password.setEchoMode(QLineEdit::Password);
    password.setPlaceholderText("Trống = giữ nguyên");
    QComboBox role;
    role.addItem("Người dùng", "viewer");
    role.addItem("Admin", "admin");
    role.setCurrentIndex(user.value("role").toString() == "admin" ? 1 : 0);
    QCheckBox enabled("Đang hoạt động");
    enabled.setChecked(user.value("enabled").toBool(true));

    form->addRow("Tài khoản", &username);
    form->addRow("Mật khẩu mới", &password);
    form->addRow("Quyền", &role);
    form->addRow("Trạng thái", &enabled);
    root->addLayout(form);

    auto *actions = new QHBoxLayout;
    auto *remove = button("Xóa", "delete");
    auto *cancel = button("Hủy", "cancel");
    auto *save = button("Lưu");
    actions->addWidget(remove);
    actions->addWidget(cancel);
    actions->addWidget(save);
    root->addLayout(actions);

    connect(cancel, &QPushButton::clicked, &d, &QDialog::reject);
    connect(save, &QPushButton::clicked, &d, &QDialog::accept);
    connect(remove, &QPushButton::clicked, &d, [&] {
        if (QMessageBox::question(this, "Xác nhận", "Xóa người dùng " + oldUsername + "?") != QMessageBox::Yes)
            return;
        del("/api/admin/users/" + QString::fromUtf8(QUrl::toPercentEncoding(oldUsername)), [this](QJsonObject) { refreshUsers(); });
        d.reject();
    });

    if (d.exec() == QDialog::Accepted) {
        QJsonObject payload{{"username", username.text().trimmed()}, {"password", password.text()}, {"role", role.currentData().toString()}, {"enabled", enabled.isChecked()}};
        auto *reply = m_net.put(request("/api/admin/users/" + QString::fromUtf8(QUrl::toPercentEncoding(oldUsername))), QJsonDocument(payload).toJson());
        connect(reply, &QNetworkReply::finished, this, [this, reply] {
            if (reply->error() != QNetworkReply::NoError)
                QMessageBox::warning(this, "Lỗi", reply->errorString());
            reply->deleteLater();
            refreshUsers();
        });
    }
}
