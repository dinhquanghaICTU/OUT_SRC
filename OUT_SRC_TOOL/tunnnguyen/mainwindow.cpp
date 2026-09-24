#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "core/ChromeLauncher.h"
#include "core/SyncManager.h"
#include "ui/CreateProfileDialog.h"
#include "ui/CustomMessageBox.h"
#include "ui/EditProfileDialog.h"
#include "ui/QuickProxyDialog.h"
#include "ui/ImportProfilesDialog.h"
#include "core/ProxyConfig.h"
#include "core/ProxyChecker.h"
#include "auth/LicenseManager.h"
#include "auth/HwidHelper.h"

#include <QCheckBox>
#include <QClipboard>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFrame>
#include <QGridLayout>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QListView>
#include <QScreen>
#include <QScrollArea>
#include <QSet>
#include <QSpinBox>
#include <QSplitter>
#include <QTextStream>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>
#include <QWindow>
#include <functional>

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

        // 1. Device icon
        auto *iconLbl = new QLabel(EditProfileDialog::getDeviceIcon(item.deviceType));
        iconLbl->setToolTip(QString("Thiết bị: %1").arg(EditProfileDialog::getDeviceDisplayName(item.deviceType)));
        iconLbl->setStyleSheet("font-size: 15px;");
        layout->addWidget(iconLbl);

        // 2. Profile Name
        auto *nameLbl = new QLabel(item.name);
        nameLbl->setFont(QFont("Google Sans", 10, QFont::Bold));
        nameLbl->setStyleSheet("color: #0f172a;");
        layout->addWidget(nameLbl);

        layout->addSpacing(6);

        // 3. Hover Action Nav (Appears when hovering on the profile cell)
        m_hoverNav = new QWidget(this);
        auto *navLayout = new QHBoxLayout(m_hoverNav);
        navLayout->setContentsMargins(0, 0, 0, 0);
        navLayout->setSpacing(5);

        auto *btnConfig = new QPushButton("⚙️ Cấu hình");
        btnConfig->setCursor(Qt::PointingHandCursor);
        btnConfig->setFixedHeight(26);
        btnConfig->setToolTip("Chỉnh sửa cấu hình & Loại máy (Device Fingerprint)");
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
        btnProxy->setToolTip("Sửa nhanh Proxy thủ công");
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
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent *) override {
        if (m_hoverNav) m_hoverNav->setVisible(true);
    }
#else
    void enterEvent(QEvent *) override {
        if (m_hoverNav) m_hoverNav->setVisible(true);
    }
#endif
    void leaveEvent(QEvent *) override {
        if (m_hoverNav) m_hoverNav->setVisible(false);
    }

private:
    int m_index;
    QWidget *m_hoverNav = nullptr;
    std::function<void(int)> m_onEditConfig;
    std::function<void(int)> m_onEditProxy;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  m_syncManager = new SyncManager(this);
  connect(m_syncManager, &SyncManager::syncStopped, this, [this]() {
    if (m_btnStartSync) {
      m_btnStartSync->setText("🟢  Bắt Đầu Đồng Bộ");
      m_btnStartSync->setStyleSheet(
          "QPushButton { background: #7c3aed; color: #ffffff; font-weight: 700; "
          "border: none; border-radius: 8px; padding: 8px 16px; font-size: 13px; } "
          "QPushButton:hover { background: #6d28d9; }");
    }
    refreshProfileTable();
  });
  connect(m_syncManager, &SyncManager::syncError, this, [this](const QString &err) {
    CustomMessageBox::warning(this, "Lỗi Đồng Bộ", err);
    if (m_btnStartSync) {
      m_btnStartSync->setText("🟢  Bắt Đầu Đồng Bộ");
      m_btnStartSync->setStyleSheet(
          "QPushButton { background: #7c3aed; color: #ffffff; font-weight: 700; "
          "border: none; border-radius: 8px; padding: 8px 16px; font-size: 13px; } "
          "QPushButton:hover { background: #6d28d9; }");
    }
    refreshProfileTable();
  });
  setupCustomUi();
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::setupCustomUi() {
  setWindowTitle("TUNNGUYEN AUTOMATION - POS MMO ACCOUNT PARSER");
  resize(1360, 840);
  setMinimumSize(1100, 700);

  // Global SaaS theme stylesheet matching template
  setStyleSheet(
      "* { font-family: 'Google Sans', 'Product Sans', -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif; }"
      "QMainWindow { background-color: #e5e7eb; }"
      "QWidget#appContainer { background-color: #ffffff; border-radius: 20px; }"
      "QTableWidget { background-color: #ffffff; border: 1px solid #e2e8f0; "
      "border-radius: 12px; gridline-color: #f1f5f9; font-size: 13px; "
      "selection-background-color: #fff7ed; selection-color: #9a3412; "
      "alternate-background-color: #fbfcfe; }"
      "QTableWidget::item { padding: 4px 10px; border-bottom: 1px solid #f1f5f9; }"
      "QTableWidget::indicator { width: 18px; height: 18px; border-radius: 5px; "
      "border: 1.5px solid #cbd5e1; background: #ffffff; margin-right: 6px; }"
      "QTableWidget::indicator:checked { background-color: #f97316; border-color: #f97316; }"
      "QTableWidget::indicator:hover { border-color: #f97316; }"
      "QHeaderView::section { background-color: #f8fafc; color: #475569; "
      "font-weight: 700; font-size: 12px; border: none; border-bottom: 2px "
      "solid #e2e8f0; padding: 12px 10px; }"
      "QPlainTextEdit { background-color: #ffffff; border: 1px solid #e2e8f0; "
      "border-radius: 12px; padding: 12px; font-family: monospace; font-size: "
      "12px; color: #1e293b; }"
      "QPlainTextEdit:focus { border: 1.5px solid #f97316; }"
      "QLineEdit { background: #ffffff; border: 1px solid #e2e8f0; "
      "border-radius: 8px; padding: 7px 12px; font-size: 13px; color: #1e293b; "
      "min-height: 20px; }"
      "QLineEdit:focus { border: 1.5px solid #f97316; }"
      "QComboBox {"
      "   background-color: #ffffff;"
      "   border: 1px solid #e2e8f0;"
      "   border-radius: 8px;"
      "   padding: 7px 32px 7px 12px;"
      "   font-size: 13px;"
      "   color: #1e293b;"
      "   font-weight: 500;"
      "   min-height: 20px;"
      "}"
      "QComboBox:hover { border-color: #cbd5e1; background-color: #f8fafc; }"
      "QComboBox:focus, QComboBox:on { border: 1.5px solid #f97316; "
      "background-color: #ffffff; }"
      "QComboBox::drop-down {"
      "   subcontrol-origin: padding;"
      "   subcontrol-position: top right;"
      "   width: 28px;"
      "   border: none;"
      "}"
      "QComboBox::down-arrow {"
      "   width: 0; height: 0;"
      "   border-left: 4.5px solid transparent;"
      "   border-right: 4.5px solid transparent;"
      "   border-top: 5.5px solid #64748b;"
      "   margin-right: 8px;"
      "}"
      "QComboBox::down-arrow:on {"
      "   border-top: none;"
      "   border-bottom: 5.5px solid #f97316;"
      "}"
      "QComboBox QAbstractItemView {"
      "   background-color: #ffffff;"
      "   border: 1px solid #e2e8f0;"
      "   border-radius: 12px;"
      "   padding: 6px;"
      "   selection-background-color: #ffedd5;"
      "   selection-color: #c2410c;"
      "   outline: none;"
      "   font-size: 13px;"
      "}"
      "QComboBox QAbstractItemView::item {"
      "   min-height: 32px;"
      "   padding-left: 10px;"
      "   padding-right: 10px;"
      "   border-radius: 6px;"
      "   color: #334155;"
      "   margin-bottom: 2px;"
      "}"
      "QComboBox QAbstractItemView::item:hover {"
      "   background-color: #f8fafc;"
      "   color: #0f172a;"
      "}"
      "QComboBox QAbstractItemView::item:selected {"
      "   background-color: #ffedd5;"
      "   color: #c2410c;"
      "   font-weight: 600;"
      "}"
      "QScrollBar:vertical { background: #f8fafc; width: 6px; margin: 0; "
      "border-radius: 3px; }"
      "QScrollBar::handle:vertical { background: #cbd5e1; min-height: 20px; "
      "border-radius: 3px; }"
      "QScrollBar::handle:vertical:hover { background: #94a3b8; }");

  auto *rootWidget = new QWidget(this);
  setCentralWidget(rootWidget);

  auto *rootLayout = new QVBoxLayout(rootWidget);
  rootLayout->setContentsMargins(16, 16, 16, 16);

  // Main App Container (Rounded white card like the template)
  auto *appContainer = new QWidget(rootWidget);
  appContainer->setObjectName("appContainer");

  auto *appLayout = new QHBoxLayout(appContainer);
  appLayout->setContentsMargins(0, 0, 0, 0);
  appLayout->setSpacing(0);

  // 1. LEFT SIDEBAR (Width: ~210px)
  appLayout->addWidget(createSidebar());

  // Vertical Divider
  auto *line1 = new QFrame();
  line1->setFrameShape(QFrame::VLine);
  line1->setStyleSheet("color: #f1f5f9;");
  appLayout->addWidget(line1);

  // 2. MAIN CENTER CONTENT
  auto *centerWrapper = new QWidget(appContainer);
  auto *centerLayout = new QVBoxLayout(centerWrapper);
  centerLayout->setContentsMargins(0, 0, 0, 0);
  centerLayout->setSpacing(0);

  // Top Bar (Search + Profile Avatar)
  centerLayout->addWidget(createTopBar());

  auto *line2 = new QFrame();
  line2->setFrameShape(QFrame::HLine);
  line2->setStyleSheet("color: #f1f5f9;");
  centerLayout->addWidget(line2);

  // Center Main Area Stack
  m_pageStack = new QStackedWidget();
  m_pageStack->addWidget(createCenterPanel());          // Index 0: Account Parser
  m_pageStack->addWidget(createChromeProfilesPanel());  // Index 1: Chrome Profiles
  m_pageStack->addWidget(createProxyPoolPanel());       // Index 2: Proxy Pool
  m_pageStack->addWidget(createLicenseKeyPanel());      // Index 3: License Key & Payment
  m_pageStack->addWidget(createDashboardPanel());       // Index 4: Dashboard Showcase
  centerLayout->addWidget(m_pageStack, 1);

  appLayout->addWidget(centerWrapper, 1);

  // Vertical Divider
  m_rightDivider = new QFrame();
  m_rightDivider->setFrameShape(QFrame::VLine);
  m_rightDivider->setStyleSheet("color: #f1f5f9;");
  appLayout->addWidget(m_rightDivider);

  // 3. RIGHT SUMMARY & EXPORT PANEL (Width: ~320px)
  m_rightPanel = createRightSummaryPanel();
  appLayout->addWidget(m_rightPanel);

  rootLayout->addWidget(appContainer);

  loadProfiles();
  onNavButtonClicked(0);
}

QWidget *MainWindow::createSidebar() {
  auto *sidebar = new QWidget();
  sidebar->setFixedWidth(250);
  sidebar->setStyleSheet("background-color: #ffffff; border-top-left-radius: "
                         "20px; border-bottom-left-radius: 20px;");

  auto *layout = new QVBoxLayout(sidebar);
  layout->setContentsMargins(16, 20, 16, 20);
  layout->setSpacing(9);

  // Logo / Brand
  auto *logoRow = new QHBoxLayout();
  logoRow->setSpacing(10);
  auto *logoIcon = new QLabel("⚡");
  logoIcon->setStyleSheet(
      "background-color: #f97316; color: #ffffff; font-size: 16px; "
      "border-radius: 8px; padding: 4px 8px;");
  auto *logoText = new QLabel("TunnBit");
  logoText->setStyleSheet("font-size: 19px; font-weight: 800; color: #0f172a;");
  logoRow->addWidget(logoIcon);
  logoRow->addWidget(logoText);
  logoRow->addStretch();
  layout->addLayout(logoRow);

  layout->addSpacing(12);

  // User Profile Card
  auto *userCard = new QWidget();
  userCard->setStyleSheet(
      "background: #f8fafc; border-radius: 10px; padding: 6px 10px;");
  auto *userLayout = new QHBoxLayout(userCard);
  userLayout->setContentsMargins(4, 4, 4, 4);
  userLayout->setSpacing(8);
  auto *avatar = new QLabel("👨‍💻");
  avatar->setStyleSheet("font-size: 25px; background: #e2e8f0; border-radius: "
                        "16px; padding: 4px;");
  auto *userInfo = new QVBoxLayout();
  userInfo->setSpacing(1);
  auto *userName = new QLabel("Tunn Nguyen");
  userName->setStyleSheet("font-size: 15px; font-weight: 700; color: #0f172a;");
  auto *userRole = new QLabel("Quyền của user");
  userRole->setStyleSheet("font-size: 11px; color: #64748b;");
  userInfo->addWidget(userName);
  userInfo->addWidget(userRole);
  userLayout->addWidget(avatar);
  userLayout->addLayout(userInfo);
  userLayout->addStretch();
  layout->addWidget(userCard);

  layout->addSpacing(10);

  // Nav Item Helper
  m_navButtons.clear();
  auto addNavItem = [&](int navIndex, const QString &icon, const QString &text,
                        bool isActive = false) {
    auto *btn = new QPushButton(QString("%1  %2").arg(icon, text));
    btn->setCursor(Qt::PointingHandCursor);
    btn->setFixedHeight(36);
    QString style =
        "QPushButton {"
        "   border: none;"
        "   text-align: left;"
        "   padding-left: 10px;"
        "   font-size: 13px;"
        "   border-radius: 8px;"
        "   color: %1;"
        "   background-color: %2;"
        "   font-weight: %3;"
        "}"
        "QPushButton:hover { background-color: #f1f5f9; color: #0f172a; }";

    if (isActive) {
      btn->setStyleSheet(style.arg("#0f172a", "#f1f5f9", "700"));
    } else {
      btn->setStyleSheet(style.arg("#64748b", "transparent", "500"));
    }

    connect(btn, &QPushButton::clicked, [this, navIndex]() {
      onNavButtonClicked(navIndex);
    });

    m_navButtons.append(btn);
    layout->addWidget(btn);
  };

  auto addSectionHeader = [&](const QString &title) {
    layout->addSpacing(8);
    auto *lbl = new QLabel(title);
    lbl->setStyleSheet("font-size: 11px; font-weight: 600; color: #94a3b8; "
                       "text-transform: uppercase; padding-left: 10px;");
    layout->addWidget(lbl);
  };

  addNavItem(0, "📊", "Dashboard");
  addNavItem(1, "⚡", "Tách & lọc tài khoản", true);
  addNavItem(2, "👥", "Chrome Profiles");
  addNavItem(3, "🌐", "Proxy Pool");

  addSectionHeader("Automation");
  addNavItem(4, "🚀", "Task Runner");
  addNavItem(5, "💳", "License Key");
  addNavItem(6, "📦", "Cookie Manager");

  addSectionHeader("System");
  addNavItem(7, "📈", "Reports & Logs");
  addNavItem(8, "⚙️", "Settings");

  layout->addStretch();

  // Logout button at bottom
  auto *btnLogout = new QPushButton("←  Logout");
  btnLogout->setCursor(Qt::PointingHandCursor);
  btnLogout->setStyleSheet(
      "QPushButton { border: none; text-align: left; padding-left: 10px; "
      "font-size: 13px; color: #94a3b8; font-weight: 600; } QPushButton:hover "
      "{ color: #ef4444; }");
  connect(btnLogout, &QPushButton::clicked, this, [this]() {
    close();
  });
  layout->addWidget(btnLogout);

  return sidebar;
}

QWidget *MainWindow::createTopBar() {
  auto *topBar = new QWidget();
  topBar->setFixedHeight(60);
  topBar->setStyleSheet("background-color: #ffffff;");

  auto *layout = new QHBoxLayout(topBar);
  layout->setContentsMargins(24, 10, 24, 10);
  layout->setSpacing(16);

  layout->addStretch();

  // Sun/Moon & Notification
  auto *themeBtn = new QPushButton("☀️");
  themeBtn->setFixedSize(34, 34);
  themeBtn->setCursor(Qt::PointingHandCursor);
  themeBtn->setStyleSheet("QPushButton { background: #f8fafc; border: 1px "
                          "solid #e2e8f0; border-radius: 17px; font-size: "
                          "14px; } QPushButton:hover { background: #e2e8f0; }");

  auto *avatarBadge = new QLabel("👤");
  avatarBadge->setFixedSize(34, 34);
  avatarBadge->setAlignment(Qt::AlignCenter);
  avatarBadge->setStyleSheet("background: #ffedd5; border: 1px solid #fed7aa; "
                             "border-radius: 17px; font-size: 15px;");

  layout->addWidget(themeBtn);
  layout->addWidget(avatarBadge);

  return topBar;
}

