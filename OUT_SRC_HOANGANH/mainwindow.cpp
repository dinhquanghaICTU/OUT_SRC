#include "mainwindow.h"

#include <algorithm>
#include <limits>

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
#include <QPushButton>
#include <QScrollArea>
#include <QStackedWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTimer>
#include <QUrlQuery>
#include <QVBoxLayout>

namespace {
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
    setWindowTitle(QStringLiteral("HoangAnh IoT Command Center"));
    m_root = new QStackedWidget(this);
    setCentralWidget(m_root);
    setStyleSheet(R"QSS(
        QMainWindow { background: #080b1f; }
        QWidget { color: #ecf2ff; }
        QFrame#loginWrap { background: qradialgradient(cx:0.15, cy:0.2, radius:1.2, stop:0 #242066, stop:0.45 #0b1534, stop:1 #080b1f); }
        QFrame#loginCard { background: rgba(255,255,255,245); border-radius: 30px; }
        QLabel#loginHero { color: #ffffff; font-size: 34px; font-weight: 950; }
        QLabel#loginSub { color: #b9c6ff; font-size: 14px; font-weight: 700; }
        QLabel#loginTitle { color: #11182f; font-size: 28px; font-weight: 950; }
        QLabel#loginHint { color: #657085; font-size: 13px; font-weight: 750; }
        QLineEdit, QComboBox { background: #f4f7fb; color: #11182f; border: 1px solid #d9e1ee; border-radius: 14px; padding: 10px 14px; min-height: 28px; font-weight: 800; }
        QLineEdit:focus, QComboBox:focus { border: 2px solid #7c5cff; background: #ffffff; }
        QPushButton { background: #7c5cff; border: none; border-radius: 14px; color: white; padding: 10px 16px; font-weight: 950; }
        QPushButton:hover { background: #6848f4; }
        QPushButton#ghost { background: rgba(255,255,255,18); border: 1px solid rgba(255,255,255,35); }
        QPushButton#danger { background: #ef476f; }
        QFrame#shell { background: #080b1f; }
        QFrame#topbar { background: rgba(16,22,52,235); border: 1px solid rgba(124,92,255,60); border-radius: 22px; }
        QLabel#brand { font-size: 22px; font-weight: 950; color: #ffffff; }
        QLabel#muted { color: #93a4c7; font-weight: 750; }
        QPushButton#nav { background: transparent; color: #aebdf0; border-radius: 12px; padding: 9px 14px; text-align: left; }
        QPushButton#nav:checked { background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #ff8a3d, stop:1 #7c5cff); color: white; }
        QFrame#pageCard, QFrame#deviceCard, QFrame#metricCard, QFrame#chartCard { background: rgba(18,26,62,235); border: 1px solid rgba(124,92,255,55); border-radius: 24px; }
        QFrame#deviceCard { background: qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #151d48, stop:1 #0d3f4b); }
        QFrame#availableCard { background: qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #141c45, stop:1 #103f4a); border: 1px solid rgba(127,255,212,85); border-radius: 22px; }
        QFrame#availableCard QLabel { color: #ecf2ff; }
        QLabel#readyBadge { background: rgba(32,201,107,210); color: #ffffff; border-radius: 14px; padding: 6px 14px; font-weight: 950; }
        QPushButton#readyBadge { background: #20c96b; color: #ffffff; border-radius: 14px; padding: 7px 16px; font-weight: 950; }
        QPushButton#readyBadge:hover { background: #17ad5a; }
        QLabel#pageTitle { font-size: 32px; font-weight: 950; }
        QLabel#cardIcon { background: rgba(255,255,255,18); color: #ffbe55; border-radius: 22px; min-width: 54px; min-height: 54px; font-size: 25px; qproperty-alignment: AlignCenter; }
        QLabel#deviceName { font-size: 20px; font-weight: 950; }
        QLabel#bigMetric { font-size: 30px; font-weight: 950; color: #7fffd4; }
        QLabel#orange { color: #ffbe55; font-weight: 950; }
        QLabel#onlineBadge { background: #20c96b; color: #ffffff; border-radius: 14px; padding: 6px 14px; font-weight: 950; }
        QLabel#offlineBadge { background: #6f7788; color: #ffffff; border-radius: 14px; padding: 6px 14px; font-weight: 950; }
        QLabel#irSafeBadge { background: rgba(32,201,107,220); color: #ffffff; border-radius: 14px; padding: 7px 14px; font-weight: 950; }
        QLabel#irAlertBadge { background: #ef3f46; color: #ffffff; border-radius: 14px; padding: 7px 14px; font-weight: 950; }
        QPushButton:disabled { background: #596071; color: #c5ccda; }
        QTableWidget { background: rgba(18,26,62,235); color: #ecf2ff; border: 1px solid rgba(124,92,255,55); border-radius: 18px; gridline-color: rgba(255,255,255,25); selection-background-color: #7c5cff; }
        QHeaderView::section { background: #111a3e; color: #cdd7ff; border: none; padding: 10px; font-weight: 950; }
        QFrame#miniChartCard { background: rgba(18,26,62,230); border: 1px solid rgba(124,92,255,60); border-radius: 18px; }
        QChartView { background: transparent; border: none; }
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

void MainWindow::buildLogin()
{
    m_loginPage = panel("loginWrap");
    auto *root = new QHBoxLayout(m_loginPage);
    root->setContentsMargins(70, 55, 70, 55);
    root->setSpacing(0);
    auto *hero = panel("loginWrap");
    auto *hl = new QVBoxLayout(hero);
    hl->setContentsMargins(40,40,40,40);
    hl->addStretch();
    hl->addWidget(label("HOANGANH\nIoT ORBIT", "loginHero"));
    hl->addWidget(label("Một giao diện command center riêng, vẫn dùng server Trung Kiên.", "loginSub"));
    hl->addStretch();
    auto *card = panel("loginCard");
    card->setMinimumWidth(440);
    auto *cl = new QVBoxLayout(card);
    cl->setContentsMargins(42,42,42,42);
    cl->setSpacing(16);
    cl->addStretch();
    cl->addWidget(label("Đăng nhập", "loginTitle"));
    cl->addWidget(label("Kết nối tới Raspberry Pi API", "loginHint"));
    auto *u = new QLineEdit; u->setPlaceholderText("Tài khoản");
    auto *p = new QLineEdit; p->setPlaceholderText("Mật khẩu"); p->setEchoMode(QLineEdit::Password);
    auto *login = button("Vào hệ thống");
    cl->addWidget(u); cl->addWidget(p); cl->addWidget(login); cl->addStretch();
    root->addWidget(hero, 1); root->addWidget(card);
    connect(login, &QPushButton::clicked, this, [=] {
        post("/api/auth/login", {{"username", u->text()}, {"password", p->text()}}, [=](QJsonObject obj) {
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
    });
    connect(p, &QLineEdit::returnPressed, login, &QPushButton::click);
}

void MainWindow::buildShell()
{
    m_shellPage = panel("shell");
    auto *root = new QVBoxLayout(m_shellPage);
    root->setContentsMargins(22,22,22,22);
    root->setSpacing(16);
    auto *top = panel("topbar");
    auto *tl = new QHBoxLayout(top);
    tl->setContentsMargins(18,14,18,14);
    tl->addWidget(label("HA Command Center", "brand"));
    const QStringList names{"◆ Tổng quan", "▣ Thiết bị", "▥ Thống kê", "♙ Tài khoản"};
    for (int i=0;i<names.size();++i) {
        auto *b = button(names[i], "nav"); b->setCheckable(true); if (i==0) b->setChecked(true);
        connect(b, &QPushButton::clicked, this, [this, i, b] { setPage(i); b->setChecked(true); });
        tl->addWidget(b);
    }
    tl->addStretch();
    m_status = label("offline", "muted"); tl->addWidget(m_status);
    root->addWidget(top);
    m_pages = new QStackedWidget;
    buildHome(); buildDevices(); buildHistory(); buildUsers();
    root->addWidget(m_pages, 1);
}

void MainWindow::setPage(int index) { m_pages->setCurrentIndex(index); refreshAll(); }

void MainWindow::buildHome()
{
    auto *page = new QWidget;
    auto *l = new QGridLayout(page); l->setSpacing(16);
    m_homeTitle = label("Nhà của --", "pageTitle"); l->addWidget(m_homeTitle,0,0,1,3);
    auto mk = [](const QString &t){ auto *f=panel("metricCard"); auto *v=new QVBoxLayout(f); v->addWidget(label(t,"orange")); auto *val=label("--","bigMetric"); v->addWidget(val); return qMakePair(f,val); };
    auto a=mk("Tổng thiết bị"), b=mk("Online"), c=mk("Loại cảm biến");
    m_kpiDevices=a.second; m_kpiOnline=b.second; m_kpiType=c.second;
    l->addWidget(a.first,1,0); l->addWidget(b.first,1,1); l->addWidget(c.first,1,2);
    auto *hero=panel("pageCard");
    auto *hl=new QVBoxLayout(hero);
    hl->addWidget(label("Live device console", "pageTitle"));
    hl->addWidget(label("Xem thông số cảm biến và điều khiển từng thiết bị ngay tại tổng quan.", "muted"));
    m_homeDeviceGrid = new QGridLayout;
    m_homeDeviceGrid->setSpacing(14);
    hl->addLayout(m_homeDeviceGrid);
    hl->addStretch();
    l->addWidget(hero,2,0,1,3);
    m_pages->addWidget(page);
}

void MainWindow::buildDevices()
{
    auto *page = new QWidget; auto *root=new QVBoxLayout(page); root->setSpacing(14);
    root->addWidget(label("Thiết bị dạng timeline", "pageTitle"));
    auto *scroll = new QScrollArea; scroll->setWidgetResizable(true); scroll->setFrameShape(QFrame::NoFrame);
    auto *wrap = new QWidget; auto *vl = new QVBoxLayout(wrap);
    m_deviceGrid = new QGridLayout; m_deviceGrid->setSpacing(14); vl->addLayout(m_deviceGrid);
    vl->addWidget(label("Thiết bị có thể thêm", "pageTitle"));
    m_availableGrid = new QGridLayout; m_availableGrid->setSpacing(14); vl->addLayout(m_availableGrid); vl->addStretch();
    scroll->setWidget(wrap); root->addWidget(scroll,1); m_pages->addWidget(page);
}

void MainWindow::buildHistory()
{
    auto *page = new QWidget; auto *root=new QVBoxLayout(page); root->setSpacing(12);
    auto *bar=new QHBoxLayout; bar->addWidget(label("Phòng phân tích", "pageTitle")); m_historyDevice=new QComboBox; m_historyPeriod=new QComboBox; m_historyPeriod->addItem("Ngày","day"); m_historyPeriod->addItem("Tháng","month"); m_historyPeriod->addItem("Năm","year"); auto *search=button("Quét dữ liệu"); bar->addWidget(m_historyDevice); bar->addWidget(m_historyPeriod); bar->addWidget(search); root->addLayout(bar);
    auto *charts=panel("pageCard");
    charts->setMaximumHeight(520);
    auto *cl=new QVBoxLayout(charts);
    cl->setContentsMargins(16, 14, 16, 14);
    m_historyCharts=new QGridLayout;
    m_historyCharts->setSpacing(8);
    m_historyCharts->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    cl->addLayout(m_historyCharts);
    root->addWidget(charts,0);
    m_historyTable=new QTableWidget;
    m_historyTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_historyTable->verticalHeader()->hide();
    root->addWidget(m_historyTable,1);
    connect(search,&QPushButton::clicked,this,&MainWindow::refreshHistory); connect(m_historyDevice,&QComboBox::currentIndexChanged,this,[this](int){refreshHistory();});
    m_pages->addWidget(page);
}

void MainWindow::buildUsers()
{
    auto *page=new QWidget; auto *root=new QVBoxLayout(page); auto *top=new QHBoxLayout; top->addWidget(label("Ma trận người dùng", "pageTitle")); auto *add=button("+ Tạo user"); top->addWidget(add); root->addLayout(top);
    m_usersTable=new QTableWidget(0,4); m_usersTable->setHorizontalHeaderLabels({"Tài khoản","Quyền","Thiết bị","Trạng thái"}); m_usersTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); m_usersTable->verticalHeader()->hide(); root->addWidget(m_usersTable);
    connect(add,&QPushButton::clicked,this,&MainWindow::createUserDialog);
    connect(m_usersTable, &QTableWidget::cellDoubleClicked, this, [this](int row, int){ if(row>=0 && row<m_users.size()) editUserDialog(m_users.at(row).toObject()); });
    connect(m_usersTable, &QTableWidget::cellClicked, this, [this](int row, int){ if(row>=0 && row<m_users.size()) editUserDialog(m_users.at(row).toObject()); });
    m_pages->addWidget(page);
}

QString MainWindow::deviceIcon(const QString &type){ if(type=="temperature_sound")return "♪"; if(type=="weather_pressure")return "☁"; if(type=="uv_pressure")return "☀"; if(type=="electric_power")return "⚡"; if(type=="water_flow_pump")return "🚰"; return "◆"; }
QString MainWindow::deviceTypeName(const QString &type){ if(type=="temperature_sound")return "Nhiệt độ & âm thanh"; if(type=="weather_pressure")return "Môi trường"; if(type=="uv_pressure")return "UV & áp suất"; if(type=="electric_power")return "Đo điện"; if(type=="water_flow_pump")return "Bơm & lưu lượng nước"; return "Thiết bị IoT"; }
QString MainWindow::metricText(const QJsonObject &d){ const QString type=d.value("device_type").toString(); if(type=="temperature_sound") return firstMetric(d,"temperature_c","°C")+"  •  "+firstMetric(d,"sound_vpp","Vpp"); if(type=="weather_pressure") return firstMetric(d,"temperature_c","°C")+"  •  "+firstMetric(d,"pressure_hpa","hPa"); if(type=="uv_pressure") return firstMetric(d,"uv_index","UV")+"  •  "+firstMetric(d,"pressure_hpa","hPa"); if(type=="electric_power") return firstMetric(d,"current_a","A")+"  •  "+firstMetric(d,"voltage_v","V"); if(type=="water_flow_pump") return firstMetric(d,"flow_l_min","L/min")+"  •  "+firstMetric(d,"total_liters","L"); return "--"; }

void MainWindow::refreshAll(){ refreshDevices(); if(m_role=="admin") refreshUsers(); }
void MainWindow::refreshDevices(){ get("/api/devices/me", [this](QJsonObject o){m_devices=o.value("data").toArray(); renderDevices(); refreshAvailable(); refreshHistory();}); }
void MainWindow::refreshAvailable(){ get("/api/devices/available", [this](QJsonObject o){m_available=o.value("data").toArray(); renderAvailable();}); }
void MainWindow::refreshUsers(){ if(m_role!="admin") return; get("/api/admin/users", [this](QJsonObject o){m_users=o.value("data").toArray(); renderUsers();}); }

void MainWindow::renderDevices()
{
    m_homeTitle->setText("Nhà của " + m_username.toUpper());
    m_kpiDevices->setText(QString::number(m_devices.size()));
    int online=0; QSet<QString> types;
    while(auto *it=m_deviceGrid->takeAt(0)){ if(it->widget()) it->widget()->deleteLater(); delete it; }
    if (m_homeDeviceGrid) {
        while(auto *it=m_homeDeviceGrid->takeAt(0)){ if(it->widget()) it->widget()->deleteLater(); delete it; }
    }
    const QString selectedHistoryDevice = m_historyDevice->currentData().toString();
    m_historyDevice->blockSignals(true);
    m_historyDevice->clear();
    auto makeDeviceCard = [this](const QJsonObject &d) {
        auto *card=panel("deviceCard");
        card->setCursor(Qt::PointingHandCursor);
        auto *l=new QVBoxLayout(card);
        l->setSpacing(10);
        auto *top=new QHBoxLayout;
        top->addWidget(label(deviceIcon(d.value("device_type").toString()),"cardIcon"));
        auto *titleBox=new QVBoxLayout;
        titleBox->addWidget(label(d.value("name").toString(d.value("device_id").toString()),"deviceName"));
        titleBox->addWidget(label(deviceTypeName(d.value("device_type").toString()) + " · " + d.value("device_id").toString(),"muted"));
        top->addLayout(titleBox,1);
        const bool onlineNow = isDeviceOnline(d);
        auto *statusBadge = label(onlineNow ? "ONLINE" : "OFFLINE", onlineNow ? "onlineBadge" : "offlineBadge");
        top->addWidget(statusBadge);
        l->addLayout(top);
        l->addWidget(label(metricText(d),"bigMetric"));
        const auto m=d.value("metrics").toObject();
        if (d.value("device_type").toString() == "weather_pressure" && m.contains("ir_detected")) {
            const bool detected = m.value("ir_detected").toDouble() >= 0.5;
            l->addWidget(label(detected ? QStringLiteral("●  IR: Có vật")
                                        : QStringLiteral("●  IR: Không có vật"),
                               detected ? "irAlertBadge" : "irSafeBadge"));
        }
        QStringList lines;
        for (auto it=m.begin(); it!=m.end(); ++it) {
            if (it.value().isDouble()) lines << QStringLiteral("%1: %2").arg(it.key(), QString::number(it.value().toDouble(),'f',2));
        }
        l->addWidget(label(lines.isEmpty()?"Chưa có telemetry":lines.join("  •  "),"muted"));
        auto *row=new QHBoxLayout;
        auto *cfg=button("Chỉnh ngưỡng");
        auto *rel=button("Gỡ","danger");
        row->addWidget(cfg);
        row->addWidget(rel);
        bool hasRelay=false;
        for (const auto &cap : d.value("capabilities").toArray()) {
            if (cap.toString() == "relay") hasRelay = true;
        }
        if(hasRelay){
            const bool relayOn=d.value("state").toObject().value("relay").toBool(false);
            auto *r=button((d.value("device_type").toString()=="water_flow_pump") ? (relayOn?"Bơm bật":"Bơm tắt") : (relayOn?"Relay bật":"Relay tắt"));
            r->setEnabled(onlineNow);
            r->setToolTip(onlineNow ? QString() : QStringLiteral("Thiết bị đang offline, không gửi lệnh relay."));
            row->addWidget(r);
            connect(r,&QPushButton::clicked,this,[=]{ if (onlineNow) toggleRelay(d.value("device_id").toString(), !relayOn); });
        }
        row->addStretch();
        l->addLayout(row);
        connect(cfg,&QPushButton::clicked,this,[=]{openDeviceConfigDialog(d);});
        connect(card, &QFrame::destroyed, this, []{});
        connect(rel,&QPushButton::clicked,this,[=]{releaseDevice(d.value("device_id").toString());});
        return card;
    };
    int i=0;
    for(const auto &v:m_devices){
        auto d=v.toObject();
        if(isDeviceOnline(d)) online++;
        types.insert(d.value("device_type").toString());
        m_historyDevice->addItem(d.value("name").toString(d.value("device_id").toString())+" · "+d.value("device_id").toString(), d.value("device_id").toString());
        m_deviceGrid->addWidget(makeDeviceCard(d),i/2,i%2);
        if (m_homeDeviceGrid) m_homeDeviceGrid->addWidget(makeDeviceCard(d),i/3,i%3);
        i++;
    }
    if (!selectedHistoryDevice.isEmpty()) {
        const int keepIndex = m_historyDevice->findData(selectedHistoryDevice);
        if (keepIndex >= 0) m_historyDevice->setCurrentIndex(keepIndex);
    }
    m_historyDevice->blockSignals(false);
    m_kpiOnline->setText(QString::number(online));
    m_kpiType->setText(QString::number(types.size()));
    m_status->setText("sync " + QTime::currentTime().toString("HH:mm:ss"));
}

void MainWindow::renderAvailable()
{
    while(auto *it=m_availableGrid->takeAt(0)){ if(it->widget()) it->widget()->deleteLater(); delete it; }

    int i = 0;
    for (const auto &v : m_available) {
        const auto d = v.toObject();
        auto *card = panel("availableCard");
        card->setMinimumHeight(120);
        card->setMaximumHeight(150);
        card->setCursor(Qt::PointingHandCursor);

        auto *row = new QHBoxLayout(card);
        row->setContentsMargins(18, 16, 18, 16);
        row->setSpacing(18);

        row->addWidget(label(deviceIcon(d.value("device_type").toString()), "cardIcon"));

        auto *info = new QVBoxLayout;
        info->setSpacing(5);
        info->addWidget(label(d.value("device_id").toString(), "deviceName"));
        info->addWidget(label(deviceTypeName(d.value("device_type").toString()), "muted"));
        const auto metrics = d.value("metrics").toObject();
        QStringList metricBits;
        for (auto it = metrics.begin(); it != metrics.end(); ++it) {
            if (it.value().isDouble()) metricBits << QStringLiteral("%1: %2").arg(it.key(), QString::number(it.value().toDouble(), 'f', 1));
        }
        info->addWidget(label(metricBits.isEmpty() ? QStringLiteral("Đang chờ liên kết") : metricBits.join("  •  "), "muted"));
        row->addLayout(info, 1);

        auto *side = new QVBoxLayout;
        side->setSpacing(10);
        auto *ready = button("SẴN SÀNG", "readyBadge");
        auto *add = button("+ Liên kết");
        ready->setMinimumWidth(150);
        add->setMinimumWidth(150);
        side->addWidget(ready);
        side->addWidget(add);
        side->addStretch();
        row->addLayout(side);

        auto claimThisDevice = [=]{
            claimDevice(d.value("device_id").toString(), deviceTypeName(d.value("device_type").toString()));
        };
        connect(ready, &QPushButton::clicked, this, claimThisDevice);
        connect(add, &QPushButton::clicked, this, claimThisDevice);
        m_availableGrid->addWidget(card, i / 2, i % 2);
        i++;
    }
}

void MainWindow::claimDevice(const QString &id,const QString &name){ post("/api/devices/claim",{{"device_id",id},{"name",name}},[this](QJsonObject){refreshDevices(); QTimer::singleShot(350, this, &MainWindow::refreshDevices);}); }
void MainWindow::releaseDevice(const QString &id){ post("/api/devices/release",{{"device_id",id}},[this](QJsonObject){refreshDevices();}); }
void MainWindow::toggleRelay(const QString &id,bool state){ post("/api/devices/relay",{{"device_id",id},{"state",state}},[this](QJsonObject){refreshDevices();}); }

void MainWindow::refreshHistory()
{
    const QString id=m_historyDevice->currentData().toString(); if(id.isEmpty()) return; QUrlQuery q; q.addQueryItem("device_id",id); q.addQueryItem("period",m_historyPeriod->currentData().toString()); q.addQueryItem("date",QDate::currentDate().toString(Qt::ISODate)); q.addQueryItem("limit","160"); get("/api/devices/history?"+q.toString(QUrl::FullyEncoded),[this](QJsonObject o){renderHistory(o);});
}

void MainWindow::renderHistory(const QJsonObject &h)
{
    while(auto *it=m_historyCharts->takeAt(0)){ if(it->widget()) it->widget()->deleteLater(); delete it; }
    const auto keys=h.value("metric_keys").toArray(), rows=h.value("data").toArray();
    QStringList headers{"Thời gian"}; for(auto k:keys) headers<<k.toString(); m_historyTable->setColumnCount(headers.size()); m_historyTable->setHorizontalHeaderLabels(headers); m_historyTable->setRowCount(rows.size());
    for(int r=0;r<rows.size();++r){ auto e=rows[r].toObject(); m_historyTable->setItem(r,0,new QTableWidgetItem(e.value("recorded_at").toString())); auto m=e.value("metrics").toObject(); for(int c=0;c<keys.size();++c)m_historyTable->setItem(r,c+1,new QTableWidgetItem(QString::number(m.value(keys[c].toString()).toDouble(),'f',2))); }
    int ci = 0;
    for (auto k : keys) {
        QString key = k.toString();
        auto *set = new QBarSet(key);
        QStringList cats;
        double minValue = std::numeric_limits<double>::max();
        double maxValue = std::numeric_limits<double>::lowest();
        int used = 0;
        for (int r = rows.size() - 1; r >= 0 && used < 6; --r, ++used) {
            auto e = rows[r].toObject();
            cats << QDateTime::fromString(e.value("recorded_at").toString(), Qt::ISODateWithMs).toLocalTime().toString("HH:mm:ss");
            const double value = e.value("metrics").toObject().value(key).toDouble();
            *set << value;
            minValue = std::min(minValue, value);
            maxValue = std::max(maxValue, value);
        }
        auto *series = new QBarSeries;
        series->setBarWidth(0.16);
        series->append(set);
        auto *chart = new QChart;
        chart->setTitle(key);
        chart->legend()->hide();
        chart->addSeries(series);
        chart->setBackgroundVisible(false);
        chart->setMargins(QMargins(4, 4, 4, 4));
        auto *ax = new QBarCategoryAxis;
        ax->append(cats);
        auto *ay = new QValueAxis;
        if (key == "pressure_hpa") {
            ay->setRange(1000.0, std::max(1015.0, maxValue + 1.0));
        } else if (key.contains("uv") || key.contains("ir")) {
            ay->setRange(0.0, std::max(1.0, maxValue + 0.5));
        } else if (minValue != std::numeric_limits<double>::max() && maxValue > minValue) {
            const double pad = std::max(0.5, (maxValue - minValue) * 0.25);
            ay->setRange(std::max(0.0, minValue - pad), maxValue + pad);
        }
        chart->addAxis(ax, Qt::AlignBottom);
        chart->addAxis(ay, Qt::AlignLeft);
        series->attachAxis(ax);
        series->attachAxis(ay);
        auto *mini = panel("miniChartCard");
        mini->setFixedSize(300, 210);
        auto *miniLayout = new QVBoxLayout(mini);
        miniLayout->setContentsMargins(8, 6, 8, 6);
        miniLayout->setSpacing(2);
        auto *view = new QChartView(chart);
        view->setFixedSize(280, 180);
        view->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        miniLayout->addWidget(view, 0, Qt::AlignCenter);
        m_historyCharts->addWidget(mini, ci / 4, ci % 4);
        ci++;
    }
}


void MainWindow::renderUsers()
{
    m_usersTable->setRowCount(m_users.size());
    for(int i=0;i<m_users.size();++i){ auto u=m_users[i].toObject(); m_usersTable->setItem(i,0,new QTableWidgetItem(u.value("username").toString())); m_usersTable->setItem(i,1,new QTableWidgetItem(u.value("role").toString())); auto ids=u.value("device_ids").toArray(); QStringList s; for(auto id:ids)s<<id.toString(); m_usersTable->setItem(i,2,new QTableWidgetItem(s.join(", "))); m_usersTable->setItem(i,3,new QTableWidgetItem(u.value("enabled").toBool()?"active":"locked")); }
}


void MainWindow::updateDeviceConfig(const QString &deviceId, const QJsonObject &config)
{
    post("/api/devices/config", {{"device_id", deviceId}, {"config", config}}, [this](QJsonObject) {
        refreshDevices();
        QMessageBox::information(this, tr("Đã gửi"), tr("Đã gửi cấu hình xuống thiết bị qua server."));
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
    dialog.setMinimumWidth(520);
    dialog.setStyleSheet(R"QSS(
        QDialog#configDialog { background: #101735; border-radius: 22px; }
        QLabel#configTitle { color: #ffffff; font-size: 24px; font-weight: 950; }
        QLabel#configHint { color: #aebdf0; font-size: 13px; font-weight: 800; }
        QLabel { color: #dce7ff; font-weight: 850; }
        QLineEdit { background: #f5f8ff; color: #101735; border: 1px solid #dce4f2; border-radius: 13px; padding: 10px 14px; min-height: 28px; font-weight: 900; }
        QLineEdit:focus { border: 2px solid #7c5cff; }
        QPushButton { background: #7c5cff; color: white; border: none; border-radius: 13px; padding: 10px 16px; font-weight: 950; }
        QPushButton#configCancel { background: #263154; color: #dce7ff; }
    )QSS");
    auto *root = new QVBoxLayout(&dialog);
    root->setContentsMargins(26, 24, 26, 24);
    root->setSpacing(14);
    root->addWidget(label(tr("Cấu hình ngưỡng · %1").arg(device.value("name").toString(deviceId)), "configTitle"));
    root->addWidget(label(tr("Device ID: %1").arg(deviceId), "configHint"));

    auto *form = new QFormLayout;
    form->setHorizontalSpacing(14);
    form->setVerticalSpacing(10);
    QVector<QPair<QString, QLineEdit*>> inputs;
    auto addInput = [&](const QString &key, const QString &title, double fallback) {
        auto *edit = new QLineEdit(QString::number(current.value(key).toDouble(fallback), 'f', 2), &dialog);
        edit->setProperty("configKey", key);
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
        addInput("pressure_min_hpa", tr("Áp suất tối thiểu (hPa)"), 990);
        addInput("pressure_max_hpa", tr("Áp suất tối đa (hPa)"), 1030);
        addInput("ir_alarm_seconds", tr("IR báo động sau (giây)"), 1);
        addInput("sampling_interval_seconds", tr("Chu kỳ gửi (giây)"), 3);
    } else if (type == "uv_pressure") {
        addInput("uv_warn_index", tr("UV cảnh báo"), 6);
        addInput("uv_danger_index", tr("UV nguy hiểm"), 8);
        addInput("pressure_min_hpa", tr("Áp suất tối thiểu (hPa)"), 990);
        addInput("pressure_max_hpa", tr("Áp suất tối đa (hPa)"), 1030);
        addInput("sampling_interval_seconds", tr("Chu kỳ gửi (giây)"), 5);
    } else {
        addInput("sampling_interval_seconds", tr("Chu kỳ gửi (giây)"), 5);
    }
    root->addLayout(form);

    auto *actions = new QHBoxLayout;
    auto *cancel = button(tr("Hủy"), "configCancel");
    auto *save = button(tr("Lưu xuống thiết bị"));
    actions->addWidget(cancel);
    actions->addWidget(save);
    root->addLayout(actions);
    connect(cancel, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(save, &QPushButton::clicked, &dialog, &QDialog::accept);

    if (dialog.exec() != QDialog::Accepted)
        return;
    QJsonObject config;
    for (const auto &item : inputs)
        config.insert(item.first, item.second->text().replace(',', '.').toDouble());
    updateDeviceConfig(deviceId, config);
}

void MainWindow::createUserDialog()
{
    QDialog d(this);
    d.setObjectName("userDialog");
    d.setWindowTitle("Tạo user");
    d.setMinimumWidth(430);
    d.setStyleSheet(R"QSS(
        QDialog#userDialog { background: #101735; border-radius: 22px; }
        QLabel#userDialogTitle { color: #ffffff; font-size: 24px; font-weight: 950; }
        QLabel { color: #dce7ff; font-weight: 850; }
        QLineEdit, QComboBox { background: #f5f8ff; color: #101735; border: 1px solid #dce4f2; border-radius: 13px; padding: 10px 14px; min-height: 28px; font-weight: 900; }
        QLineEdit:focus, QComboBox:focus { border: 2px solid #7c5cff; }
        QPushButton { background: #7c5cff; color: white; border: none; border-radius: 13px; padding: 10px 16px; font-weight: 950; }
        QPushButton#cancelUser { background: #263154; color: #dce7ff; }
        QPushButton#deleteUser { background: #ef476f; color: white; }
    )QSS");
    auto *root = new QVBoxLayout(&d);
    root->setContentsMargins(24, 22, 24, 22);
    root->setSpacing(14);
    root->addWidget(label("Tạo tài khoản mới", "userDialogTitle"));
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
    auto *cancel = button("Hủy", "cancelUser");
    auto *ok = button("Tạo user");
    actions->addWidget(cancel);
    actions->addWidget(ok);
    root->addLayout(actions);
    connect(cancel, &QPushButton::clicked, &d, &QDialog::reject);
    connect(ok, &QPushButton::clicked, &d, &QDialog::accept);
    if (d.exec() == QDialog::Accepted) {
        post("/api/admin/users", {{"username", u.text()}, {"password", p.text()}, {"role", role.currentData().toString()}},
             [this](QJsonObject){ refreshUsers(); });
    }
}

void MainWindow::editUserDialog(const QJsonObject &user)
{
    const QString oldUsername = user.value("username").toString();
    QDialog d(this);
    d.setObjectName("userDialog");
    d.setWindowTitle("Sửa user");
    d.setMinimumWidth(460);
    d.setStyleSheet(R"QSS(
        QDialog#userDialog { background: #101735; border-radius: 22px; }
        QLabel#userDialogTitle { color: #ffffff; font-size: 24px; font-weight: 950; }
        QLabel#userHint { color: #aebdf0; font-weight: 800; }
        QLabel { color: #dce7ff; font-weight: 850; }
        QLineEdit, QComboBox { background: #f5f8ff; color: #101735; border: 1px solid #dce4f2; border-radius: 13px; padding: 10px 14px; min-height: 28px; font-weight: 900; }
        QLineEdit:focus, QComboBox:focus { border: 2px solid #7c5cff; }
        QCheckBox { color: #dce7ff; font-weight: 850; }
        QPushButton { background: #7c5cff; color: white; border: none; border-radius: 13px; padding: 10px 16px; font-weight: 950; }
        QPushButton#cancelUser { background: #263154; color: #dce7ff; }
        QPushButton#deleteUser { background: #ef476f; color: white; }
    )QSS");
    auto *root = new QVBoxLayout(&d);
    root->setContentsMargins(24, 22, 24, 22);
    root->setSpacing(14);
    root->addWidget(label("Sửa / xóa tài khoản", "userDialogTitle"));
    root->addWidget(label("Click user trong bảng để mở popup này.", "userHint"));
    auto *form = new QFormLayout;
    QLineEdit username(oldUsername), password;
    password.setEchoMode(QLineEdit::Password);
    password.setPlaceholderText("Không nhập = giữ mật khẩu cũ");
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
    auto ids = user.value("device_ids").toArray();
    QStringList deviceIds;
    for (auto id : ids) deviceIds << id.toString();
    root->addWidget(label("Thiết bị: " + (deviceIds.isEmpty() ? QString("Chưa có") : deviceIds.join(", ")), "userHint"));
    auto *actions = new QHBoxLayout;
    auto *remove = button("Xóa user", "deleteUser");
    auto *cancel = button("Hủy", "cancelUser");
    auto *save = button("Lưu thay đổi");
    actions->addWidget(remove);
    actions->addWidget(cancel);
    actions->addWidget(save);
    root->addLayout(actions);
    connect(cancel, &QPushButton::clicked, &d, &QDialog::reject);
    connect(save, &QPushButton::clicked, &d, &QDialog::accept);
    connect(remove, &QPushButton::clicked, &d, [&] {
        if (QMessageBox::question(this, "Xóa user", "Xóa tài khoản " + oldUsername + "?") != QMessageBox::Yes)
            return;
        del("/api/admin/users/" + QString::fromUtf8(QUrl::toPercentEncoding(oldUsername)), [this](QJsonObject){ refreshUsers(); });
        d.reject();
    });
    if (d.exec() == QDialog::Accepted) {
        QJsonObject payload{{"username", username.text()}, {"password", password.text()}, {"role", role.currentData().toString()}, {"enabled", enabled.isChecked()}};
        auto *reply = m_net.put(request("/api/admin/users/" + QString::fromUtf8(QUrl::toPercentEncoding(oldUsername))), QJsonDocument(payload).toJson());
        connect(reply, &QNetworkReply::finished, this, [this, reply] {
            if (reply->error() != QNetworkReply::NoError)
                QMessageBox::warning(this, "Lỗi", reply->errorString());
            reply->deleteLater();
            refreshUsers();
        });
    }
}