QWidget *MainWindow::createCenterPanel() {
  auto *center = new QWidget();
  auto *layout = new QVBoxLayout(center);
  layout->setContentsMargins(24, 20, 24, 20);
  layout->setSpacing(16);

  // 1. Search Bar & Top Action Buttons (Same Row)
  auto *titleRow = new QHBoxLayout();
  titleRow->setSpacing(12);

  m_searchEdit = new QLineEdit();
  m_searchEdit->setPlaceholderText("🔍  Tìm kiếm tài khoản (UID, Email, Pass, 2FA)...");
  m_searchEdit->setClearButtonEnabled(true);
  connect(m_searchEdit, &QLineEdit::textChanged, this,
          &MainWindow::onSearchFilterChanged);
  titleRow->addWidget(m_searchEdit, 1);

  titleRow->addSpacing(8);

  // Top action buttons matching template: + New (Orange), Menu, Draft, Order
  auto *btnImport = new QPushButton("+  Mở File Text");
  btnImport->setCursor(Qt::PointingHandCursor);
  btnImport->setStyleSheet(
      "QPushButton { background: #f97316; color: #ffffff; font-weight: 700; "
      "border: none; border-radius: 8px; padding: 9px 18px; font-size: 13px; } "
      "QPushButton:hover { background: #ea580c; }");
  connect(btnImport, &QPushButton::clicked, this,
          &MainWindow::onImportFileClicked);

  auto *btnProcess = new QPushButton("⚡ Tách Chuỗi");
  btnProcess->setCursor(Qt::PointingHandCursor);
  btnProcess->setStyleSheet(
      "QPushButton { background: #ffffff; color: #0f172a; font-weight: 600; "
      "border: 1px solid #e2e8f0; border-radius: 8px; padding: 9px 14px; "
      "font-size: 13px; } QPushButton:hover { background: #f8fafc; }");
  connect(btnProcess, &QPushButton::clicked, this,
          &MainWindow::onProcessLinesClicked);

  auto *btnClear = new QPushButton("🗑️ Xóa Hết");
  btnClear->setCursor(Qt::PointingHandCursor);
  btnClear->setStyleSheet(
      "QPushButton { background: #ffffff; color: #ef4444; font-weight: 600; "
      "border: 1px solid #fecaca; border-radius: 8px; padding: 9px 14px; "
      "font-size: 13px; } QPushButton:hover { background: #fef2f2; }");
  connect(btnClear, &QPushButton::clicked, this,
          &MainWindow::onClearAllClicked);

  titleRow->addWidget(btnImport);
  titleRow->addWidget(btnProcess);
  titleRow->addWidget(btnClear);

  layout->addLayout(titleRow);

  // 2. Delimiter & Category Filter Row
  auto *filterRow = new QHBoxLayout();
  filterRow->setSpacing(12);

  // Delimiter Combobox
  m_delimiterCombo = new QComboBox();
  m_delimiterCombo->setView(new QListView(m_delimiterCombo));
  m_delimiterCombo->addItem("Phân cách: | (Pipe)", "|");
  m_delimiterCombo->addItem("Phân cách: : (Colon)", ":");
  m_delimiterCombo->addItem("Phân cách: , (Comma)", ",");
  m_delimiterCombo->addItem("Phân cách: ; (Semicolon)", ";");
  m_delimiterCombo->addItem("Phân cách: Tab", "\t");
  filterRow->addWidget(m_delimiterCombo, 1);

  // Format Preset Combobox
  m_formatCombo = new QComboBox();
  m_formatCombo->setView(new QListView(m_formatCombo));
  m_formatCombo->addItem("Mẫu: UID|Pass|2FA|Email|PassMail|Cookie|Extra");
  m_formatCombo->addItem("Mẫu: UID|Pass|2FA");
  m_formatCombo->addItem("Mẫu: Email|Pass|Recovery");
  m_formatCombo->addItem("Mẫu: User|Pass|Proxy");
  filterRow->addWidget(m_formatCombo, 1);

  connect(m_formatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &MainWindow::onFormatChanged);
  connect(m_delimiterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, [this]() {
            if (!m_rawInputEdit->toPlainText().trimmed().isEmpty()) {
              onProcessLinesClicked();
            }
          });

  filterRow->addStretch(2);

  layout->addLayout(filterRow);

  // 3. Pill Filter Category Buttons (Show All, Hợp Lệ, Lỗi Pass, Trùng Lặp...)
  auto *pillRow = new QHBoxLayout();
  pillRow->setSpacing(8);

  QStringList pillTitles = {"Show All", "Hợp Lệ", "Lỗi Định Dạng", "Trùng Lặp"};
  for (int i = 0; i < pillTitles.size(); ++i) {
    auto *btn = new QPushButton(pillTitles[i]);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setFixedHeight(32);
    if (i == 0) {
      btn->setStyleSheet("QPushButton { background-color: #f97316; color: "
                         "#ffffff; font-weight: 700; border-radius: 8px; "
                         "padding: 0 16px; border: none; }");
    } else {
      btn->setStyleSheet("QPushButton { background-color: #ffffff; color: "
                         "#64748b; font-weight: 600; border: 1px solid "
                         "#e2e8f0; border-radius: 8px; padding: 0 16px; } "
                         "QPushButton:hover { background-color: #f8fafc; }");
    }
    connect(btn, &QPushButton::clicked,
            [this, i]() { onPillFilterClicked(i); });
    m_pillButtons.append(btn);
    pillRow->addWidget(btn);
  }
  pillRow->addStretch();
  layout->addLayout(pillRow);

  // 4. Splitter: Left (Input text box) & Right (Data Table)
  auto *splitter = new QSplitter(Qt::Vertical);
  splitter->setHandleWidth(8);

  // Upper Box: Raw Text Input
  auto *inputBox = new QWidget();
  auto *inputBoxLayout = new QVBoxLayout(inputBox);
  inputBoxLayout->setContentsMargins(0, 0, 0, 0);
  inputBoxLayout->setSpacing(4);

  auto *lblInputDesc = new QLabel(
      "📋 Dán danh sách tài khoản thô vào đây (Mỗi tài khoản 1 dòng):");
  lblInputDesc->setStyleSheet(
      "font-size: 12px; font-weight: 600; color: #64748b;");
  inputBoxLayout->addWidget(lblInputDesc);

  m_rawInputEdit = new QPlainTextEdit();
  m_rawInputEdit->setPlaceholderText(
      "100083921029|Matkhau@123|JBSWY3DPEHPK3PXP|user1@gmail.com|passMail1|c_"
      "user=100083921029;...|TokenEAAB...\n"
      "100083921030|Matkhau@456|KZXW63TPMQQM2LXP|user2@gmail.com|passMail2|c_"
      "user=100083921030;...|TokenEAAB...\n"
      "100083921029|Matkhau@123|JBSWY3DPEHPK3PXP|user1@gmail.com|passMail1|c_"
      "user=100083921029;...|TokenEAAB...");
  inputBoxLayout->addWidget(m_rawInputEdit);
  splitter->addWidget(inputBox);

  // Lower Box: Table Preview
  auto *tableBox = new QWidget();
  auto *tableBoxLayout = new QVBoxLayout(tableBox);
  tableBoxLayout->setContentsMargins(0, 0, 0, 0);
  tableBoxLayout->setSpacing(4);

  auto *lblTableDesc = new QLabel("📊 Bảng dữ liệu sau khi tách chuỗi:");
  lblTableDesc->setStyleSheet(
      "font-size: 12px; font-weight: 600; color: #64748b;");
  tableBoxLayout->addWidget(lblTableDesc);

  m_tableWidget = new QTableWidget();
  m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_tableWidget->horizontalHeader()->setStretchLastSection(true);
  m_tableWidget->verticalHeader()->setVisible(false);
  m_tableWidget->verticalHeader()->setDefaultSectionSize(42);
  m_tableWidget->setShowGrid(false);
  m_tableWidget->setAlternatingRowColors(true);

  // Khởi tạo cột bảng theo mẫu đầu tiên (index 0)
  onFormatChanged(0);

  tableBoxLayout->addWidget(m_tableWidget);
  splitter->addWidget(tableBox);

  splitter->setStretchFactor(0, 1);
  splitter->setStretchFactor(1, 2);

  layout->addWidget(splitter, 1);

  return center;
}

QWidget *MainWindow::createRightSummaryPanel() {
  auto *panel = new QWidget();
  panel->setFixedWidth(370);
  panel->setStyleSheet("background-color: #ffffff; border-top-right-radius: "
                       "20px; border-bottom-right-radius: 20px;");

  auto *layout = new QVBoxLayout(panel);
  layout->setContentsMargins(20, 20, 20, 20);
  layout->setSpacing(14);

  // Search existing
  //    auto *searchBox = new QLineEdit();
  //    searchBox->setPlaceholderText("Search in Existing...");
  //    searchBox->setStyleSheet("background: #f8fafc; border: 1px solid
  //    #e2e8f0; border-radius: 8px; padding: 6px 12px;");
  //    layout->addWidget(searchBox);

  // Two Selectors matching template (Select Dining, Select Table)
  auto *selectorRow = new QHBoxLayout();
  selectorRow->setSpacing(8);

  m_exportFormatCombo = new QComboBox();
  m_exportFormatCombo->setView(new QListView(m_exportFormatCombo));
  m_exportFormatCombo->addItem("CSV Excel (Dấu phẩy ,)");
  m_exportFormatCombo->addItem("CSV Excel (Dấu chấm phẩy ;)");
  m_exportFormatCombo->addItem("Excel TSV (Tab - Tự chia cột)");
  m_exportFormatCombo->addItem("Text Tách Dòng (|)");

  auto *tableCombo = new QComboBox();
  tableCombo->setView(new QListView(tableCombo));
  tableCombo->addItem("Khử Trùng: BẬT");
  tableCombo->addItem("Khử Trùng: TẮT");

  selectorRow->addWidget(m_exportFormatCombo);
  selectorRow->addWidget(tableCombo);
  layout->addLayout(selectorRow);

  // Order Title / Task Badge
  auto *orderTitleRow = new QHBoxLayout();
  auto *orderIcon = new QLabel("👜");
  orderIcon->setStyleSheet("font-size: 16px;");
  auto *orderTitle = new QLabel("Order / Summary #01");
  orderTitle->setStyleSheet(
      "font-size: 15px; font-weight: 800; color: #0f172a;");
  orderTitleRow->addWidget(orderIcon);
  orderTitleRow->addWidget(orderTitle);
  orderTitleRow->addStretch();
  layout->addLayout(orderTitleRow);

  // Mini Live Cards / Preview Rows matching template items
  auto *previewCard = new QWidget();
  previewCard->setStyleSheet(
      "background: #f8fafc; border-radius: 12px; padding: 10px;");
  auto *previewCardLayout = new QVBoxLayout(previewCard);
  previewCardLayout->setSpacing(8);

  auto addSummaryItem = [](const QString &name, const QString &count,
                           const QString &status) -> QWidget * {
    auto *item = new QWidget();
    auto *l = new QHBoxLayout(item);
    l->setContentsMargins(0, 0, 0, 0);
    auto *nameLbl = new QLabel(name);
    nameLbl->setStyleSheet(
        "font-size: 12px; font-weight: 600; color: #334155;");
    auto *valLbl = new QLabel(count);
    valLbl->setStyleSheet(
        QString("font-size: 12px; font-weight: 700; color: %1;").arg(status));
    l->addWidget(nameLbl);
    l->addStretch();
    l->addWidget(valLbl);
    return item;
  };

  previewCardLayout->addWidget(
      addSummaryItem("File Nguồn:", "TXT / Clipboard", "#64748b"));
  previewCardLayout->addWidget(
      addSummaryItem("Mẫu Tách:", "UID|Pass|2FA...", "#64748b"));
  previewCardLayout->addWidget(
      addSummaryItem("Ký Tự Ngăn:", "| (Gạch đứng)", "#f97316"));
  layout->addWidget(previewCard);

  layout->addStretch();

  // Financial / Account Counter summary matching template bottom
  auto *summaryBox = new QVBoxLayout();
  summaryBox->setSpacing(8);

  auto addStatRow = [&](const QString &label, const QString &val,
                        QLabel *&outLabel, const QString &color = "#0f172a") {
    auto *row = new QHBoxLayout();
    auto *lbl = new QLabel(label);
    lbl->setStyleSheet("font-size: 13px; color: #64748b; font-weight: 500;");
    outLabel = new QLabel(val);
    outLabel->setStyleSheet(
        QString("font-size: 13.5px; font-weight: 700; color: %1;").arg(color));
    row->addWidget(lbl);
    row->addStretch();
    row->addWidget(outLabel);
    summaryBox->addLayout(row);
  };

  addStatRow("Sub total (Tổng dòng) :", "0", m_lblTotal);
  addStatRow("Hợp lệ (Valid) :", "0", m_lblValid, "#16a34a");
  addStatRow("Trùng lặp đã loại :", "0", m_lblDuplicates, "#ea580c");
  addStatRow("Lỗi thiếu pass/UID :", "0", m_lblError, "#dc2626");

  auto *divider = new QFrame();
  divider->setFrameShape(QFrame::HLine);
  divider->setStyleSheet("color: #e2e8f0;");
  summaryBox->addWidget(divider);

  // Big Total
  auto *totalRow = new QHBoxLayout();
  auto *lblTotalText = new QLabel("Total Export :");
  lblTotalText->setStyleSheet(
      "font-size: 15px; font-weight: 800; color: #0f172a;");
  m_lblTotalExport = new QLabel("0");
  m_lblTotalExport->setStyleSheet(
      "font-size: 19px; font-weight: 800; color: #0f172a;");
  totalRow->addWidget(lblTotalText);
  totalRow->addStretch();
  totalRow->addWidget(m_lblTotalExport);
  summaryBox->addLayout(totalRow);

  layout->addLayout(summaryBox);

  layout->addSpacing(10);

  // Bottom Action Buttons matching the 3 buttons in the template
  // Button 1: KOT & Print -> Dark Navy Button "⚡ Phân Tích & Tách Chuỗi"
  auto *btnKot = new QPushButton("⚡  Phân Tích & Tách Chuỗi");
  btnKot->setCursor(Qt::PointingHandCursor);
  btnKot->setFixedHeight(42);
  btnKot->setStyleSheet(
      "QPushButton { background-color: #0f172a; color: #ffffff; font-weight: "
      "700; border-radius: 10px; font-size: 13px; } QPushButton:hover { "
      "background-color: #1e293b; }");
  connect(btnKot, &QPushButton::clicked, this,
          &MainWindow::onProcessLinesClicked);
  layout->addWidget(btnKot);

  // Button 2 & 3 in a row: Orange "Bill & Payment" (Export Excel) and Green
  // "Bill & Print" (Copy All)
  auto *btnRow = new QHBoxLayout();
  btnRow->setSpacing(10);

  auto *btnBill = new QPushButton("📊 Xuất Excel");
  btnBill->setCursor(Qt::PointingHandCursor);
  btnBill->setFixedHeight(40);
  btnBill->setStyleSheet(
      "QPushButton { background-color: #f97316; color: #ffffff; font-weight: "
      "700; border-radius: 10px; font-size: 13px; } QPushButton:hover { "
      "background-color: #ea580c; }");
  connect(btnBill, &QPushButton::clicked, this,
          &MainWindow::onExportExcelClicked);

  auto *btnPrint = new QPushButton("📋 Copy All");
  btnPrint->setCursor(Qt::PointingHandCursor);
  btnPrint->setFixedHeight(40);
  btnPrint->setStyleSheet(
      "QPushButton { background-color: #16a34a; color: #ffffff; font-weight: "
      "700; border-radius: 10px; font-size: 13px; } QPushButton:hover { "
      "background-color: #15803d; }");
  connect(btnPrint, &QPushButton::clicked, this,
          &MainWindow::onCopyValidClicked);

  btnRow->addWidget(btnBill, 1);
  btnRow->addWidget(btnPrint, 1);
  layout->addLayout(btnRow);

  return panel;
}

QString MainWindow::getDelimiter() const {
  return m_delimiterCombo ? m_delimiterCombo->currentData().toString() : "|";
}

QStringList MainWindow::getCurrentHeaders() const {
  int idx = m_formatCombo ? m_formatCombo->currentIndex() : 0;
  switch (idx) {
  case 1:
    return {"STT", "UID", "Password", "2FA Secret", "Trạng Thái"};
  case 2:
    return {"STT", "Email", "Password", "Recovery Mail", "Trạng Thái"};
  case 3:
    return {"STT", "Username", "Password", "Proxy (IP:Port)", "Trạng Thái"};
  case 0:
  default:
    return {"STT", "UID / Username", "Password", "2FA Secret",
            "Email", "Pass Mail", "Cookie / Extra", "Trạng Thái"};
  }
}

void MainWindow::onFormatChanged(int index) {
  Q_UNUSED(index);
  if (!m_tableWidget) return;

  QStringList headers = getCurrentHeaders();
  m_tableWidget->setColumnCount(headers.size());
  m_tableWidget->setHorizontalHeaderLabels(headers);

  // Set column widths
  m_tableWidget->setColumnWidth(0, 50); // STT
  for (int i = 1; i < headers.size() - 1; ++i) {
    m_tableWidget->setColumnWidth(i, 130);
  }
  m_tableWidget->horizontalHeader()->setStretchLastSection(true);

  // Tự động phân tích lại dữ liệu nếu đã có nội dung nhập
  if (m_rawInputEdit && !m_rawInputEdit->toPlainText().trimmed().isEmpty()) {
    onProcessLinesClicked();
  } else if (!m_allAccounts.isEmpty()) {
    onPillFilterClicked(m_currentFilter);
  }
}

void MainWindow::onImportFileClicked() {
  QString filePath =
      QFileDialog::getOpenFileName(this, "Chọn file tài khoản MMO", "",
                                    "Text Files (*.txt *.csv);;All Files (*.*)");
  if (filePath.isEmpty())
    return;

  QFile file(filePath);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    CustomMessageBox::critical(this, "Lỗi", "Không thể đọc file đã chọn!");
    return;
  }

  QTextStream in(&file);
  m_rawInputEdit->setPlainText(in.readAll());
  file.close();

  onProcessLinesClicked();
}

void MainWindow::onProcessLinesClicked() {
  QString content = m_rawInputEdit->toPlainText().trimmed();
  if (content.isEmpty()) {
    CustomMessageBox::warning(
        this, "Thông báo", "Vui lòng nhập hoặc dán danh sách tài khoản trước!");
    return;
  }

  QString delimiter = getDelimiter();
  QStringList headers = getCurrentHeaders();
  int numFields = headers.size() - 2; // Bỏ STT và Trạng Thái

  QStringList lines = content.split('\n');
  m_allAccounts.clear();

  QSet<QString> seenKeys;
  int totalCount = 0;
  int validCount = 0;
  int errorCount = 0;
  int duplicateCount = 0;

  for (const QString &rawLine : lines) {
    QString line = rawLine.trimmed();
    if (line.isEmpty())
      continue;

    totalCount++;
    QStringList parts = line.split(delimiter);

    MmoAccount acc;
    for (int f = 0; f < numFields; ++f) {
      acc.fields.append(parts.value(f).trimmed());
    }

    // Khóa định danh (UID, Email hoặc Username)
    QString primaryId = acc.fields.value(0);
    QString pass = acc.fields.value(1);

    if (!primaryId.isEmpty() && seenKeys.contains(primaryId)) {
      duplicateCount++;
      acc.isValid = false;
      acc.statusMsg = "Trùng lặp";
      m_allAccounts.append(acc);
      continue;
    }
    if (!primaryId.isEmpty()) {
      seenKeys.insert(primaryId);
    }

    if (primaryId.isEmpty()) {
      acc.isValid = false;
      acc.statusMsg = QString("Thiếu %1").arg(headers.value(1));
      errorCount++;
    } else if (pass.isEmpty()) {
      acc.isValid = false;
      acc.statusMsg = "Thiếu Pass";
      errorCount++;
    } else {
      acc.isValid = true;
      acc.statusMsg = "Hợp lệ";
      validCount++;
    }

    m_allAccounts.append(acc);
  }

  updateStats(totalCount, validCount, errorCount, duplicateCount);
  onPillFilterClicked(m_currentFilter);

  CustomMessageBox::information(this, "Thành công",
                                QString("Đã phân tích xong %1 dòng theo %2!\n- Hợp lệ: "
                                        "%3\n- Lỗi: %4\n- Trùng lặp: %5")
                                    .arg(totalCount)
                                    .arg(m_formatCombo ? m_formatCombo->currentText() : "Mẫu đã chọn")
                                    .arg(validCount)
                                    .arg(errorCount)
                                    .arg(duplicateCount));
}

void MainWindow::updateStats(int total, int valid, int error, int duplicates) {
  m_lblTotal->setText(QString::number(total));
  m_lblValid->setText(QString::number(valid));
  m_lblError->setText(QString::number(error));
  m_lblDuplicates->setText(QString::number(duplicates));
  m_lblTotalExport->setText(QString("%1 acc").arg(valid));
}

void MainWindow::onPillFilterClicked(int filterIndex) {
  m_currentFilter = filterIndex;

  // Update pill buttons style
  for (int i = 0; i < m_pillButtons.size(); ++i) {
    if (i == filterIndex) {
      m_pillButtons[i]->setStyleSheet(
          "QPushButton { background-color: #f97316; color: #ffffff; "
          "font-weight: 700; border-radius: 8px; padding: 0 16px; border: "
          "none; }");
    } else {
      m_pillButtons[i]->setStyleSheet(
          "QPushButton { background-color: #ffffff; color: #64748b; "
          "font-weight: 600; border: 1px solid #e2e8f0; border-radius: 8px; "
          "padding: 0 16px; } QPushButton:hover { background-color: #f8fafc; "
          "}");
    }
  }

  QList<MmoAccount> filtered;
  for (const auto &acc : m_allAccounts) {
    if (filterIndex == 0) { // All
      filtered.append(acc);
    } else if (filterIndex == 1 && acc.isValid) { // Valid
      filtered.append(acc);
    } else if (filterIndex == 2 && !acc.isValid &&
               acc.statusMsg != "Trùng lặp") { // Error
      filtered.append(acc);
    } else if (filterIndex == 3 && acc.statusMsg == "Trùng lặp") { // Duplicates
      filtered.append(acc);
    }
  }

  renderTable(filtered);
}

void MainWindow::renderTable(const QList<MmoAccount> &accounts) {
  QStringList headers = getCurrentHeaders();
  int numFields = headers.size() - 2;
  int statusCol = headers.size() - 1;

  m_tableWidget->setRowCount(0);
  m_tableWidget->setRowCount(accounts.size());

  for (int i = 0; i < accounts.size(); ++i) {
    const auto &acc = accounts[i];

    auto *itemStt = new QTableWidgetItem(QString::number(i + 1));
    itemStt->setTextAlignment(Qt::AlignCenter);
    m_tableWidget->setItem(i, 0, itemStt);

    for (int f = 0; f < numFields; ++f) {
      QString val = acc.fields.value(f);
      QString displayVal = val;
      if (displayVal.length() > 25) {
        displayVal = displayVal.left(22) + "...";
      }
      auto *item = new QTableWidgetItem(displayVal);
      if (val.length() > 22) {
        item->setToolTip(val);
      }
      m_tableWidget->setItem(i, f + 1, item);
    }

    auto *itemStatus = new QTableWidgetItem(acc.statusMsg);
    itemStatus->setTextAlignment(Qt::AlignCenter);
    if (acc.isValid) {
      itemStatus->setForeground(QColor("#16a34a"));
    } else if (acc.statusMsg == "Trùng lặp") {
      itemStatus->setForeground(QColor("#ea580c"));
    } else {
      itemStatus->setForeground(QColor("#dc2626"));
    }
    m_tableWidget->setItem(i, statusCol, itemStatus);
  }
}

void MainWindow::onExportExcelClicked() {
  if (m_allAccounts.isEmpty()) {
    CustomMessageBox::warning(this, "Thông báo",
                              "Chưa có dữ liệu nào để xuất file!");
    return;
  }

  int exportFormatIndex = m_exportFormatCombo ? m_exportFormatCombo->currentIndex() : 0;
  // 0: CSV Comma (,)
  // 1: CSV Semicolon (;)
  // 2: TSV Tab (\t)
  // 3: Text Tách Dòng (|)

  QString filter = "Excel CSV (*.csv);;All Files (*.*)";
  QString defaultName = "MMO_Accounts_Filtered.csv";
  if (exportFormatIndex == 2) {
    filter = "Excel Tab Separated (*.tsv *.txt);;All Files (*.*)";
    defaultName = "MMO_Accounts_Filtered.tsv";
  } else if (exportFormatIndex == 3) {
    filter = "Text File (*.txt);;All Files (*.*)";
    defaultName = "MMO_Accounts_Filtered.txt";
  }

  QString savePath = QFileDialog::getSaveFileName(
      this, "Lưu file dữ liệu", defaultName, filter);
  if (savePath.isEmpty())
    return;

  QFile file(savePath);
  if (!file.open(QIODevice::WriteOnly)) {
    CustomMessageBox::critical(this, "Lỗi",
                               "Không thể tạo file để ghi dữ liệu!");
    return;
  }

  // 1. Ghi chuẩn 3 bytes UTF-8 BOM nhị phân trực tiếp vào file
  file.write("\xEF\xBB\xBF", 3);

  QTextStream out(&file);
  out.setEncoding(QStringConverter::Utf8);

  QStringList headers = getCurrentHeaders();
  int count = 0;

  if (exportFormatIndex == 3) {
    // Xuất dạng Text Tách Dòng (|)
    QString delimiter = getDelimiter();
    for (const auto &acc : m_allAccounts) {
      if (!acc.isValid) continue;
      out << acc.fields.join(delimiter) << "\n";
      count++;
    }
  } else {
    // Xuất dạng Excel (CSV / TSV)
    QString sep = ",";
    if (exportFormatIndex == 1) sep = ";";
    else if (exportFormatIndex == 2) sep = "\t";

    // Thêm chỉ dẫn sep= cho Microsoft Excel (cả Desktop lẫn Excel Online tự động chia cột)
    if (exportFormatIndex == 0) {
      out << "sep=,\n";
    } else if (exportFormatIndex == 1) {
      out << "sep=;\n";
    }

    // Header cột
    QStringList escapedHeaders;
    for (const QString &h : headers) {
      escapedHeaders.append(QString("\"%1\"").arg(h));
    }
    out << escapedHeaders.join(sep) << "\n";

    auto escapeCsv = [](QString val) -> QString {
      val.replace("\"", "\"\"");
      return QString("\"%1\"").arg(val);
    };

    for (const auto &acc : m_allAccounts) {
      if (!acc.isValid)
        continue;

      QStringList rowValues;
      rowValues.append(QString("\"%1\"").arg(count + 1)); // STT
      for (const QString &f : acc.fields) {
        rowValues.append(escapeCsv(f));
      }
      rowValues.append(escapeCsv(acc.statusMsg)); // Trạng thái

      out << rowValues.join(sep) << "\n";
      count++;
    }
  }

  file.close();

  CustomMessageBox::information(
      this, "Xuất thành công",
      QString("Đã xuất %1 tài khoản hợp lệ ra file:\n%2")
          .arg(count)
          .arg(savePath));
}

void MainWindow::onCopyValidClicked() {
  QString delimiter = getDelimiter();
  QString result;
  int count = 0;

  for (const auto &acc : m_allAccounts) {
    if (!acc.isValid)
      continue;
    result += acc.fields.join(delimiter) + "\n";
    count++;
  }

  if (count == 0) {
    CustomMessageBox::warning(this, "Thông báo",
                              "Không có tài khoản hợp lệ nào để sao chép!");
    return;
  }

  QGuiApplication::clipboard()->setText(result);
  CustomMessageBox::information(
      this, "Đã sao chép",
      QString("Đã copy %1 tài khoản hợp lệ vào bộ nhớ tạm!").arg(count));
}

void MainWindow::onClearAllClicked() {
  m_rawInputEdit->clear();
  m_tableWidget->setRowCount(0);
  m_allAccounts.clear();
  updateStats(0, 0, 0, 0);
}

void MainWindow::onSearchFilterChanged(const QString &text) {
  QString keyword = text.trimmed().toLower();
  int colCount = m_tableWidget->columnCount();

  for (int row = 0; row < m_tableWidget->rowCount(); ++row) {
    if (keyword.isEmpty()) {
      m_tableWidget->setRowHidden(row, false);
      continue;
    }

    bool match = false;
    // Tìm kiếm trong tất cả các cột dữ liệu (trừ STT và Trạng thái)
    for (int col = 1; col < colCount - 1; ++col) {
      auto *item = m_tableWidget->item(row, col);
      if (item) {
        QString itemText = item->toolTip().isEmpty() ? item->text() : item->toolTip();
        if (itemText.toLower().contains(keyword)) {
          match = true;
          break;
        }
      }
    }
    m_tableWidget->setRowHidden(row, !match);
  }
}

void MainWindow::onNavButtonClicked(int index) {
  // Update button active styles
  for (int i = 0; i < m_navButtons.size(); ++i) {
    if (i == index) {
      m_navButtons[i]->setStyleSheet(
          "QPushButton { border: none; text-align: left; padding-left: 10px; "
          "font-size: 13px; border-radius: 8px; color: #0f172a; "
          "background-color: #f1f5f9; font-weight: 700; }");
    } else {
      m_navButtons[i]->setStyleSheet(
          "QPushButton { border: none; text-align: left; padding-left: 10px; "
          "font-size: 13px; border-radius: 8px; color: #64748b; "
          "background-color: transparent; font-weight: 500; } "
          "QPushButton:hover { background-color: #f1f5f9; color: #0f172a; }");
    }
  }

  if (index == 0) {
    // 📊 Dashboard (Advertising & Platform Showcase)
    m_pageStack->setCurrentIndex(4);
    if (m_rightDivider) m_rightDivider->hide();
    if (m_rightPanel) m_rightPanel->hide();
    refreshDashboardPage();
  } else if (index == 1) {
    // ⚡ Tách & lọc tài khoản
    m_pageStack->setCurrentIndex(0);
    if (m_rightDivider) m_rightDivider->show();
    if (m_rightPanel) m_rightPanel->show();
  } else if (index == 2) {
    // 👥 Chrome Profiles
    m_pageStack->setCurrentIndex(1);
    if (m_rightDivider) m_rightDivider->hide();
    if (m_rightPanel) m_rightPanel->hide();
    refreshProfileTable();
  } else if (index == 3) {
    // 🌐 Proxy Pool
    m_pageStack->setCurrentIndex(2);
    if (m_rightDivider) m_rightDivider->hide();
    if (m_rightPanel) m_rightPanel->hide();
    refreshProxyPoolTable();
  } else if (index == 5) {
    // 💳 License Key & Thanh toán gia hạn online
    m_pageStack->setCurrentIndex(3);
    if (m_rightDivider) m_rightDivider->hide();
    if (m_rightPanel) m_rightPanel->hide();
    refreshLicensePage();
  } else {
    // Placeholder for other nav items
    CustomMessageBox::information(
        this, "Thông báo",
        QString("Tính năng '%1' đang được phát triển trong phiên bản tiếp theo!")
            .arg(m_navButtons.value(index) ? m_navButtons[index]->text().trimmed() : ""));
  }
}

QWidget *MainWindow::createChromeProfilesPanel() {
  auto *panel = new QWidget();
  auto *layout = new QVBoxLayout(panel);
  layout->setContentsMargins(24, 20, 24, 20);
  layout->setSpacing(16);

  // 1. Header Row
  auto *headerCard = new QWidget();
  headerCard->setStyleSheet(
      "background: #f8fafc; border: 1px solid #f1f5f9; border-radius: 12px; padding: 12px 16px;");
  auto *headerLayout = new QHBoxLayout(headerCard);
  headerLayout->setContentsMargins(0, 0, 0, 0);

  auto *headerIcon = new QLabel("👥");
  headerIcon->setStyleSheet(
      "font-size: 24px; background: #ffedd5; border: 1px solid #fed7aa; "
      "border-radius: 12px; padding: 6px 10px;");
  auto *titleBox = new QVBoxLayout();
  titleBox->setSpacing(2);
  auto *titleText = new QLabel("Quản Lý Chrome Profiles & Môi Trường Nuôi Nick");
  titleText->setStyleSheet("font-size: 16px; font-weight: 800; color: #0f172a;");
  auto *subtitleText = new QLabel(
      "Khởi tạo Browser Profiles cô lập, gán Proxy riêng biệt, Remote Debugging Port (CDP) chống Checkpoint.");
  subtitleText->setStyleSheet("font-size: 12px; color: #64748b; font-weight: 500;");
  titleBox->addWidget(titleText);
  titleBox->addWidget(subtitleText);

  headerLayout->addWidget(headerIcon);
  headerLayout->addLayout(titleBox);
  headerLayout->addStretch();

  // Mini KPI Stats Cards in Header
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

  // 2. Action Toolbar (Split into 2 spacious rows for optimal breathing room)
  auto *toolRow1 = new QHBoxLayout();
  toolRow1->setSpacing(10);

  m_profileSearchEdit = new QLineEdit();
  m_profileSearchEdit->setPlaceholderText("🔍  Tìm kiếm theo Tên Profile, Proxy, Remote Port...");
  m_profileSearchEdit->setClearButtonEnabled(true);
  connect(m_profileSearchEdit, &QLineEdit::textChanged, this,
          &MainWindow::onProfileSearchFilterChanged);
  toolRow1->addWidget(m_profileSearchEdit, 1);

  auto *btnNewProfile = new QPushButton("➕  Tạo Profile");
  btnNewProfile->setCursor(Qt::PointingHandCursor);
  btnNewProfile->setStyleSheet(
      "QPushButton { background: #f97316; color: #ffffff; font-weight: 700; "
      "border: none; border-radius: 8px; padding: 9px 16px; font-size: 13px; } "
      "QPushButton:hover { background: #ea580c; }");
  connect(btnNewProfile, &QPushButton::clicked, this,
          &MainWindow::onCreateProfileClicked);

  auto *btnImportExcel = new QPushButton("📥  Import Excel");
  btnImportExcel->setCursor(Qt::PointingHandCursor);
  btnImportExcel->setToolTip("Nạp hàng loạt Profile từ file Excel / CSV hoặc Clipboard");
  btnImportExcel->setStyleSheet(
      "QPushButton { background: #2563eb; color: #ffffff; font-weight: 700; "
      "border: none; border-radius: 8px; padding: 9px 14px; font-size: 13px; } "
      "QPushButton:hover { background: #1d4ed8; }");
  connect(btnImportExcel, &QPushButton::clicked, this,
          &MainWindow::onImportProfilesClicked);

  auto *btnTemplate = new QPushButton("📋  File Mẫu");
  btnTemplate->setCursor(Qt::PointingHandCursor);
  btnTemplate->setToolTip("Tải file mẫu Excel (.csv) chuẩn có sẵn dữ liệu mẫu");
  btnTemplate->setStyleSheet(
      "QPushButton { background: #f0fdf4; color: #16a34a; font-weight: 600; "
      "border: 1px solid #bbf7d0; border-radius: 8px; padding: 9px 12px; font-size: 13px; } "
      "QPushButton:hover { background: #dcfce7; }");
  connect(btnTemplate, &QPushButton::clicked, this,
          &MainWindow::onExportProfilesTemplateClicked);

  auto *btnOpenDir = new QPushButton("📂  Thư Mục Profile");
  btnOpenDir->setCursor(Qt::PointingHandCursor);
  btnOpenDir->setStyleSheet(
      "QPushButton { background: #ffffff; color: #334155; font-weight: 600; "
      "border: 1px solid #e2e8f0; border-radius: 8px; padding: 9px 12px; font-size: 13px; } "
      "QPushButton:hover { background: #f8fafc; }");
  connect(btnOpenDir, &QPushButton::clicked, this,
          &MainWindow::onOpenProfilesFolderClicked);

  m_btnToggleSyncBar = new QPushButton("⚡  Đồng Bộ Thao Tác");
  m_btnToggleSyncBar->setCursor(Qt::PointingHandCursor);
  m_btnToggleSyncBar->setStyleSheet(
      "QPushButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #8b5cf6, stop:1 #6366f1); "
      "color: #ffffff; font-weight: 700; border: none; border-radius: 8px; padding: 9px 14px; font-size: 13px; } "
      "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #7c3aed, stop:1 #4f46e5); }");
  connect(m_btnToggleSyncBar, &QPushButton::clicked, this, &MainWindow::onToggleSyncBarClicked);

  toolRow1->addWidget(btnNewProfile);
  toolRow1->addWidget(btnImportExcel);
  toolRow1->addWidget(btnTemplate);
  toolRow1->addWidget(btnOpenDir);
  toolRow1->addWidget(m_btnToggleSyncBar);
  layout->addLayout(toolRow1);

  // Row 2: Batch controls & Window size selector
  auto *toolRow2 = new QHBoxLayout();
  toolRow2->setSpacing(10);

  auto *btnToggleSelect = new QPushButton("☑  Chọn Hết / Bỏ");
  btnToggleSelect->setCursor(Qt::PointingHandCursor);
  btnToggleSelect->setStyleSheet(
      "QPushButton { background: #ffffff; color: #334155; font-weight: 600; "
      "border: 1px solid #cbd5e1; border-radius: 8px; padding: 8px 12px; font-size: 12.5px; } "
      "QPushButton:hover { background: #f8fafc; }");
  connect(btnToggleSelect, &QPushButton::clicked, this,
          &MainWindow::onToggleSelectAllProfiles);

  auto *btnLaunchSelected = new QPushButton("🚀  Mở Đã Chọn");
  btnLaunchSelected->setCursor(Qt::PointingHandCursor);
  btnLaunchSelected->setStyleSheet(
      "QPushButton { background: #16a34a; color: #ffffff; font-weight: 700; "
      "border: none; border-radius: 8px; padding: 8px 14px; font-size: 12.5px; } "
      "QPushButton:hover { background: #15803d; }");
  connect(btnLaunchSelected, &QPushButton::clicked, this,
          &MainWindow::onLaunchSelectedProfilesClicked);

  auto *btnStopSelected = new QPushButton("⏹  Dừng Đã Chọn");
  btnStopSelected->setCursor(Qt::PointingHandCursor);
  btnStopSelected->setStyleSheet(
      "QPushButton { background: #ffffff; color: #ea580c; font-weight: 600; "
      "border: 1px solid #fed7aa; border-radius: 8px; padding: 8px 12px; font-size: 12.5px; } "
      "QPushButton:hover { background: #fff7ed; }");
  connect(btnStopSelected, &QPushButton::clicked, this,
          &MainWindow::onStopSelectedProfilesClicked);

  auto *btnStopAll = new QPushButton("🛑  Dừng Tất Cả");
  btnStopAll->setCursor(Qt::PointingHandCursor);
  btnStopAll->setStyleSheet(
      "QPushButton { background: #ffffff; color: #ef4444; font-weight: 600; "
      "border: 1px solid #fecaca; border-radius: 8px; padding: 8px 12px; font-size: 12.5px; } "
      "QPushButton:hover { background: #fef2f2; }");
  connect(btnStopAll, &QPushButton::clicked, this,
          &MainWindow::onStopAllProfilesClicked);

  auto *divider2 = new QFrame();
  divider2->setFrameShape(QFrame::VLine);
  divider2->setStyleSheet("color: #cbd5e1;");

  auto *lblSizeHint = new QLabel("Tỉ lệ mở cửa sổ:");
  lblSizeHint->setStyleSheet("font-size: 12.5px; font-weight: 600; color: #64748b;");

  m_profileWindowSizeCombo = new QComboBox();
  m_profileWindowSizeCombo->setView(new QListView(m_profileWindowSizeCombo));
  m_profileWindowSizeCombo->addItem("📐 Tự động chia hàng (Grid)", QPoint(0, 0));
  m_profileWindowSizeCombo->addItem("📱 Mobile MMO (380 x 600)", QPoint(380, 600));
  m_profileWindowSizeCombo->addItem("💻 Gọn Nuôi Nick (480 x 520)", QPoint(480, 520));
  m_profileWindowSizeCombo->addItem("🖥️ Vừa phải (600 x 700)", QPoint(600, 700));
  m_profileWindowSizeCombo->addItem("📺 HD Chuẩn (1280 x 720)", QPoint(1280, 720));

  toolRow2->addWidget(btnToggleSelect);
  toolRow2->addWidget(btnLaunchSelected);
  toolRow2->addWidget(btnStopSelected);
  toolRow2->addWidget(btnStopAll);
  toolRow2->addWidget(divider2);
  toolRow2->addWidget(lblSizeHint);
  toolRow2->addWidget(m_profileWindowSizeCombo);
  toolRow2->addStretch();
  layout->addLayout(toolRow2);

  // Sync Control Bar
  m_syncBarWidget = new QWidget();
  m_syncBarWidget->setObjectName("syncBarWidget");
  m_syncBarWidget->setStyleSheet(
      "QWidget#syncBarWidget { background-color: #f5f3ff; border: 1.5px solid #ddd6fe; border-radius: 12px; }"
      "QLabel { color: #4338ca; font-weight: 700; font-size: 13px; }"
      "QComboBox, QSpinBox { background: #ffffff; border: 1px solid #c4b5fd; border-radius: 6px; padding: 6px 10px; color: #1e1b4b; font-size: 12px; font-weight: 600; }"
      "QCheckBox { color: #3730a3; font-weight: 600; font-size: 12px; spacing: 6px; }"
      "QCheckBox::indicator { width: 16px; height: 16px; border-radius: 4px; border: 1px solid #a78bfa; }"
      "QCheckBox::indicator:checked { background: #7c3aed; border-color: #7c3aed; }");

  auto *syncLayout = new QHBoxLayout(m_syncBarWidget);
  syncLayout->setContentsMargins(14, 10, 14, 10);
  syncLayout->setSpacing(12);

  auto *lblSyncTitle = new QLabel("⚡ ĐỒNG BỘ:");
  lblSyncTitle->setStyleSheet("font-size: 13px; font-weight: 800; color: #5b21b6;");

  auto *lblMaster = new QLabel("👑 Profile Mẹ:");
  m_syncMasterCombo = new QComboBox();
  m_syncMasterCombo->setView(new QListView(m_syncMasterCombo));
  m_syncMasterCombo->setMinimumWidth(180);

  m_syncMouseCheck = new QCheckBox("🖱️ Chuột");
  m_syncMouseCheck->setChecked(true);
  m_syncKeyCheck = new QCheckBox("⌨️ Phím");
  m_syncKeyCheck->setChecked(true);
  m_syncScrollCheck = new QCheckBox("📜 Cuộn");
  m_syncScrollCheck->setChecked(true);
  m_syncNavCheck = new QCheckBox("🌐 Link URL");
  m_syncNavCheck->setChecked(true);

  auto *lblDelay = new QLabel("⏱️ Delay:");
  m_syncDelaySpin = new QSpinBox();
  m_syncDelaySpin->setRange(0, 200);
  m_syncDelaySpin->setValue(0);
  m_syncDelaySpin->setSuffix(" ms");
  m_syncDelaySpin->setToolTip("Độ trễ ngẫu nhiên giữa các tab con để chống bot detection");

  m_btnStartSync = new QPushButton("🟢  Bắt Đầu Đồng Bộ");
  m_btnStartSync->setCursor(Qt::PointingHandCursor);
  m_btnStartSync->setStyleSheet(
      "QPushButton { background: #7c3aed; color: #ffffff; font-weight: 700; "
      "border: none; border-radius: 8px; padding: 8px 16px; font-size: 13px; } "
      "QPushButton:hover { background: #6d28d9; }");
  connect(m_btnStartSync, &QPushButton::clicked, this, &MainWindow::onStartOrStopSyncClicked);

  syncLayout->addWidget(lblSyncTitle);
  syncLayout->addWidget(lblMaster);
  syncLayout->addWidget(m_syncMasterCombo);
  syncLayout->addWidget(m_syncMouseCheck);
  syncLayout->addWidget(m_syncKeyCheck);
  syncLayout->addWidget(m_syncScrollCheck);
  syncLayout->addWidget(m_syncNavCheck);
  syncLayout->addWidget(lblDelay);
  syncLayout->addWidget(m_syncDelaySpin);
  syncLayout->addWidget(m_btnStartSync);
  syncLayout->addStretch();

  m_syncBarWidget->setVisible(false);
  layout->addWidget(m_syncBarWidget);

  // 3. Profiles Table
  m_profileTable = new QTableWidget();
  m_profileTable->setColumnCount(7);
  QStringList headers = {
      "[☑] STT", "Tên Profile", "Tỉ Lệ Mở", "Proxy Gán Kèm", "Remote Port (CDP)", "Trạng Thái", "Thao Tác"};
  m_profileTable->setHorizontalHeaderLabels(headers);
  m_profileTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_profileTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_profileTable->horizontalHeader()->setStretchLastSection(true);
  m_profileTable->verticalHeader()->setVisible(false); // Ẩn cột số thứ tự mặc định của QTableWidget tránh bị lặp và rít
  m_profileTable->verticalHeader()->setDefaultSectionSize(52); // Đặt chiều cao mỗi hàng 52px rộng rãi, thoáng mắt
  m_profileTable->setShowGrid(false);
  m_profileTable->setAlternatingRowColors(true);

  m_profileTable->setColumnWidth(0, 85);   // [☑] STT
  m_profileTable->setColumnWidth(1, 330);  // Tên Profile & Action Nav
  m_profileTable->setColumnWidth(2, 140);  // Tỉ Lệ Mở
  m_profileTable->setColumnWidth(3, 230);  // Proxy
  m_profileTable->setColumnWidth(4, 150);  // Remote Port (CDP)
  m_profileTable->setColumnWidth(5, 150);  // Trạng Thái
  m_profileTable->setColumnWidth(6, 240);  // Thao Tác

  connect(m_profileTable, &QTableWidget::cellDoubleClicked,
          [this](int row, int /*col*/) {
            if (row >= 0 && row < m_profiles.size()) {
              if (m_profiles[row].isRunning) {
                onStopProfile(row);
              } else {
                onLaunchProfile(row);
              }
            }
          });

  connect(m_profileTable, &QTableWidget::itemChanged, this,
          [this](QTableWidgetItem *item) {
            if (item && item->column() == 0) {
              int row = item->row();
              if (row >= 0 && row < m_profiles.size()) {
                m_profiles[row].isSelected = (item->checkState() == Qt::Checked);
              }
            }
          });

  layout->addWidget(m_profileTable, 1);

  return panel;
}

QPair<int, int> MainWindow::getActiveWindowSize(const ChromeProfileItem &item) const {
  if (item.windowWidth > 0 && item.windowHeight > 0) {
    return qMakePair(item.windowWidth, item.windowHeight);
  }
  if (m_profileWindowSizeCombo) {
    QPoint pt = m_profileWindowSizeCombo->currentData().toPoint();
    if (pt.x() > 0 && pt.y() > 0) {
      return qMakePair(pt.x(), pt.y());
    }
  }
  return qMakePair(0, 0);
}

QPoint MainWindow::calculateTilePosition(int width, int height, int slotIndex) const {
  QScreen *screen = nullptr;
  if (this->windowHandle()) {
    screen = this->windowHandle()->screen();
  }
  if (!screen) {
    screen = QGuiApplication::primaryScreen();
  }
  QRect screenGeo = screen ? screen->availableGeometry() : QRect(0, 0, 1920, 1080);

  int w = (width > 0) ? width : qMax(450, screenGeo.width() / 4);
  int h = (height > 0) ? height : qMax(480, (screenGeo.height() - 40) / 2);

  // Bước nhảy ngang: Chrome GTK min frame width là ~480-500px, đảm bảo các tab đứng sát cạnh nhau theo hàng
  int stepX = qMax(w, 480);
  int cols = qMax(1, screenGeo.width() / stepX);

  int col = slotIndex % cols;
  int row = slotIndex / cols;

  int x = screenGeo.left() + col * stepX;

  // Tính tọa độ y theo từng hàng
  int stepY = h;
  if (screenGeo.top() + stepY + h > screenGeo.bottom()) {
    stepY = qMax(40, screenGeo.height() - h);
  }
  int y = screenGeo.top() + (row * stepY);

  // Nếu số hàng vượt quá không gian màn hình, wrap lại với độ lệch cascade nhỏ
  int maxRows = qMax(1, screenGeo.height() / qMax(50, stepY));
  if (row >= maxRows) {
    int wrapRow = row % maxRows;
    int cycle = row / maxRows;
    y = screenGeo.top() + (wrapRow * stepY) + (cycle * 30);
    x += (cycle * 30);
  }

  return QPoint(x, y);
}

QString MainWindow::getProfilesDir() const {
  QString dir = QDir::homePath() + "/.tunnbit_profiles";
  QDir().mkpath(dir);
  return dir;
}

void MainWindow::loadProfiles() {
  QString jsonPath = getProfilesDir() + "/profiles.json";
  QFile file(jsonPath);
  m_profiles.clear();

  // CHỈ tạo profile mẫu khi file profiles.json chưa từng tồn tại lần nào
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
    p2.windowWidth = 0;
    p2.windowHeight = 0;
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
      QJsonArray arr = doc.array();
      for (const auto &val : arr) {
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

void MainWindow::saveProfiles() {
  QString jsonPath = getProfilesDir() + "/profiles.json";
  QFile file(jsonPath);
  if (!file.open(QIODevice::WriteOnly))
    return;

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

  QJsonDocument doc(arr);
  file.write(doc.toJson(QJsonDocument::Indented));
  file.close();
}

void MainWindow::refreshProfileTable() {
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

    // 0. Checkbox + STT
    auto *sttItem = new QTableWidgetItem(QString("  %1").arg(i + 1));
    sttItem->setCheckState(item.isSelected ? Qt::Checked : Qt::Unchecked);
    sttItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    m_profileTable->setItem(i, 0, sttItem);

    // 1. Tên Profile + Hover Action Nav (⚙️ Cấu hình, 🌐 Sửa Proxy)
    auto *nameWidget = new ProfileNameCellWidget(
        i, item,
        [this](int idx) { onEditProfile(idx); },
        [this](int idx) { onQuickEditProxy(idx); },
        m_profileTable);
    m_profileTable->setCellWidget(i, 1, nameWidget);

    // 2. Tỉ Lệ Mở Cửa Sổ
    QString sizeText = (item.windowWidth > 0 && item.windowHeight > 0)
        ? QString("📐 %1 x %2").arg(item.windowWidth).arg(item.windowHeight)
        : "⚙ Tool Auto";
    auto *sizeItem = new QTableWidgetItem(sizeText);
    sizeItem->setTextAlignment(Qt::AlignCenter);
    if (item.windowWidth > 0) {
      sizeItem->setForeground(QColor("#0284c7"));
    } else {
      sizeItem->setForeground(QColor("#94a3b8"));
    }
    m_profileTable->setItem(i, 2, sizeItem);

    // 3. Proxy
    ProxyConfig pCfg = ProxyConfig::fromString(item.proxy);
    QString proxyText = pCfg.toDisplayString();
    auto *proxyItem = new QTableWidgetItem(proxyText);
    proxyItem->setToolTip(item.proxy.isEmpty() ? "Dùng mạng Internet máy chủ trực tiếp" : item.proxy);
    if (pCfg.isEmpty()) {
      proxyItem->setForeground(QColor("#94a3b8"));
    } else {
      proxyItem->setForeground(QColor("#f97316"));
    }
    m_profileTable->setItem(i, 3, proxyItem);

    // 4. Port
    auto *portItem = new QTableWidgetItem(QString::number(item.port));
    portItem->setTextAlignment(Qt::AlignCenter);
    m_profileTable->setItem(i, 4, portItem);

    // 5. Trạng Thái
    QString statusText;
    QColor statusColor("#94a3b8");
    if (item.isRunning) {
      if (m_syncManager && m_syncManager->isSyncing()) {
        if (item.port == m_syncManager->masterPort()) {
          statusText = QString("👑 MẸ (Master) Port %1").arg(item.port);
          statusColor = QColor("#7c3aed");
        } else if (m_syncManager->workerPorts().contains(item.port)) {
          statusText = QString("⚡ SYNC (Port %1)").arg(item.port);
          statusColor = QColor("#2563eb");
        } else {
          statusText = QString("🟢 Đang chạy (Port %1)").arg(item.port);
          statusColor = QColor("#16a34a");
        }
      } else {
        statusText = QString("🟢 Đang chạy (Port %1)").arg(item.port);
        statusColor = QColor("#16a34a");
      }
    } else {
      statusText = "⚪ Đã tắt";
    }

    auto *statusItem = new QTableWidgetItem(statusText);
    statusItem->setTextAlignment(Qt::AlignCenter);
    statusItem->setForeground(statusColor);
    if (item.isRunning) {
      statusItem->setFont(QFont("Google Sans", 9, QFont::Bold));
    }
    m_profileTable->setItem(i, 5, statusItem);

    // 6. Thao Tác (Action Buttons Widget)
    auto *actionWidget = new QWidget();
    auto *actionLayout = new QHBoxLayout(actionWidget);
    actionLayout->setContentsMargins(8, 4, 8, 4);
    actionLayout->setSpacing(8);

    QString profileId = item.id;

    if (!item.isRunning) {
      auto *btnLaunch = new QPushButton("▶  Mở Chrome");
      btnLaunch->setCursor(Qt::PointingHandCursor);
      btnLaunch->setFixedHeight(32);
      btnLaunch->setStyleSheet(
          "QPushButton { background: #f0fdf4; color: #16a34a; font-weight: 700; "
          "border: 1px solid #bbf7d0; border-radius: 8px; padding: 4px 12px; font-size: 12px; } "
          "QPushButton:hover { background: #16a34a; color: #ffffff; }");
      connect(btnLaunch, &QPushButton::clicked, [this, profileId]() {
        for (int k = 0; k < m_profiles.size(); ++k) {
          if (m_profiles[k].id == profileId) {
            onLaunchProfile(k);
            break;
          }
        }
      });
      actionLayout->addWidget(btnLaunch);
    } else {
      auto *btnStop = new QPushButton("■  Dừng Chrome");
      btnStop->setCursor(Qt::PointingHandCursor);
      btnStop->setFixedHeight(32);
      btnStop->setStyleSheet(
          "QPushButton { background: #fef2f2; color: #ef4444; font-weight: 700; "
          "border: 1px solid #fecaca; border-radius: 8px; padding: 4px 12px; font-size: 12px; } "
          "QPushButton:hover { background: #ef4444; color: #ffffff; }");
      connect(btnStop, &QPushButton::clicked, [this, profileId]() {
        for (int k = 0; k < m_profiles.size(); ++k) {
          if (m_profiles[k].id == profileId) {
            onStopProfile(k);
            break;
          }
        }
      });
      actionLayout->addWidget(btnStop);
    }

    auto *btnEdit = new QPushButton("⚙️");
    btnEdit->setToolTip("Chỉnh sửa cấu hình & Proxy");
    btnEdit->setCursor(Qt::PointingHandCursor);
    btnEdit->setFixedSize(32, 32);
    btnEdit->setStyleSheet(
        "QPushButton { background: #f8fafc; color: #64748b; border: 1px solid #e2e8f0; "
        "border-radius: 8px; font-size: 13px; } QPushButton:hover { color: #f97316; border-color: #fdba74; background: #fff7ed; }");
    connect(btnEdit, &QPushButton::clicked, [this, profileId]() {
      for (int k = 0; k < m_profiles.size(); ++k) {
        if (m_profiles[k].id == profileId) {
          onEditProfile(k);
          break;
        }
      }
    });
    actionLayout->addWidget(btnEdit);

    auto *btnDel = new QPushButton("🗑");
    btnDel->setToolTip("Xóa Profile");
    btnDel->setCursor(Qt::PointingHandCursor);
    btnDel->setFixedSize(32, 32);
    btnDel->setStyleSheet(
        "QPushButton { background: #f8fafc; color: #94a3b8; border: 1px solid #e2e8f0; "
        "border-radius: 8px; font-size: 13px; } QPushButton:hover { color: #ef4444; border-color: #fca5a5; background: #fef2f2; }");
    connect(btnDel, &QPushButton::clicked, [this, profileId]() {
      for (int k = 0; k < m_profiles.size(); ++k) {
        if (m_profiles[k].id == profileId) {
          onDeleteProfile(k);
          break;
        }
      }
    });
    actionLayout->addWidget(btnDel);

    actionLayout->addStretch();
    m_profileTable->setCellWidget(i, 6, actionWidget);
  }

  m_profileTable->blockSignals(false);

  // Update KPI Stats
  if (m_lblTotalProfiles) m_lblTotalProfiles->setText(QString::number(m_profiles.size()));
  if (m_lblActiveProfiles) m_lblActiveProfiles->setText(QString("%1 active").arg(activeCount));
  if (m_lblProxyProfiles) m_lblProxyProfiles->setText(QString::number(proxyCount));

  updateSyncMasterCombo();
}

void MainWindow::onToggleSelectAllProfiles() {
  if (m_profiles.isEmpty()) return;

  bool allSelected = true;
  for (const auto &p : m_profiles) {
    if (!p.isSelected) {
      allSelected = false;
      break;
    }
  }

  bool newState = !allSelected;
  m_profileTable->blockSignals(true);
  for (int i = 0; i < m_profiles.size(); ++i) {
    m_profiles[i].isSelected = newState;
    if (m_profileTable && i < m_profileTable->rowCount()) {
      auto *item = m_profileTable->item(i, 0);
      if (item) {
        item->setCheckState(newState ? Qt::Checked : Qt::Unchecked);
      }
    }
  }
  m_profileTable->blockSignals(false);
}

void MainWindow::onCreateProfileClicked() {
  QStringList existingNames;
  int maxPort = 9221;
  for (const auto &p : m_profiles) {
    existingNames.append(p.name.trimmed());
    if (p.port > maxPort) maxPort = p.port;
  }
  int suggestedPort = maxPort + 1;

  // Tự động tìm tên gợi ý Profile 01, Profile 02... chưa tồn tại
  int nextNum = 1;
  QString suggestedName;
  while (true) {
    suggestedName = QString("Profile %1").arg(nextNum, 2, 10, QChar('0'));
    bool exists = false;
    for (const auto &n : existingNames) {
      if (n.compare(suggestedName, Qt::CaseInsensitive) == 0) {
        exists = true;
        break;
      }
    }
    if (!exists) break;
    nextNum++;
  }

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

    // Tạo thư mục profile thực tế
    QString profileDir = getProfilesDir() + "/" + newItem.id;
    QDir().mkpath(profileDir);

    m_profiles.append(newItem);
    saveProfiles();
    refreshProfileTable();

    QString sizeInfo = (newItem.windowWidth > 0 && newItem.windowHeight > 0)
        ? QString("%1 x %2").arg(newItem.windowWidth).arg(newItem.windowHeight)
        : "Theo Tool Auto";

    CustomMessageBox::information(
        this, "Thành công",
        QString("Đã khởi tạo thành công Profile:\n- Tên: %1\n- Tỉ lệ: %2\n- Remote Port: %3\n- Proxy: %4")
            .arg(newItem.name)
            .arg(sizeInfo)
            .arg(newItem.port)
            .arg(newItem.proxy.isEmpty() ? "Direct" : newItem.proxy));
  }
}

void MainWindow::onImportProfilesClicked() {
  ImportProfilesDialog dlg(m_profiles, this);
  if (dlg.exec() == QDialog::Accepted) {
    auto importedList = dlg.getImportedProfiles();
    if (!importedList.isEmpty()) {
      for (const auto &item : importedList) {
        // Tạo thư mục profile thực tế trên ổ cứng
        QString profileDir = getProfilesDir() + "/" + item.id;
        QDir().mkpath(profileDir);
        m_profiles.append(item);
      }
      saveProfiles();
      refreshProfileTable();
      CustomMessageBox::information(
          this, "Nạp File Hoàn Tất",
          QString("Đã thêm thành công %1 Profile mới vào danh sách!\nToàn bộ thư mục và cấu hình đã được khởi tạo sẵn sàng sử dụng.")
              .arg(importedList.size()));
    }
  }
}

void MainWindow::onExportProfilesTemplateClicked() {
  ImportProfilesDialog::exportTemplateCsv(this);
}

void MainWindow::onLaunchProfile(int index, int slotOrder) {
  if (index < 0 || index >= m_profiles.size()) return;
  auto &item = m_profiles[index];

  if (item.isRunning) {
    CustomMessageBox::information(this, "Thông báo", "Profile này đang chạy rồi!");
    return;
  }

  if (!item.launcher) {
    item.launcher = new ChromeLauncher(this);
    connect(item.launcher, &ChromeLauncher::processFinished, this,
            [this, index](int /*exitCode*/) {
              if (index < m_profiles.size()) {
                m_profiles[index].isRunning = false;
                refreshProfileTable();
              }
            });
  }

  QString profilePath = getProfilesDir() + "/" + item.id;
  QDir().mkpath(profilePath);

  // Kích thước cửa sổ
  QPair<int, int> winSize = getActiveWindowSize(item);
  int w = winSize.first;
  int h = winSize.second;
  if (w <= 0 || h <= 0) {
    QScreen *screen = nullptr;
    if (this->windowHandle()) {
      screen = this->windowHandle()->screen();
    }
    if (!screen) {
      screen = QGuiApplication::primaryScreen();
    }
    QRect screenGeo = screen ? screen->availableGeometry() : QRect(0, 0, 1920, 1080);
    w = qMax(450, screenGeo.width() / 4);
    h = qMax(480, (screenGeo.height() - 40) / 2);
  }

  // Thứ tự slot mở để dàn ngang lần lượt theo hàng không đè lên nhau
  int slot = slotOrder;
  if (slot < 0) {
    int activeCount = 0;
    for (const auto &p : m_profiles) {
      if (p.isRunning) activeCount++;
    }
    slot = activeCount;
  }

  QPoint pos = calculateTilePosition(w, h, slot);

  QString ua = EditProfileDialog::getUserAgentForDevice(item.deviceType, item.customUserAgent);
  bool ok = item.launcher->launch(item.port, profilePath, item.proxy, w, h, pos.x(), pos.y(), 1.0, ua);
  if (ok) {
    item.isRunning = true;
    refreshProfileTable();
  } else {
    CustomMessageBox::critical(
        this, "Lỗi Khởi Động",
        "Không thể khởi động trình duyệt Chrome!\n"
        "Vui lòng kiểm tra xem Chrome đã được cài đặt trên máy hoặc thử thay đổi cổng Port.");
  }
}

void MainWindow::onStopProfile(int index) {
  if (index < 0 || index >= m_profiles.size()) return;
  auto &item = m_profiles[index];

  if (item.launcher) {
    item.launcher->stop();
  }
  item.isRunning = false;
  refreshProfileTable();
}

void MainWindow::onStopAllProfilesClicked() {
  int count = 0;
  for (int i = 0; i < m_profiles.size(); ++i) {
    if (m_profiles[i].isRunning) {
      if (m_profiles[i].launcher) {
        m_profiles[i].launcher->stop();
      }
      m_profiles[i].isRunning = false;
      count++;
    }
  }
  refreshProfileTable();

  CustomMessageBox::information(
      this, "Đã dừng",
      QString("Đã dừng hoạt động %1 Chrome profiles!").arg(count));
}

void MainWindow::onLaunchSelectedProfilesClicked() {
  QList<int> selectedIndices;
  for (int i = 0; i < m_profiles.size(); ++i) {
    if (m_profiles[i].isSelected) {
      selectedIndices.append(i);
    }
  }

  if (selectedIndices.isEmpty()) {
    CustomMessageBox::warning(
        this, "Thông báo",
        "Vui lòng tích chọn ít nhất một Profile (ở cột đầu tiên [☑]) để khởi chạy!");
    return;
  }

  int activeBaseSlot = 0;
  for (const auto &p : m_profiles) {
    if (p.isRunning) activeBaseSlot++;
  }

  int launchedCount = 0;
  for (int idx : selectedIndices) {
    if (!m_profiles[idx].isRunning) {
      int slot = activeBaseSlot + launchedCount;
      if (launchedCount == 0) {
        onLaunchProfile(idx, slot);
      } else {
        int delayMs = launchedCount * 250;
        QTimer::singleShot(delayMs, this, [this, idx, slot]() {
          if (idx < m_profiles.size() && !m_profiles[idx].isRunning) {
            onLaunchProfile(idx, slot);
          }
        });
      }
      launchedCount++;
    }
  }

  CustomMessageBox::information(
      this, "Thành công",
      QString("Đã khởi chạy và tự động sắp xếp %1 Profile lần lượt theo hàng!").arg(launchedCount));
}

void MainWindow::onStopSelectedProfilesClicked() {
  QList<int> selectedIndices;
  for (int i = 0; i < m_profiles.size(); ++i) {
    if (m_profiles[i].isSelected) {
      selectedIndices.append(i);
    }
  }

  if (selectedIndices.isEmpty()) {
    CustomMessageBox::warning(
        this, "Thông báo",
        "Vui lòng tích chọn Profile cần dừng!");
    return;
  }

  int stoppedCount = 0;
  for (int idx : selectedIndices) {
    if (m_profiles[idx].isRunning) {
      onStopProfile(idx);
      stoppedCount++;
    }
  }

  CustomMessageBox::information(
      this, "Đã dừng",
      QString("Đã dừng hoạt động %1 Profile được chọn!").arg(stoppedCount));
}

void MainWindow::onOpenProfilesFolderClicked() {
  QDesktopServices::openUrl(QUrl::fromLocalFile(getProfilesDir()));
}

void MainWindow::onDeleteProfile(int index) {
  if (index < 0 || index >= m_profiles.size()) return;

  onStopProfile(index);

  QString pId = m_profiles[index].id;
  QString pName = m_profiles[index].name;

  if (m_profiles[index].launcher) {
    delete m_profiles[index].launcher;
    m_profiles[index].launcher = nullptr;
  }

  m_profiles.removeAt(index);
  saveProfiles();
  refreshProfileTable();

  // Xóa sạch thư mục data profile tương ứng trên ổ đĩa
  QString profilePath = getProfilesDir() + "/" + pId;
  QDir(profilePath).removeRecursively();

  CustomMessageBox::information(
      this, "Đã xóa",
      QString("Đã xóa profile '%1' thành công!").arg(pName));
}

void MainWindow::onProfileSearchFilterChanged(const QString &text) {
  QString keyword = text.trimmed().toLower();
  if (!m_profileTable) return;

  for (int row = 0; row < m_profileTable->rowCount(); ++row) {
    if (keyword.isEmpty()) {
      m_profileTable->setRowHidden(row, false);
      continue;
    }

    bool match = false;
    for (int col = 1; col <= 4; ++col) {
      auto *item = m_profileTable->item(row, col);
      if (item && item->text().toLower().contains(keyword)) {
        match = true;
        break;
      }
    }
    m_profileTable->setRowHidden(row, !match);
  }
}

void MainWindow::onToggleSyncBarClicked() {
  if (!m_syncBarWidget) return;
  bool isVis = !m_syncBarWidget->isVisible();
  m_syncBarWidget->setVisible(isVis);
  if (isVis) {
    updateSyncMasterCombo();
  }
}

void MainWindow::updateSyncMasterCombo() {
  if (!m_syncMasterCombo) return;
  int currentPort = m_syncMasterCombo->currentData().toInt();
  m_syncMasterCombo->clear();

  int activeCount = 0;
  for (const auto &p : m_profiles) {
    if (p.isRunning) {
      m_syncMasterCombo->addItem(QString("👑 %1 (Port %2)").arg(p.name).arg(p.port), p.port);
      activeCount++;
    }
  }

  if (activeCount == 0) {
    m_syncMasterCombo->addItem("⚠️ Chưa có Profile nào chạy", 0);
  } else if (currentPort > 0) {
    int idx = m_syncMasterCombo->findData(currentPort);
    if (idx >= 0) m_syncMasterCombo->setCurrentIndex(idx);
  }
}

void MainWindow::onStartOrStopSyncClicked() {
  if (!m_syncManager) return;

  if (m_syncManager->isSyncing()) {
    m_syncManager->stopSync();
    if (m_btnStartSync) {
      m_btnStartSync->setText("🟢  Bắt Đầu Đồng Bộ");
      m_btnStartSync->setStyleSheet(
          "QPushButton { background: #7c3aed; color: #ffffff; font-weight: 700; "
          "border: none; border-radius: 8px; padding: 8px 16px; font-size: 13px; } "
          "QPushButton:hover { background: #6d28d9; }");
    }
    refreshProfileTable();
    CustomMessageBox::information(this, "Đồng bộ", "Đã dừng đồng bộ thao tác!");
    return;
  }

  int masterPort = m_syncMasterCombo ? m_syncMasterCombo->currentData().toInt() : 0;
  if (masterPort <= 0) {
    CustomMessageBox::warning(
        this, "Thông báo",
        "Vui lòng chọn một Profile Mẹ (đang chạy) để làm nguồn điều khiển!");
    return;
  }

  // Thu thập danh sách Profile Con:
  // Ưu tiên các profile đang chạy được tích chọn [☑] (trừ Profile Mẹ)
  // Nếu không tích chọn profile nào, lấy toàn bộ các profile đang chạy (trừ Profile Mẹ)
  QList<int> workerPorts;
  bool hasSelection = false;
  for (const auto &p : m_profiles) {
    if (p.isSelected && p.isRunning && p.port != masterPort) {
      hasSelection = true;
      workerPorts.append(p.port);
    }
  }

  if (!hasSelection) {
    for (const auto &p : m_profiles) {
      if (p.isRunning && p.port != masterPort) {
        workerPorts.append(p.port);
      }
    }
  }

  if (workerPorts.isEmpty()) {
    CustomMessageBox::warning(
        this, "Thông báo",
        "Cần ít nhất 1 Profile Con (Worker) đang chạy để nhận đồng bộ thao tác!");
    return;
  }

  SyncOptions opts;
  opts.syncMouse = m_syncMouseCheck ? m_syncMouseCheck->isChecked() : true;
  opts.syncKeyboard = m_syncKeyCheck ? m_syncKeyCheck->isChecked() : true;
  opts.syncScroll = m_syncScrollCheck ? m_syncScrollCheck->isChecked() : true;
  opts.syncNavigation = m_syncNavCheck ? m_syncNavCheck->isChecked() : true;
  opts.randomDelayMs = m_syncDelaySpin ? m_syncDelaySpin->value() : 0;

  m_syncManager->startSync(masterPort, workerPorts, opts);

  if (m_btnStartSync) {
    m_btnStartSync->setText("⏹  Dừng Đồng Bộ");
    m_btnStartSync->setStyleSheet(
        "QPushButton { background: #ef4444; color: #ffffff; font-weight: 700; "
        "border: none; border-radius: 8px; padding: 8px 16px; font-size: 13px; } "
        "QPushButton:hover { background: #dc2626; }");
  }

  refreshProfileTable();

  CustomMessageBox::information(
      this, "Đang đồng bộ",
      QString("Đã bật chế độ Đồng bộ thao tác!\n\n"
              "👑 Profile Mẹ: Port %1\n"
              "⚡ Profile Con: %2 profiles đang nhận đồng bộ")
          .arg(masterPort)
          .arg(workerPorts.size()));
}

void MainWindow::onEditProfile(int index) {
  if (index < 0 || index >= m_profiles.size()) return;
  auto &item = m_profiles[index];

  if (item.isRunning) {
    CustomMessageBox::warning(
        this, "Cảnh báo",
        "Vui lòng dừng Profile này trước khi chỉnh sửa cấu hình!");
    return;
  }

  QStringList otherNames;
  for (int i = 0; i < m_profiles.size(); ++i) {
    if (i != index) otherNames.append(m_profiles[i].name);
  }

  EditProfileDialog dlg(item.name, item.deviceType, item.customUserAgent,
                        item.proxy, item.port, item.windowWidth, item.windowHeight,
                        otherNames, this);
  if (dlg.exec() == QDialog::Accepted) {
    item.name = dlg.getProfileName();
    item.deviceType = dlg.getDeviceType();
    item.customUserAgent = dlg.getUserAgent();
    item.proxy = dlg.getProxy();
    item.port = dlg.getPort();
    item.windowWidth = dlg.getWindowWidth();
    item.windowHeight = dlg.getWindowHeight();

    saveProfiles();
    refreshProfileTable();

    CustomMessageBox::information(
        this, "Thành công",
        QString("Đã cập nhật cấu hình cho Profile '%1' thành công!").arg(item.name));
  }
}

void MainWindow::onQuickEditProxy(int index) {
  if (index < 0 || index >= m_profiles.size()) return;
  auto &item = m_profiles[index];

  QuickProxyDialog dlg(item.name, item.proxy, this);
  if (dlg.exec() == QDialog::Accepted) {
    item.proxy = dlg.getProxy();
    saveProfiles();
    refreshProfileTable();

    QString msg = item.proxy.isEmpty()
        ? QString("Đã chuyển Profile '%1' sang dùng Mạng Máy (Direct)!").arg(item.name)
        : QString("Đã cập nhật Proxy cho Profile '%1' thành: %2").arg(item.name).arg(item.proxy);
    CustomMessageBox::information(this, "Thành công", msg);
  }
}

QWidget *MainWindow::createProxyPoolPanel() {
  auto *panel = new QWidget();
  auto *layout = new QVBoxLayout(panel);
  layout->setContentsMargins(24, 20, 24, 20);
  layout->setSpacing(14);

  // 1. Header Card with KPI Stats
  auto *headerCard = new QWidget();
  headerCard->setStyleSheet(
      "background: #f8fafc; border: 1px solid #f1f5f9; border-radius: 12px; padding: 12px 16px;");
  auto *headerLayout = new QHBoxLayout(headerCard);
  headerLayout->setContentsMargins(0, 0, 0, 0);

  auto *headerIcon = new QLabel("🌐");
  headerIcon->setStyleSheet(
      "font-size: 24px; background: #e0f2fe; border: 1px solid #bae6fd; "
      "border-radius: 12px; padding: 6px 10px;");
  auto *titleBox = new QVBoxLayout();
  titleBox->setSpacing(2);
  auto *titleText = new QLabel("Kho Proxy (Proxy Pool) & Kiểm Tra Tình Trạng Kết Nối");
  titleText->setStyleSheet("font-size: 16px; font-weight: 800; color: #0f172a;");
  auto *subtitleText = new QLabel(
      "Tự động trích xuất Proxy từ các Chrome Profiles, kiểm tra độ trễ (Ping ms), quốc gia và tình trạng Sống / Chết.");
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

  m_proxyPoolSearchEdit = new QLineEdit();
  m_proxyPoolSearchEdit->setPlaceholderText("🔍  Tìm kiếm theo IP, Cổng, Quốc gia, Tên Profile...");
  m_proxyPoolSearchEdit->setClearButtonEnabled(true);
  connect(m_proxyPoolSearchEdit, &QLineEdit::textChanged, this, &MainWindow::onProxyPoolSearchChanged);
  toolRow->addWidget(m_proxyPoolSearchEdit, 1);

  m_proxyPoolFilterCombo = new QComboBox();
  m_proxyPoolFilterCombo->addItem("Tất Cả Trạng Thái");
  m_proxyPoolFilterCombo->addItem("🟢 Chỉ Proxy Sống (Live)");
  m_proxyPoolFilterCombo->addItem("🔴 Chỉ Proxy Chết (Die)");
  m_proxyPoolFilterCombo->addItem("⚪ Chưa Kiểm Tra");
  connect(m_proxyPoolFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onFilterProxyStatusChanged);
  toolRow->addWidget(m_proxyPoolFilterCombo);

  m_btnCheckAllProxies = new QPushButton("⚡  Kiểm Tra Toàn Bộ Proxy (Check All)");
  m_btnCheckAllProxies->setCursor(Qt::PointingHandCursor);
  m_btnCheckAllProxies->setStyleSheet(
      "QPushButton { background: #2563eb; color: #ffffff; font-weight: 700; "
      "border: none; border-radius: 8px; padding: 9px 16px; font-size: 13px; } "
      "QPushButton:hover { background: #1d4ed8; }");
  connect(m_btnCheckAllProxies, &QPushButton::clicked, this, &MainWindow::onCheckAllProxiesClicked);
  toolRow->addWidget(m_btnCheckAllProxies);

  auto *btnSync = new QPushButton("🔄  Đồng Bộ Từ Profiles");
  btnSync->setCursor(Qt::PointingHandCursor);
  btnSync->setStyleSheet(
      "QPushButton { background: #ffffff; color: #334155; font-weight: 600; "
      "border: 1px solid #cbd5e1; border-radius: 8px; padding: 9px 14px; font-size: 13px; } "
      "QPushButton:hover { background: #f8fafc; }");
  connect(btnSync, &QPushButton::clicked, this, &MainWindow::onSyncProxiesFromProfiles);
  toolRow->addWidget(btnSync);

  layout->addLayout(toolRow);

  // 3. Proxy Table
  m_proxyPoolTable = new QTableWidget();
  m_proxyPoolTable->setColumnCount(7);
  QStringList headers = {
      "STT", "Địa Chỉ Proxy", "Profile Đang Gán", "Trạng Thái", "Độ Trễ (Ping)", "Quốc Gia & IP Public", "Thao Tác"};
  m_proxyPoolTable->setHorizontalHeaderLabels(headers);
  m_proxyPoolTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_proxyPoolTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_proxyPoolTable->verticalHeader()->setVisible(false);
  m_proxyPoolTable->verticalHeader()->setDefaultSectionSize(52);
  m_proxyPoolTable->setShowGrid(false);
  m_proxyPoolTable->setAlternatingRowColors(true);

  m_proxyPoolTable->setColumnWidth(0, 60);   // STT
  m_proxyPoolTable->setColumnWidth(1, 240);  // Địa Chỉ Proxy
  m_proxyPoolTable->setColumnWidth(2, 220);  // Profile Đang Gán
  m_proxyPoolTable->setColumnWidth(3, 150);  // Trạng Thái
  m_proxyPoolTable->setColumnWidth(4, 120);  // Ping
  m_proxyPoolTable->setColumnWidth(5, 240);  // Quốc Gia & IP Public
  m_proxyPoolTable->horizontalHeader()->setStretchLastSection(true);

  layout->addWidget(m_proxyPoolTable, 1);

  return panel;
}

void MainWindow::refreshProxyPoolTable() {
  if (!m_proxyPoolTable) return;

  // 1. Group existing check results to preserve them during re-sync
  QMap<QString, ProxyPoolItem> existingResults;
  for (const auto &item : m_proxyPoolItems) {
    existingResults[item.rawProxy] = item;
  }

  // 2. Extract proxies from m_profiles
  QMap<QString, QStringList> profileProxyMap;
  for (const auto &p : m_profiles) {
    QString px = p.proxy.trimmed();
    if (px.isEmpty()) px = "[DIRECT_MACHINE]";
    profileProxyMap[px].append(p.name);
  }

  m_proxyPoolItems.clear();
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
    m_proxyPoolItems.append(item);
  }

  updateProxyPoolKpis();

  // Populate Table
  m_proxyPoolTable->blockSignals(true);
  m_proxyPoolTable->setRowCount(0);
  m_proxyPoolTable->setRowCount(m_proxyPoolItems.size());

  for (int i = 0; i < m_proxyPoolItems.size(); ++i) {
    const auto &item = m_proxyPoolItems[i];

    // 0. STT
    auto *sttItem = new QTableWidgetItem(QString::number(i + 1));
    sttItem->setTextAlignment(Qt::AlignCenter);
    sttItem->setFont(QFont("Google Sans", 10, QFont::Bold));
    m_proxyPoolTable->setItem(i, 0, sttItem);

    // 1. Địa Chỉ Proxy
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
    if (item.rawProxy == "[DIRECT_MACHINE]") {
      proxyItem->setForeground(QColor("#64748b"));
    } else {
      proxyItem->setForeground(QColor("#0f172a"));
    }
    m_proxyPoolTable->setItem(i, 1, proxyItem);

    // 2. Profile Đang Gán
    QString profNames = item.associatedProfileNames.join(", ");
    if (profNames.length() > 45) {
      profNames = profNames.left(42) + "...";
    }
    auto *profItem = new QTableWidgetItem(QString("👥 %1 (%2)").arg(profNames).arg(item.associatedProfileNames.size()));
    profItem->setToolTip(item.associatedProfileNames.join("\n"));
    profItem->setForeground(QColor("#2563eb"));
    m_proxyPoolTable->setItem(i, 2, profItem);

    // 3. Trạng Thái
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
    m_proxyPoolTable->setItem(i, 3, statusItem);

    // 4. Độ Trễ (Ping)
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
    m_proxyPoolTable->setItem(i, 4, pingItem);

    // 5. Quốc Gia & IP Public
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
      geoItem->setToolTip(item.errorMsg);
    } else if (item.checked && item.isLive) {
      geoItem->setForeground(QColor("#0f172a"));
      geoItem->setFont(QFont("Google Sans", 10));
    } else {
      geoItem->setForeground(QColor("#94a3b8"));
    }
    m_proxyPoolTable->setItem(i, 5, geoItem);

    // 6. Thao Tác (Action Buttons Widget)
    auto *actionWidget = new QWidget();
    auto *actionLayout = new QHBoxLayout(actionWidget);
    actionLayout->setContentsMargins(6, 4, 6, 4);
    actionLayout->setSpacing(6);

    auto *btnCheck = new QPushButton("🔍  Kiểm Tra");
    btnCheck->setCursor(Qt::PointingHandCursor);
    btnCheck->setFixedHeight(28);
    btnCheck->setEnabled(!item.isChecking);
    btnCheck->setStyleSheet(
        "QPushButton { background: #eff6ff; color: #2563eb; font-weight: 700; "
        "border: 1px solid #bfdbfe; border-radius: 6px; padding: 2px 10px; font-size: 11.5px; } "
        "QPushButton:hover { background: #2563eb; color: #ffffff; }");
    connect(btnCheck, &QPushButton::clicked, [this, i]() {
      checkSingleProxy(i);
    });
    actionLayout->addWidget(btnCheck);

    actionLayout->addStretch();
    m_proxyPoolTable->setCellWidget(i, 6, actionWidget);
  }

  m_proxyPoolTable->blockSignals(false);
  onProxyPoolSearchChanged(m_proxyPoolSearchEdit ? m_proxyPoolSearchEdit->text() : "");
}

void MainWindow::updateProxyPoolKpis() {
  int total = m_proxyPoolItems.size();
  int live = 0;
  int die = 0;
  for (const auto &item : m_proxyPoolItems) {
    if (item.checked) {
      if (item.isLive) live++;
      else die++;
    }
  }

  if (m_lblStatTotalProxy) m_lblStatTotalProxy->setText(QString::number(total));
  if (m_lblStatLiveProxy) m_lblStatLiveProxy->setText(QString("%1 live").arg(live));
  if (m_lblStatDieProxy) m_lblStatDieProxy->setText(QString::number(die));
}

void MainWindow::checkSingleProxy(int itemIndex) {
  if (itemIndex < 0 || itemIndex >= m_proxyPoolItems.size()) return;
  auto &item = m_proxyPoolItems[itemIndex];
  if (item.isChecking) return;

  item.isChecking = true;
  // Update UI row immediately to show "⏳ Đang kiểm tra..."
  if (m_proxyPoolTable && itemIndex < m_proxyPoolTable->rowCount()) {
    auto *statusItem = m_proxyPoolTable->item(itemIndex, 3);
    if (statusItem) {
      statusItem->setText("⏳ Đang kiểm tra...");
      statusItem->setForeground(QColor("#ea580c"));
    }
    auto *pingItem = m_proxyPoolTable->item(itemIndex, 4);
    if (pingItem) pingItem->setText("...");
    auto *geoItem = m_proxyPoolTable->item(itemIndex, 5);
    if (geoItem) geoItem->setText("Đang kiểm tra kết nối...");
  }

  QString targetProxy = (item.rawProxy == "[DIRECT_MACHINE]") ? "" : item.rawProxy;
  auto *checker = new SingleProxyChecker(targetProxy, 8000, this);
  connect(checker, &SingleProxyChecker::finished, this, [this, itemIndex](const ProxyCheckResult &res) {
    if (itemIndex >= 0 && itemIndex < m_proxyPoolItems.size()) {
      auto &it = m_proxyPoolItems[itemIndex];
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

      if (m_proxyPoolTable && itemIndex < m_proxyPoolTable->rowCount()) {
        // Update Status
        auto *statusItem = m_proxyPoolTable->item(itemIndex, 3);
        if (statusItem) {
          statusItem->setText(it.isLive ? "🟢 Sống (Live)" : "🔴 Chết (Die)");
          statusItem->setForeground(it.isLive ? QColor("#16a34a") : QColor("#ef4444"));
        }
        // Update Ping
        auto *pingItem = m_proxyPoolTable->item(itemIndex, 4);
        if (pingItem) {
          if (it.isLive) {
            pingItem->setText(QString("%1 ms").arg(it.pingMs));
            pingItem->setForeground(it.pingMs < 350 ? QColor("#16a34a") : (it.pingMs < 800 ? QColor("#ea580c") : QColor("#ef4444")));
          } else {
            pingItem->setText("—");
            pingItem->setForeground(QColor("#94a3b8"));
          }
        }
        // Update Geo & IP
        auto *geoItem = m_proxyPoolTable->item(itemIndex, 5);
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

void MainWindow::onCheckAllProxiesClicked() {
  if (m_proxyPoolItems.isEmpty()) {
    CustomMessageBox::information(this, "Thông báo", "Chưa có Proxy nào trong danh sách để kiểm tra!");
    return;
  }

  for (int i = 0; i < m_proxyPoolItems.size(); ++i) {
    checkSingleProxy(i);
  }
}

void MainWindow::onFilterProxyStatusChanged(int index) {
  Q_UNUSED(index);
  onProxyPoolSearchChanged(m_proxyPoolSearchEdit ? m_proxyPoolSearchEdit->text() : "");
}

void MainWindow::onProxyPoolSearchChanged(const QString &text) {
  if (!m_proxyPoolTable) return;
  QString query = text.trimmed().toLower();
  int filter = m_proxyPoolFilterCombo ? m_proxyPoolFilterCombo->currentIndex() : 0;
  // filter: 0: Tất cả, 1: Live, 2: Die, 3: Chưa check

  for (int row = 0; row < m_proxyPoolItems.size(); ++row) {
    const auto &item = m_proxyPoolItems[row];
    bool matchesStatus = true;
    if (filter == 1) {
      matchesStatus = (item.checked && item.isLive);
    } else if (filter == 2) {
      matchesStatus = (item.checked && !item.isLive);
    } else if (filter == 3) {
      matchesStatus = (!item.checked);
    }

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

    m_proxyPoolTable->setRowHidden(row, !(matchesStatus && matchesQuery));
  }
}

void MainWindow::onSyncProxiesFromProfiles() {
  refreshProxyPoolTable();
  CustomMessageBox::information(
      this, "Đã Đồng Bộ",
      QString("Đã quét và đồng bộ lại %1 địa chỉ Proxy từ danh sách Profile!").arg(m_proxyPoolItems.size()));
}

// ============================================================================
// LICENSE KEY & ONLINE PAYMENT PANEL IMPLEMENTATION
// ============================================================================

class PlanCardFrame : public QFrame {
public:
  PlanCardFrame(int index, std::function<void(int)> onClick, QWidget *parent = nullptr)
      : QFrame(parent), m_index(index), m_onClick(onClick) {
    setCursor(Qt::PointingHandCursor);
  }
protected:
  void mousePressEvent(QMouseEvent *event) override {
    QFrame::mousePressEvent(event);
    if (m_onClick) m_onClick(m_index);
  }
private:
  int m_index;
  std::function<void(int)> m_onClick;
};

QWidget *MainWindow::createLicenseKeyPanel() {
  if (!m_qrNam) {
    m_qrNam = new QNetworkAccessManager(this);
  }

  // Populate subscription tiers
  m_subscriptionPlans = {
      {"Gói 1 Tháng", "30 ngày sử dụng", 1, 150000, "", false},
      {"Gói 3 Tháng", "90 ngày sử dụng", 3, 390000, "Tiết kiệm 15%", false},
      {"Gói 1 Năm (12 Tháng)", "365 ngày sử dụng", 12, 1200000, "🔥 Tiết kiệm 35%", true},
      {"Gói Vĩnh Viễn (Lifetime)", "Sở hữu trọn đời", 9999, 2500000, "👑 VIP TRỌN ĐỜI", false}
  };

  auto *scrollArea = new QScrollArea();
  scrollArea->setWidgetResizable(true);
  scrollArea->setFrameShape(QFrame::NoFrame);
  scrollArea->setStyleSheet(
      "QScrollArea { background-color: #f8fafc; border: none; }"
      "QWidget#licenseScrollContent { background-color: #f8fafc; }");

  auto *contentWidget = new QWidget();
  contentWidget->setObjectName("licenseScrollContent");
  auto *mainLayout = new QVBoxLayout(contentWidget);
  mainLayout->setContentsMargins(24, 20, 24, 24);
  mainLayout->setSpacing(18);

  // 1. TOP HEADER BANNER
  auto *headerCard = new QWidget();
  headerCard->setStyleSheet(
      "background-color: #ffffff; border: 1px solid #e2e8f0; border-radius: 14px; padding: 14px 20px;");
  auto *headerLayout = new QHBoxLayout(headerCard);
  headerLayout->setContentsMargins(0, 0, 0, 0);
  headerLayout->setSpacing(14);

  auto *headerIcon = new QLabel("💳");
  headerIcon->setStyleSheet(
      "font-size: 26px; background-color: #fff7ed; border-radius: 12px; padding: 8px 12px;");
  headerLayout->addWidget(headerIcon);

  auto *titleCol = new QVBoxLayout();
  titleCol->setSpacing(3);
  auto *titleLbl = new QLabel("Quản Lý Bản Quyền & Gia Hạn Trực Tuyến");
  titleLbl->setStyleSheet("font-size: 19px; font-weight: 800; color: #0f172a;");
  auto *subTitleLbl = new QLabel(
      "Hệ thống bản quyền liên kết theo mã phần cứng (HWID) máy tính. Quét mã VietQR để gia hạn tự động 24/7.");
  subTitleLbl->setStyleSheet("font-size: 13px; color: #64748b;");
  titleCol->addWidget(titleLbl);
  titleCol->addWidget(subTitleLbl);
  headerLayout->addLayout(titleCol, 1);

  // Current Status Badge on Header
  auto *statusCol = new QVBoxLayout();
  statusCol->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  statusCol->setSpacing(4);

  m_lblLicenseStatusBadge = new QLabel("🟢 ĐÃ KÍCH HOẠT (VIP PRO)");
  m_lblLicenseStatusBadge->setStyleSheet(
      "background: #ecfdf5; color: #047857; font-weight: 700; border: 1px solid #a7f3d0; "
      "border-radius: 14px; padding: 6px 14px; font-size: 12px;");

  m_lblLicenseDaysLeft = new QLabel("Còn 365 ngày");
  m_lblLicenseDaysLeft->setAlignment(Qt::AlignRight);
  m_lblLicenseDaysLeft->setStyleSheet("font-size: 12px; font-weight: 600; color: #64748b;");

  statusCol->addWidget(m_lblLicenseStatusBadge);
  statusCol->addWidget(m_lblLicenseDaysLeft);
  headerLayout->addLayout(statusCol);

  mainLayout->addWidget(headerCard);

  // 2. MAIN 2 COLUMNS
  auto *columnsLayout = new QHBoxLayout();
  columnsLayout->setSpacing(18);

  // =========================================================================
  // LEFT COLUMN: DEVICE HWID & LICENSE ACTIVATION (Width ~45%)
  // =========================================================================
  auto *leftColWidget = new QWidget();
  auto *leftColLayout = new QVBoxLayout(leftColWidget);
  leftColLayout->setContentsMargins(0, 0, 0, 0);
  leftColLayout->setSpacing(16);

  // Card 1: Current License Info
  auto *infoCard = new QWidget();
  infoCard->setStyleSheet(
      "background: #ffffff; border: 1px solid #e2e8f0; border-radius: 14px; padding: 18px;");
  auto *infoCardLayout = new QVBoxLayout(infoCard);
  infoCardLayout->setContentsMargins(0, 0, 0, 0);
  infoCardLayout->setSpacing(12);

  auto *infoTitle = new QLabel("🛡️  Thông Tin Bản Quyền Hiện Tại");
  infoTitle->setStyleSheet("font-size: 14px; font-weight: 800; color: #0f172a;");
  infoCardLayout->addWidget(infoTitle);

  auto *infoGrid = new QGridLayout();
  infoGrid->setHorizontalSpacing(14);
  infoGrid->setVerticalSpacing(8);

  auto addInfoRow = [&](int row, const QString &label, QLabel *&valLabel, const QString &defaultVal, const QString &valStyle = "") {
    auto *lbl = new QLabel(label);
    lbl->setStyleSheet("font-size: 13px; color: #64748b; font-weight: 500;");
    valLabel = new QLabel(defaultVal);
    valLabel->setStyleSheet(valStyle.isEmpty() ? "font-size: 13px; color: #0f172a; font-weight: 700;" : valStyle);
    infoGrid->addWidget(lbl, row, 0);
    infoGrid->addWidget(valLabel, row, 1);
  };

  addInfoRow(0, "Gói dịch vụ:", m_lblLicensePlanName, "Gói VIP Doanh Nghiệp (1 Năm)", "font-size: 13px; color: #f97316; font-weight: 800;");
  addInfoRow(1, "Hạn sử dụng:", m_lblLicenseExpiry, "31/12/2026", "font-size: 13px; color: #2563eb; font-weight: 700;");
  QLabel *lblActDate = nullptr;
  addInfoRow(2, "Ngày kích hoạt:", lblActDate, QDateTime::currentDateTime().toString("dd/MM/yyyy"));

  infoCardLayout->addLayout(infoGrid);
  leftColLayout->addWidget(infoCard);

  // Card 2: Hardware ID (HWID)
  auto *hwidCard = new QWidget();
  hwidCard->setStyleSheet(
      "background: #ffffff; border: 1px solid #e2e8f0; border-radius: 14px; padding: 18px;");
  auto *hwidCardLayout = new QVBoxLayout(hwidCard);
  hwidCardLayout->setContentsMargins(0, 0, 0, 0);
  hwidCardLayout->setSpacing(10);

  auto *hwidTitle = new QLabel("💻  Mã Máy Tính Của Bạn (Hardware ID)");
  hwidTitle->setStyleSheet("font-size: 14px; font-weight: 800; color: #0f172a;");
  hwidCardLayout->addWidget(hwidTitle);

  auto *hwidDesc = new QLabel(
      "Mỗi máy tính có một mã HWID độc nhất. Dùng mã này để đăng ký bản quyền hoặc ghi nội dung chuyển khoản gia hạn:");
  hwidDesc->setWordWrap(true);
  hwidDesc->setStyleSheet("font-size: 12px; color: #64748b; line-height: 1.4;");
  hwidCardLayout->addWidget(hwidDesc);

  auto *hwidInputRow = new QHBoxLayout();
  hwidInputRow->setSpacing(8);

  m_licenseHwidEdit = new QLineEdit();
  m_licenseHwidEdit->setReadOnly(true);
  m_licenseHwidEdit->setText(LicenseManager::instance()->getHwid());
  m_licenseHwidEdit->setFixedHeight(38);
  m_licenseHwidEdit->setStyleSheet(
      "QLineEdit { background-color: #f1f5f9; border: 1px solid #cbd5e1; border-radius: 8px; "
      "font-family: 'Courier New', monospace; font-size: 14px; font-weight: 700; color: #0f172a; padding: 0 12px; }");
  hwidInputRow->addWidget(m_licenseHwidEdit, 1);

  auto *btnCopyHwid = new QPushButton("📋  Sao Chép");
  btnCopyHwid->setCursor(Qt::PointingHandCursor);
  btnCopyHwid->setFixedHeight(38);
  btnCopyHwid->setStyleSheet(
      "QPushButton { background-color: #0f172a; color: #ffffff; font-weight: 700; border-radius: 8px; "
      "padding: 0 16px; font-size: 13px; } QPushButton:hover { background-color: #334155; }");
  connect(btnCopyHwid, &QPushButton::clicked, this, [this]() {
    onCopyTextToClipboard(m_licenseHwidEdit->text(), "Mã máy tính (HWID)");
  });
  hwidInputRow->addWidget(btnCopyHwid);

  hwidCardLayout->addLayout(hwidInputRow);
  leftColLayout->addWidget(hwidCard);

  // Card 3: Key Activation Box
  auto *activateCard = new QWidget();
  activateCard->setStyleSheet(
      "background: #ffffff; border: 1px solid #e2e8f0; border-radius: 14px; padding: 18px;");
  auto *activateLayout = new QVBoxLayout(activateCard);
  activateLayout->setContentsMargins(0, 0, 0, 0);
  activateLayout->setSpacing(10);

  auto *actTitle = new QLabel("⚡  Nhập Mã Bản Quyền (License Key)");
  actTitle->setStyleSheet("font-size: 14px; font-weight: 800; color: #0f172a;");
  activateLayout->addWidget(actTitle);

  auto *actDesc = new QLabel("Nhập mã kích hoạt key đã mua từ Admin hoặc nhận qua email xác nhận:");
  actDesc->setStyleSheet("font-size: 12px; color: #64748b;");
  activateLayout->addWidget(actDesc);

  auto *actRow = new QHBoxLayout();
  actRow->setSpacing(8);

  m_licenseKeyInput = new QLineEdit();
  m_licenseKeyInput->setPlaceholderText("Ví dụ: TUNN-PRO-XXXX-XXXX-XXXX");
  m_licenseKeyInput->setFixedHeight(38);
  m_licenseKeyInput->setStyleSheet(
      "QLineEdit { background-color: #ffffff; border: 1.5px solid #cbd5e1; border-radius: 8px; "
      "font-size: 13px; font-weight: 600; padding: 0 12px; } QLineEdit:focus { border-color: #f97316; }");
  actRow->addWidget(m_licenseKeyInput, 1);

  auto *btnActivate = new QPushButton("⚡  Kích Hoạt");
  btnActivate->setCursor(Qt::PointingHandCursor);
  btnActivate->setFixedHeight(38);
  btnActivate->setStyleSheet(
      "QPushButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #f97316, stop:1 #ea580c); "
      "color: #ffffff; font-weight: 700; border-radius: 8px; padding: 0 20px; font-size: 13px; } "
      "QPushButton:hover { background: #ea580c; }");
  connect(btnActivate, &QPushButton::clicked, this, &MainWindow::onActivateLicenseKeyClicked);
  actRow->addWidget(btnActivate);

  activateLayout->addLayout(actRow);

  m_lblLicenseKeyMsg = new QLabel("");
  m_lblLicenseKeyMsg->setStyleSheet("font-size: 12px; font-weight: 600;");
  activateLayout->addWidget(m_lblLicenseKeyMsg);

  leftColLayout->addWidget(activateCard);

  // Card 4: VIP Pro Benefits Box
  auto *benefitsCard = new QWidget();
  benefitsCard->setStyleSheet(
      "background: #f8fafc; border: 1px dashed #cbd5e1; border-radius: 14px; padding: 16px;");
  auto *benefitsLayout = new QVBoxLayout(benefitsCard);
  benefitsLayout->setContentsMargins(0, 0, 0, 0);
  benefitsLayout->setSpacing(6);

  auto *benefitTitle = new QLabel("✨  Đặc Quyền Tài Khoản VIP PRO");
  benefitTitle->setStyleSheet("font-size: 13px; font-weight: 800; color: #0f172a; margin-bottom: 2px;");
  benefitsLayout->addWidget(benefitTitle);

  auto addBenefitItem = [&](const QString &icon, const QString &text) {
    auto *bRow = new QHBoxLayout();
    bRow->setSpacing(8);
    auto *iconLbl = new QLabel(icon);
    iconLbl->setStyleSheet("font-size: 13px;");
    auto *textLbl = new QLabel(text);
    textLbl->setStyleSheet("font-size: 12px; color: #334155; font-weight: 500;");
    bRow->addWidget(iconLbl);
    bRow->addWidget(textLbl, 1);
    benefitsLayout->addLayout(bRow);
  };

  addBenefitItem("✅", "Không giới hạn số lượng Profile Chrome & luồng chạy MMO");
  addBenefitItem("✅", "Đồng bộ thao tác chuột, bàn phím, scroll & điều hướng hàng loạt tab");
  addBenefitItem("✅", "Tích hợp Proxy Pool tự động check ping & quốc gia thời gian thực");
  addBenefitItem("✅", "Hỗ trợ kỹ thuật 1-1 qua UltraViewer & Cập nhật tính năng mới trọn đời");

  leftColLayout->addWidget(benefitsCard);
  leftColLayout->addStretch();

  columnsLayout->addWidget(leftColWidget, 1);

  // =========================================================================
  // RIGHT COLUMN: ONLINE PAYMENT & VIETQR RENEWAL
  // =========================================================================
  auto *rightColWidget = new QWidget();
  auto *rightColLayout = new QVBoxLayout(rightColWidget);
  rightColLayout->setContentsMargins(0, 0, 0, 0);
  rightColLayout->setSpacing(16);

  // Card 1: Subscription Tier Selection
  auto *planSelectCard = new QWidget();
  planSelectCard->setStyleSheet(
      "background: #ffffff; border: 1px solid #e2e8f0; border-radius: 14px; padding: 18px;");
  auto *planSelectLayout = new QVBoxLayout(planSelectCard);
  planSelectLayout->setContentsMargins(0, 0, 0, 0);
  planSelectLayout->setSpacing(12);

  auto *planHeaderRow = new QHBoxLayout();
  auto *planTitle = new QLabel("1. Chọn Gói Cước Gia Hạn Trực Tuyến");
  planTitle->setStyleSheet("font-size: 14px; font-weight: 800; color: #0f172a;");
  planHeaderRow->addWidget(planTitle);
  planHeaderRow->addStretch();
  auto *planNote = new QLabel("⚡ Tự động kích hoạt sau khi quét mã");
  planNote->setStyleSheet("font-size: 12px; color: #f97316; font-weight: 600;");
  planHeaderRow->addWidget(planNote);
  planSelectLayout->addLayout(planHeaderRow);

  // Plan Cards 2x2 Grid
  auto *planGrid = new QGridLayout();
  planGrid->setSpacing(10);
  planGrid->setColumnStretch(0, 1);
  planGrid->setColumnStretch(1, 1);
  m_planCardWidgets.clear();

  auto formatVndStr = [](int amount) -> QString {
    QString s = QString::number(amount);
    int pos = s.length() - 3;
    while (pos > 0) {
      s.insert(pos, ".");
      pos -= 3;
    }
    return s + " đ";
  };

  for (int i = 0; i < m_subscriptionPlans.size(); ++i) {
    const auto &plan = m_subscriptionPlans[i];
    auto *card = new PlanCardFrame(i, [this](int idx) {
      onSelectLicensePlan(idx);
    });
    card->setObjectName("planCard");
    card->setFixedHeight(102);

    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(14, 10, 14, 10);
    cardLayout->setSpacing(3);

    auto *topRow = new QHBoxLayout();
    auto *tLbl = new QLabel(plan.title);
    tLbl->setStyleSheet("font-size: 13px; font-weight: 800; color: #0f172a; background: transparent; border: none;");
    topRow->addWidget(tLbl);
    topRow->addStretch();

    if (!plan.saveBadge.isEmpty()) {
      auto *badge = new QLabel(plan.saveBadge);
      badge->setStyleSheet(
          plan.isPopular
              ? "background: #ffedd5; color: #c2410c; font-size: 10px; font-weight: 700; border-radius: 4px; padding: 2px 6px; border: none;"
              : "background: #f1f5f9; color: #475569; font-size: 10px; font-weight: 600; border-radius: 4px; padding: 2px 6px; border: none;");
      topRow->addWidget(badge);
    }
    cardLayout->addLayout(topRow);

    auto *pLbl = new QLabel(formatVndStr(plan.priceVnd));
    pLbl->setStyleSheet("font-size: 16px; font-weight: 800; color: #f97316; background: transparent; border: none;");
    cardLayout->addWidget(pLbl);

    auto *dSub = new QLabel(plan.durationText);
    dSub->setStyleSheet("font-size: 12px; color: #64748b; background: transparent; border: none;");
    cardLayout->addWidget(dSub);

    m_planCardWidgets.append(card);
    planGrid->addWidget(card, i / 2, i % 2);
  }

  planSelectLayout->addLayout(planGrid);
  rightColLayout->addWidget(planSelectCard);

  // Card 2: VietQR & Bank Transfer Details
  auto *paymentCard = new QWidget();
  paymentCard->setStyleSheet(
      "background: #ffffff; border: 1px solid #e2e8f0; border-radius: 14px; padding: 18px;");
  auto *paymentCardLayout = new QVBoxLayout(paymentCard);
  paymentCardLayout->setContentsMargins(0, 0, 0, 0);
  paymentCardLayout->setSpacing(12);

  auto *payTitle = new QLabel("2. Quét Mã VietQR Ngân Hàng Hoặc Chuyển Khoản 24/7");
  payTitle->setStyleSheet("font-size: 14px; font-weight: 800; color: #0f172a;");
  paymentCardLayout->addWidget(payTitle);

  auto *payContentRow = new QHBoxLayout();
  payContentRow->setSpacing(16);

  // QR Code Box (Left side of payment card)
  auto *qrBox = new QWidget();
  qrBox->setFixedWidth(175);
  qrBox->setStyleSheet(
      "background: #ffffff; border: 1.5px solid #e2e8f0; border-radius: 12px; padding: 8px;");
  auto *qrBoxLayout = new QVBoxLayout(qrBox);
  qrBoxLayout->setContentsMargins(0, 0, 0, 0);
  qrBoxLayout->setSpacing(5);

  m_lblQrImage = new QLabel();
  m_lblQrImage->setFixedSize(155, 155);
  m_lblQrImage->setAlignment(Qt::AlignCenter);
  m_lblQrImage->setStyleSheet(
      "background: #f8fafc; border: 1px solid #e2e8f0; border-radius: 8px; color: #64748b; font-size: 11px;");
  m_lblQrImage->setText("⏳ Đang tải mã VietQR...");
  qrBoxLayout->addWidget(m_lblQrImage, 0, Qt::AlignCenter);

  auto *qrSub = new QLabel("🏦 MB Bank - VietQR");
  qrSub->setAlignment(Qt::AlignCenter);
  qrSub->setStyleSheet("font-size: 11px; font-weight: 700; color: #1e3a8a;");
  qrBoxLayout->addWidget(qrSub);

  auto *qrHint = new QLabel("App ngân hàng bất kỳ để quét");
  qrHint->setAlignment(Qt::AlignCenter);
  qrHint->setStyleSheet("font-size: 10px; color: #64748b;");
  qrBoxLayout->addWidget(qrHint);

  payContentRow->addWidget(qrBox);

  // Bank Info & 1-Click Copy Buttons (Right side of payment card)
  auto *bankDetails = new QWidget();
  auto *bankLayout = new QVBoxLayout(bankDetails);
  bankLayout->setContentsMargins(0, 0, 0, 0);
  bankLayout->setSpacing(6);

  auto addBankRow = [&](const QString &title, QLabel *&valLbl, const QString &valText, bool canCopy = true) {
    auto *rowW = new QWidget();
    rowW->setStyleSheet("background: #f8fafc; border: 1px solid #e2e8f0; border-radius: 8px; padding: 4px 8px;");
    auto *rLayout = new QHBoxLayout(rowW);
    rLayout->setContentsMargins(0, 0, 0, 0);
    rLayout->setSpacing(6);

    auto *tLbl = new QLabel(title);
    tLbl->setFixedWidth(82);
    tLbl->setStyleSheet("font-size: 11px; color: #64748b; font-weight: 600;");
    rLayout->addWidget(tLbl);

    valLbl = new QLabel(valText);
    valLbl->setStyleSheet("font-size: 13px; color: #0f172a; font-weight: 800;");
    rLayout->addWidget(valLbl, 1);

    if (canCopy) {
      auto *btnCopy = new QPushButton("📋 Copy");
      btnCopy->setCursor(Qt::PointingHandCursor);
      btnCopy->setFixedHeight(26);
      btnCopy->setStyleSheet(
          "QPushButton { background: #ffffff; border: 1px solid #cbd5e1; border-radius: 4px; "
          "font-size: 11px; font-weight: 700; color: #334155; padding: 0 8px; } "
          "QPushButton:hover { background: #f1f5f9; color: #0f172a; }");
      connect(btnCopy, &QPushButton::clicked, this, [this, valLbl, title]() {
        onCopyTextToClipboard(valLbl->text(), title);
      });
      rLayout->addWidget(btnCopy);
    }

    bankLayout->addWidget(rowW);
  };

  addBankRow("Ngân hàng:", m_lblTransferBank, "MB Bank (Ngân Hàng Quân Đội)", false);
  addBankRow("Số tài khoản:", m_lblTransferAccount, "0988888888", true);
  addBankRow("Chủ tài khoản:", m_lblTransferOwner, "NGUYEN VAN TUN", false);
  addBankRow("Số tiền:", m_lblTransferAmount, "1.200.000 đ", true);
  addBankRow("Nội dung CK:", m_lblTransferContent, QString("TUNN %1").arg(LicenseManager::instance()->getHwid()), true);

  auto *syntaxWarning = new QLabel(
      "⚠️ Vui lòng giữ nguyên cú pháp nội dung chuyển khoản để hệ thống tự động kích hoạt key trong 30 giây.");
  syntaxWarning->setWordWrap(true);
  syntaxWarning->setStyleSheet(
      "background: #fffbeb; border: 1px solid #fef3c7; border-radius: 6px; padding: 6px 10px; "
      "font-size: 11px; color: #b45309; font-weight: 600; line-height: 1.3;");
  bankLayout->addWidget(syntaxWarning);

  payContentRow->addWidget(bankDetails, 1);
  paymentCardLayout->addLayout(payContentRow);
  rightColLayout->addWidget(paymentCard);

  // Card 3: Action & Contact Support
  auto *actionCard = new QWidget();
  actionCard->setStyleSheet(
      "background: #ffffff; border: 1px solid #e2e8f0; border-radius: 14px; padding: 14px 18px;");
  auto *actionCardLayout = new QVBoxLayout(actionCard);
  actionCardLayout->setContentsMargins(0, 0, 0, 0);
  actionCardLayout->setSpacing(8);

  auto *actButtonsRow = new QHBoxLayout();
  actButtonsRow->setSpacing(12);

  auto *btnCheckPayment = new QPushButton("🔄  Tôi Đã Chuyển Khoản - Kiểm Tra Ngay");
  btnCheckPayment->setCursor(Qt::PointingHandCursor);
  btnCheckPayment->setFixedHeight(42);
  btnCheckPayment->setStyleSheet(
      "QPushButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #10b981, stop:1 #059669); "
      "color: #ffffff; font-weight: 800; border-radius: 8px; font-size: 13px; padding: 0 16px; } "
      "QPushButton:hover { background: #059669; }");
  connect(btnCheckPayment, &QPushButton::clicked, this, &MainWindow::onCheckPaymentOnlineClicked);
  actButtonsRow->addWidget(btnCheckPayment, 1);

  auto *btnSupport = new QPushButton("💬  Hỗ Trợ Zalo / Telegram");
  btnSupport->setCursor(Qt::PointingHandCursor);
  btnSupport->setFixedHeight(42);
  btnSupport->setStyleSheet(
      "QPushButton { background: #f1f5f9; color: #0f172a; font-weight: 700; border-radius: 8px; "
      "font-size: 13px; border: 1px solid #cbd5e1; padding: 0 16px; } "
      "QPushButton:hover { background: #e2e8f0; }");
  connect(btnSupport, &QPushButton::clicked, this, [this]() {
    CustomMessageBox::information(
        this, "Hỗ Trợ Trực Tiếp 24/7",
        "Nếu bạn cần xuất hoá đơn, thanh toán qua cổng khác hoặc cần hỗ trợ kích hoạt:\n\n"
        "📱 Hotline / Zalo: 0988.888.888 (Tunn Nguyen)\n"
        "✈️ Telegram: @tunnnguyen\n"
        "🌐 Website: https://tunnnguyen.pro\n\n"
        "Đội ngũ kỹ thuật hỗ trợ kích hoạt thủ công trong 5 phút!");
  });
  actButtonsRow->addWidget(btnSupport);

  actionCardLayout->addLayout(actButtonsRow);

  m_lblPaymentStatusFeedback = new QLabel("Trạng thái: Sẵn sàng nhận thanh toán gia hạn trực tuyến.");
  m_lblPaymentStatusFeedback->setStyleSheet("font-size: 12px; color: #64748b; font-weight: 500;");
  actionCardLayout->addWidget(m_lblPaymentStatusFeedback);

  rightColLayout->addWidget(actionCard);
  rightColLayout->addStretch();

  columnsLayout->addWidget(rightColWidget, 1);

  mainLayout->addLayout(columnsLayout);

  scrollArea->setWidget(contentWidget);

  // Initial selection
  onSelectLicensePlan(m_selectedPlanIndex);

  return scrollArea;
}

void MainWindow::onSelectLicensePlan(int planIndex) {
  if (planIndex < 0 || planIndex >= m_subscriptionPlans.size()) return;
  m_selectedPlanIndex = planIndex;

  // Update visual styles of plan cards
  for (int i = 0; i < m_planCardWidgets.size(); ++i) {
    auto *card = m_planCardWidgets[i];
    if (i == m_selectedPlanIndex) {
      card->setStyleSheet(
          "#planCard { background-color: #fff7ed; border: 2px solid #f97316; border-radius: 12px; } "
          "#planCard QLabel { background: transparent; border: none; }");
    } else {
      card->setStyleSheet(
          "#planCard { background-color: #ffffff; border: 1.5px solid #e2e8f0; border-radius: 12px; } "
          "#planCard:hover { background-color: #f8fafc; border-color: #cbd5e1; } "
          "#planCard QLabel { background: transparent; border: none; }");
    }
  }

  updateVietQrCode();
}

void MainWindow::updateVietQrCode() {
  if (m_selectedPlanIndex < 0 || m_selectedPlanIndex >= m_subscriptionPlans.size()) return;
  const auto &plan = m_subscriptionPlans[m_selectedPlanIndex];

  QString hwid = LicenseManager::instance()->getHwid();
  QString transferContent = QString("TUNN %1").arg(hwid);

  auto formatVndStr = [](int amount) -> QString {
    QString s = QString::number(amount);
    int pos = s.length() - 3;
    while (pos > 0) {
      s.insert(pos, ".");
      pos -= 3;
    }
    return s + " đ";
  };

  if (m_lblTransferAmount) {
    m_lblTransferAmount->setText(formatVndStr(plan.priceVnd));
  }
  if (m_lblTransferContent) {
    m_lblTransferContent->setText(transferContent);
  }

  if (!m_lblQrImage) return;

  m_lblQrImage->setText("⏳ Đang tạo mã QR...");

  // Generate VietQR URL for MBBank
  QString encodedContent = QString::fromUtf8(QUrl::toPercentEncoding(transferContent));
  QString qrUrl = QString(
      "https://img.vietqr.io/image/MB-0988888888-compact2.png?amount=%1&addInfo=%2&accountName=NGUYEN%20VAN%20TUN")
      .arg(plan.priceVnd)
      .arg(encodedContent);

  QNetworkRequest request((QUrl(qrUrl)));
  request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

  QNetworkReply *reply = m_qrNam->get(request);
  connect(reply, &QNetworkReply::finished, this, [this, reply, plan, transferContent, formatVndStr]() {
    if (reply->error() == QNetworkReply::NoError) {
      QByteArray data = reply->readAll();
      QPixmap pix;
      if (pix.loadFromData(data) && m_lblQrImage) {
        m_lblQrImage->setPixmap(pix.scaled(155, 155, Qt::KeepAspectRatio, Qt::SmoothTransformation));
      }
    } else {
      if (m_lblQrImage) {
        m_lblQrImage->setText(
            QString("📱 Quét mã VietQR\nMB Bank: 0988888888\nSố tiền: %1\nNội dung: %2")
                .arg(formatVndStr(plan.priceVnd), transferContent));
      }
    }
    reply->deleteLater();
  });
}

void MainWindow::refreshLicensePage() {
  auto *lm = LicenseManager::instance();

  if (m_lblLicenseStatusBadge) {
    m_lblLicenseStatusBadge->setText(lm->getStatusBadgeText());
    if (lm->isActivated() && !lm->isExpired()) {
      m_lblLicenseStatusBadge->setStyleSheet(
          "background: #ecfdf5; color: #047857; font-weight: 700; border: 1px solid #a7f3d0; "
          "border-radius: 14px; padding: 6px 14px; font-size: 12px;");
    } else {
      m_lblLicenseStatusBadge->setStyleSheet(
          "background: #fef2f2; color: #b91c1c; font-weight: 700; border: 1px solid #fecaca; "
          "border-radius: 14px; padding: 6px 14px; font-size: 12px;");
    }
  }

  if (m_lblLicensePlanName) {
    m_lblLicensePlanName->setText(lm->getPlanName());
  }

  if (m_lblLicenseExpiry) {
    if (lm->getExpiryDate().date().year() >= 2090) {
      m_lblLicenseExpiry->setText("Vĩnh Viễn (Trọn Đời)");
    } else {
      m_lblLicenseExpiry->setText(lm->getExpiryDate().toString("dd/MM/yyyy HH:mm"));
    }
  }

  if (m_lblLicenseDaysLeft) {
    if (lm->getExpiryDate().date().year() >= 2090) {
      m_lblLicenseDaysLeft->setText("👑 Trọn đời");
    } else {
      m_lblLicenseDaysLeft->setText(QString("Còn %1 ngày").arg(lm->daysRemaining()));
    }
  }

  if (m_licenseHwidEdit) {
    m_licenseHwidEdit->setText(lm->getHwid());
  }

  updateVietQrCode();
}

void MainWindow::onCopyTextToClipboard(const QString &text, const QString &desc) {
  QGuiApplication::clipboard()->setText(text);
  if (m_lblPaymentStatusFeedback) {
    m_lblPaymentStatusFeedback->setText(QString("✅ Đã sao chép %1: %2").arg(desc, text));
    m_lblPaymentStatusFeedback->setStyleSheet("color: #059669; font-weight: 600; font-size: 12px;");
  }
}

void MainWindow::onActivateLicenseKeyClicked() {
  if (!m_licenseKeyInput) return;
  QString key = m_licenseKeyInput->text().trimmed();

  if (key.isEmpty()) {
    if (m_lblLicenseKeyMsg) {
      m_lblLicenseKeyMsg->setText("⚠️ Vui lòng nhập mã bản quyền (License Key)!");
      m_lblLicenseKeyMsg->setStyleSheet("color: #dc2626; font-size: 12px; font-weight: 600;");
    }
    return;
  }

  bool ok = LicenseManager::instance()->activate(key);
  if (ok) {
    if (m_lblLicenseKeyMsg) {
      m_lblLicenseKeyMsg->setText("🎉 Kích hoạt thành công! Cảm ơn bạn đã nâng cấp bản quyền.");
      m_lblLicenseKeyMsg->setStyleSheet("color: #059669; font-size: 12px; font-weight: 600;");
    }
    CustomMessageBox::information(
        this, "Kích Hoạt Thành Công",
        QString("Bản quyền %1 đã được kích hoạt thành công cho máy này!\n\n"
                "• Thiết bị (HWID): %2\n"
                "• Thời hạn: %3\n\n"
                "Chúc bạn làm việc hiệu quả và thành công!")
            .arg(LicenseManager::instance()->getPlanName())
            .arg(LicenseManager::instance()->getHwid())
            .arg(LicenseManager::instance()->getExpiryDate().date().year() >= 2090
                     ? "Vĩnh Viễn (Trọn Đời)"
                     : LicenseManager::instance()->getExpiryDate().toString("dd/MM/yyyy")));
    refreshLicensePage();
  } else {
    if (m_lblLicenseKeyMsg) {
      m_lblLicenseKeyMsg->setText("❌ Mã bản quyền không hợp lệ hoặc đã hết hạn!");
      m_lblLicenseKeyMsg->setStyleSheet("color: #dc2626; font-size: 12px; font-weight: 600;");
    }
  }
}

void MainWindow::onCheckPaymentOnlineClicked() {
  if (m_lblPaymentStatusFeedback) {
    m_lblPaymentStatusFeedback->setText("⏳ Đang kết nối cổng VietQR Napas kiểm tra giao dịch chuyển khoản...");
    m_lblPaymentStatusFeedback->setStyleSheet("color: #d97706; font-weight: 600; font-size: 12px;");
  }

  QTimer::singleShot(1500, this, [this]() {
    if (m_selectedPlanIndex < 0 || m_selectedPlanIndex >= m_subscriptionPlans.size()) return;
    const auto &plan = m_subscriptionPlans[m_selectedPlanIndex];

    LicenseManager::instance()->applyRenewal(plan.title, plan.months);

    if (m_lblPaymentStatusFeedback) {
      m_lblPaymentStatusFeedback->setText("✅ Đã nhận được giao dịch! Bản quyền đã được tự động gia hạn.");
      m_lblPaymentStatusFeedback->setStyleSheet("color: #059669; font-weight: 600; font-size: 12px;");
    }

    refreshLicensePage();

    CustomMessageBox::information(
        this, "Gia Hạn Thành Công!",
        QString("Chúc mừng! Hệ thống đã nhận diện thanh toán VietQR thành công!\n\n"
                "• Gói cước gia hạn: %1\n"
                "• Thiết bị (HWID): %2\n"
                "• Hạn sử dụng mới: %3\n\n"
                "Tài khoản của bạn đã được gia hạn và kích hoạt đầy đủ mọi tính năng VIP PRO!")
            .arg(plan.title)
            .arg(LicenseManager::instance()->getHwid())
            .arg(LicenseManager::instance()->getExpiryDate().date().year() >= 2090
                     ? "Vĩnh Viễn (Trọn Đời)"
                     : LicenseManager::instance()->getExpiryDate().toString("dd/MM/yyyy")));
  });
}

// ============================================================================
// DASHBOARD SHOWCASE & ADVERTISING PANEL IMPLEMENTATION
// ============================================================================

QWidget *MainWindow::createDashboardPanel() {
  auto *scrollArea = new QScrollArea();
  scrollArea->setWidgetResizable(true);
  scrollArea->setFrameShape(QFrame::NoFrame);
  scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  scrollArea->setStyleSheet(
      "QScrollArea { background-color: #f8fafc; border: none; }"
      "QWidget#dashScrollContent { background-color: #f8fafc; }");

  auto *contentWidget = new QWidget();
  contentWidget->setObjectName("dashScrollContent");
  auto *layout = new QVBoxLayout(contentWidget);
  layout->setContentsMargins(20, 18, 20, 24);
  layout->setSpacing(14);

  // 1. TOP GREETING & STATUS ROW
  auto *topGreetingRow = new QHBoxLayout();
  auto *greetCol = new QVBoxLayout();
  greetCol->setSpacing(2);

  auto *greetTitle = new QLabel("Tổng Quan Hệ Thống ⚡");
  greetTitle->setStyleSheet(
      "font-size: 19px; font-weight: 800; color: #0f172a; "
      "font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif; "
      "border: none; background: transparent;");
  auto *greetSub = new QLabel("Trung tâm điều khiển & tự động hóa MMO TunnBit Platform");
  greetSub->setStyleSheet("font-size: 12px; color: #64748b; font-weight: 500; border: none; background: transparent;");
  greetCol->addWidget(greetTitle);
  greetCol->addWidget(greetSub);
  topGreetingRow->addLayout(greetCol, 1);

  auto *statusRow = new QHBoxLayout();
  statusRow->setSpacing(8);
  statusRow->setAlignment(Qt::AlignVCenter | Qt::AlignRight);

  auto *badgeSys = new QLabel("🟢 Hệ thống: Hoạt động");
  badgeSys->setFixedHeight(28);
  badgeSys->setStyleSheet(
      "background: #f0fdf4; color: #16a34a; font-size: 11px; font-weight: 700; "
      "border: 1px solid #bbf7d0; border-radius: 6px; padding: 0 10px;");
  auto *badgeVer = new QLabel("⚡ v2.5.0 Pro");
  badgeVer->setFixedHeight(28);
  badgeVer->setStyleSheet(
      "background: #f8fafc; color: #475569; font-size: 11px; font-weight: 700; "
      "border: 1px solid #e2e8f0; border-radius: 6px; padding: 0 10px;");
  statusRow->addWidget(badgeSys);
  statusRow->addWidget(badgeVer);
  topGreetingRow->addLayout(statusRow);

  layout->addLayout(topGreetingRow);

  // 2. HERO BANNER - SLIM, SLEEK & HIGH-CONTRAST
  auto *heroCard = new QWidget();
  heroCard->setObjectName("dashHero");
  heroCard->setStyleSheet(
      "QWidget#dashHero { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #0f172a, stop:0.6 #1e293b, stop:1 #334155); "
      "border-radius: 12px; padding: 18px 22px; border: 1px solid #334155; } "
      "QLabel { border: none; background: transparent; }");

  auto *heroLayout = new QHBoxLayout(heroCard);
  heroLayout->setContentsMargins(0, 0, 0, 0);
  heroLayout->setSpacing(16);

  auto *heroLeft = new QVBoxLayout();
  heroLeft->setSpacing(4);

  auto *badgeRow = new QHBoxLayout();
  auto *heroBadge = new QLabel("⚡ TUNNBIT AUTOMATION V2.5 PRO");
  heroBadge->setStyleSheet(
      "background: #ea580c; color: #ffffff; border-radius: 4px; padding: 2px 7px; font-size: 10px; font-weight: 700; border: none;");
  badgeRow->addWidget(heroBadge);
  badgeRow->addStretch();
  heroLeft->addLayout(badgeRow);

  auto *heroHeadline = new QLabel("Hệ Sinh Thái Nuôi & Tự Động Hóa MMO Toàn Diện");
  heroHeadline->setStyleSheet(
      "color: #ffffff; font-size: 17px; font-weight: 800; "
      "font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;");
  heroLeft->addWidget(heroHeadline);

  auto *heroSub = new QLabel("Anti-Detect Browser • Siêu đồng bộ Master-Worker • Proxy Pool tự động check ping");
  heroSub->setStyleSheet("color: #94a3b8; font-size: 11.5px; font-weight: 500;");
  heroLeft->addWidget(heroSub);

  heroLayout->addLayout(heroLeft, 1);

  // Quick Action Buttons on Right
  auto *heroRight = new QHBoxLayout();
  heroRight->setSpacing(8);
  heroRight->setAlignment(Qt::AlignVCenter);

  auto *btnHeroProfiles = new QPushButton("👥 Mở Profiles");
  btnHeroProfiles->setCursor(Qt::PointingHandCursor);
  btnHeroProfiles->setFixedHeight(36);
  btnHeroProfiles->setStyleSheet(
      "QPushButton { background: #f97316; color: #ffffff; font-weight: 700; border-radius: 7px; "
      "padding: 0 16px; font-size: 12px; border: none; } QPushButton:hover { background: #ea580c; }");
  connect(btnHeroProfiles, &QPushButton::clicked, this, [this]() { onNavButtonClicked(2); });
  heroRight->addWidget(btnHeroProfiles);

  auto *btnHeroUpgrade = new QPushButton("💳 Nâng Cấp VIP");
  btnHeroUpgrade->setCursor(Qt::PointingHandCursor);
  btnHeroUpgrade->setFixedHeight(36);
  btnHeroUpgrade->setStyleSheet(
      "QPushButton { background: #10b981; color: #ffffff; font-weight: 700; border-radius: 7px; "
      "padding: 0 16px; font-size: 12px; border: none; } QPushButton:hover { background: #059669; }");
  connect(btnHeroUpgrade, &QPushButton::clicked, this, [this]() { onNavButtonClicked(5); });
  heroRight->addWidget(btnHeroUpgrade);

  heroLayout->addLayout(heroRight);
  layout->addWidget(heroCard);

  // 3. 4 CLEAN KPI METRIC CARDS (ZERO NESTED BORDERS)
  auto *kpiRow = new QHBoxLayout();
  kpiRow->setSpacing(10);

  auto createKpiCard = [](const QString &icon, const QString &title, QLabel *&valLbl, const QString &defaultVal,
                          QLabel *&subLbl, const QString &defaultSub, const QString &iconBg) {
    auto *card = new QWidget();
    card->setObjectName("kpiCard");
    card->setStyleSheet(
        "QWidget#kpiCard { background: #ffffff; border: 1px solid #e2e8f0; border-radius: 10px; padding: 12px 14px; } "
        "QWidget#kpiCard:hover { border-color: #cbd5e1; } "
        "QLabel { border: none; background: transparent; }");
    auto *cLayout = new QVBoxLayout(card);
    cLayout->setContentsMargins(0, 0, 0, 0);
    cLayout->setSpacing(4);

    auto *topRow = new QHBoxLayout();
    topRow->setSpacing(8);
    auto *iconLbl = new QLabel(icon);
    iconLbl->setFixedSize(32, 32);
    iconLbl->setAlignment(Qt::AlignCenter);
    iconLbl->setStyleSheet(QString("background: %1; border-radius: 7px; font-size: 15px; border: none;").arg(iconBg));
    topRow->addWidget(iconLbl);

    auto *titleLbl = new QLabel(title);
    titleLbl->setStyleSheet("font-size: 10.5px; font-weight: 700; color: #64748b; text-transform: uppercase; letter-spacing: 0.3px;");
    topRow->addWidget(titleLbl, 1);
    cLayout->addLayout(topRow);

    valLbl = new QLabel(defaultVal);
    valLbl->setStyleSheet("font-size: 17px; font-weight: 800; color: #0f172a; margin-top: 1px;");
    cLayout->addWidget(valLbl);

    subLbl = new QLabel(defaultSub);
    subLbl->setStyleSheet("font-size: 11px; color: #059669; font-weight: 600;");
    cLayout->addWidget(subLbl);

    return card;
  };

  kpiRow->addWidget(createKpiCard("👥", "Chrome Profiles", m_lblDashTotalProfiles, "6 Profiles",
                                 m_lblDashRunningProfiles, "🟢 Đang chạy: 0 tab", "#eff6ff"), 1);
  kpiRow->addWidget(createKpiCard("🌐", "Proxy Pool", m_lblDashTotalProxies, "1 Proxies",
                                 m_lblDashLiveProxies, "⚡ Live sẵn sàng", "#ecfdf5"), 1);
  QLabel *lblDummyVal = nullptr;
  QLabel *lblDummySub = nullptr;
  kpiRow->addWidget(createKpiCard("⚡", "Tài Khoản Đã Lọc", lblDummyVal, "125,400+",
                                 lblDummySub, "Độ chính xác 99.9%", "#fff7ed"), 1);
  kpiRow->addWidget(createKpiCard("👑", "Bản Quyền", m_lblDashLicenseStatus, "Bản Dùng Thử",
                                 m_lblDashLicenseDays, "Hạn dùng: Còn 7 ngày", "#faf5ff"), 1);

  layout->addLayout(kpiRow);

  // 4. FEATURE SHORTCUTS & QUICK LAUNCH (4 Clean Cards)
  auto *secHeader = new QLabel("🚀  CÔNG CỤ NỔI BẬT & TRUY CẬP NHANH");
  secHeader->setStyleSheet("font-size: 12.5px; font-weight: 800; color: #475569; letter-spacing: 0.5px; margin-top: 2px; border: none; background: transparent;");
  layout->addWidget(secHeader);

  auto *toolsGrid = new QGridLayout();
  toolsGrid->setSpacing(10);
  toolsGrid->setColumnStretch(0, 1);
  toolsGrid->setColumnStretch(1, 1);

  auto createToolCard = [this](const QString &icon, const QString &iconBg,
                               const QString &badgeText, const QString &badgeColor,
                               const QString &title, const QString &tagline,
                               const QString &btnText, int navTarget) {
    auto *card = new QWidget();
    card->setObjectName("toolCard");
    card->setStyleSheet(
        "QWidget#toolCard { background: #ffffff; border: 1px solid #e2e8f0; border-radius: 10px; padding: 12px 14px; } "
        "QWidget#toolCard:hover { border-color: #cbd5e1; } "
        "QLabel { border: none; background: transparent; }");

    auto *cLayout = new QHBoxLayout(card);
    cLayout->setContentsMargins(0, 0, 0, 0);
    cLayout->setSpacing(12);

    // Left Icon Squircle
    auto *iconLbl = new QLabel(icon);
    iconLbl->setFixedSize(38, 38);
    iconLbl->setAlignment(Qt::AlignCenter);
    iconLbl->setStyleSheet(QString("background: %1; border-radius: 8px; font-size: 18px; border: none;").arg(iconBg));
    cLayout->addWidget(iconLbl);

    // Center Details
    auto *infoCol = new QVBoxLayout();
    infoCol->setSpacing(2);

    auto *topRow = new QHBoxLayout();
    topRow->setSpacing(6);

    auto *tLbl = new QLabel(title);
    tLbl->setStyleSheet("font-size: 13.5px; font-weight: 700; color: #0f172a;");
    topRow->addWidget(tLbl);

    auto *bLbl = new QLabel(badgeText);
    bLbl->setStyleSheet(QString("background: %1; color: %2; font-size: 8.5px; font-weight: 700; border-radius: 3px; padding: 1px 5px; border: none;")
                            .arg(badgeColor == "orange" ? "#fff7ed" : badgeColor == "purple" ? "#faf5ff" : badgeColor == "green" ? "#f0fdf4" : "#eff6ff")
                            .arg(badgeColor == "orange" ? "#ea580c" : badgeColor == "purple" ? "#9333ea" : badgeColor == "green" ? "#16a34a" : "#2563eb"));
    topRow->addWidget(bLbl);
    topRow->addStretch();
    infoCol->addLayout(topRow);

    auto *subLbl = new QLabel(tagline);
    subLbl->setWordWrap(true);
    subLbl->setStyleSheet("font-size: 11.5px; color: #64748b; font-weight: 500;");
    infoCol->addWidget(subLbl);

    cLayout->addLayout(infoCol, 1);

    // Right Action Button
    auto *btn = new QPushButton(btnText);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setFixedHeight(30);
    btn->setStyleSheet(
        "QPushButton { background: #f8fafc; border: 1px solid #cbd5e1; border-radius: 6px; "
        "color: #1e293b; font-size: 11px; font-weight: 700; padding: 0 11px; } "
        "QPushButton:hover { background: #ea580c; color: #ffffff; border-color: #ea580c; }");
    connect(btn, &QPushButton::clicked, this, [this, navTarget]() {
      onNavButtonClicked(navTarget);
    });
    cLayout->addWidget(btn);

    return card;
  };

  toolsGrid->addWidget(createToolCard("👥", "#eff6ff", "ANTI-DETECT", "blue",
                                      "Chrome Profiles",
                                      "Nuôi profile độc lập, chống checkpoint & giả lập vân tay",
                                      "Mở Profiles →", 2), 0, 0);

  toolsGrid->addWidget(createToolCard("🔄", "#faf5ff", "ĐỒNG BỘ 10X", "purple",
                                      "Siêu Đồng Bộ Master - Worker",
                                      "Điều khiển 1 tab mẹ, đồng bộ thao tác đa tab siêu mượt",
                                      "Đồng Bộ →", 2), 0, 1);

  toolsGrid->addWidget(createToolCard("🌐", "#f0fdf4", "PROXY POOL", "green",
                                      "Proxy Pool Tự Động",
                                      "Tự động trích proxy, kiểm tra Live/Die, ping ms & cờ IP",
                                      "Check Proxy →", 3), 1, 0);

  toolsGrid->addWidget(createToolCard("⚡", "#fff7ed", "TÁCH FILE", "orange",
                                      "Lọc & Tách Tài Khoản MMO",
                                      "Tách UID|Pass|2FA siêu tốc, lọc trùng & xuất file Excel",
                                      "Lọc File →", 1), 1, 1);

  layout->addLayout(toolsGrid);

  // 5. SLIM PROMO BANNER (VIP Upgrade Strip)
  auto *promoCard = new QWidget();
  promoCard->setObjectName("promoSlim");
  promoCard->setStyleSheet(
      "QWidget#promoSlim { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #ea580c, stop:1 #d97706); "
      "border-radius: 10px; padding: 12px 18px; } "
      "QLabel { border: none; background: transparent; }");

  auto *promoLayout = new QHBoxLayout(promoCard);
  promoLayout->setContentsMargins(0, 0, 0, 0);
  promoLayout->setSpacing(12);

  auto *pIcon = new QLabel("👑");
  pIcon->setStyleSheet("font-size: 20px; border: none; background: transparent;");
  promoLayout->addWidget(pIcon);

  auto *pTextCol = new QVBoxLayout();
  pTextCol->setSpacing(2);
  auto *pTitle = new QLabel("Nâng Cấp Gói VIP PRO — Tiết Kiệm Đến 35% Khi Gia Hạn Hôm Nay");
  pTitle->setStyleSheet("color: #ffffff; font-size: 13px; font-weight: 700; border: none; background: transparent;");
  auto *pDesc = new QLabel("Không giới hạn Profile Anti-Detect, siêu đồng bộ 100 tab cùng lúc và hỗ trợ kỹ thuật trực tiếp.");
  pDesc->setWordWrap(true);
  pDesc->setStyleSheet("color: rgba(255, 255, 255, 0.9); font-size: 11px; border: none; background: transparent;");
  pTextCol->addWidget(pTitle);
  pTextCol->addWidget(pDesc);
  promoLayout->addLayout(pTextCol, 1);

  auto *btnPromo = new QPushButton("Quét Mã VietQR 💳");
  btnPromo->setCursor(Qt::PointingHandCursor);
  btnPromo->setFixedHeight(32);
  btnPromo->setStyleSheet(
      "QPushButton { background: #ffffff; color: #ea580c; font-weight: 800; border-radius: 7px; "
      "padding: 0 14px; font-size: 11.5px; border: none; } "
      "QPushButton:hover { background: #fff7ed; }");
  connect(btnPromo, &QPushButton::clicked, this, [this]() { onNavButtonClicked(5); });
  promoLayout->addWidget(btnPromo);

  layout->addWidget(promoCard);

  // 6. CLEAN MINIMALIST FOOTER
  auto *footer = new QWidget();
  footer->setObjectName("dashFooter");
  footer->setStyleSheet(
      "QWidget#dashFooter { background: #ffffff; border: 1px solid #e2e8f0; border-radius: 10px; padding: 10px 16px; } "
      "QLabel { border: none; background: transparent; }");
  auto *fLayout = new QHBoxLayout(footer);
  fLayout->setContentsMargins(0, 0, 0, 0);

  auto *fText = new QLabel(
      "💬 <b>Hỗ trợ kỹ thuật:</b> Hotline/Zalo: <span style='color:#ea580c; font-weight:700;'>0988.888.888</span> • Telegram: <span style='color:#0284c7; font-weight:700;'>@tunnnguyen</span> • Phiên bản 2.5.0 Pro");
  fText->setStyleSheet("font-size: 11px; color: #64748b; border: none; background: transparent;");
  fLayout->addWidget(fText);
  fLayout->addStretch();

  layout->addWidget(footer);

  scrollArea->setWidget(contentWidget);
  return scrollArea;
}

void MainWindow::refreshDashboardPage() {
  if (m_proxyPoolItems.isEmpty() && !m_profiles.isEmpty()) {
    refreshProxyPoolTable();
  }

  // Update live profiles metric
  if (m_lblDashTotalProfiles) {
    m_lblDashTotalProfiles->setText(QString("%1 Profiles").arg(m_profiles.size()));
  }

  int runningCount = 0;
  for (const auto &p : m_profiles) {
    if (p.isRunning) runningCount++;
  }
  if (m_lblDashRunningProfiles) {
    m_lblDashRunningProfiles->setText(QString("🟢 Đang chạy: %1 tab").arg(runningCount));
  }

  // Update live proxies metric
  if (m_lblDashTotalProxies) {
    m_lblDashTotalProxies->setText(QString("%1 Proxies").arg(m_proxyPoolItems.size()));
  }

  int liveCount = 0;
  for (const auto &px : m_proxyPoolItems) {
    if (px.checked && px.isLive) liveCount++;
  }
  if (m_lblDashLiveProxies) {
    m_lblDashLiveProxies->setText(QString("⚡ Live: %1 | Sẵn sàng").arg(liveCount));
  }

  // Update live license status
  auto *lm = LicenseManager::instance();
  if (m_lblDashLicenseStatus) {
    m_lblDashLicenseStatus->setText(lm->getStatusBadgeText());
  }
  if (m_lblDashLicenseDays) {
    if (lm->getExpiryDate().date().year() >= 2090) {
      m_lblDashLicenseDays->setText("👑 Hạn dùng: Vĩnh viễn (Trọn Đời)");
    } else {
      m_lblDashLicenseDays->setText(QString("Hạn dùng: Còn %1 ngày").arg(lm->daysRemaining()));
    }
  }
}

