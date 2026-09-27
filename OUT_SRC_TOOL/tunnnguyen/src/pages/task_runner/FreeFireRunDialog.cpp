#include "FreeFireRunDialog.h"
#include "FreeFireLogic.h"
#include "ui/CustomMessageBox.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QProcess>
#include <QFileInfo>
#include <QFileDialog>
#include <QTextStream>
#include <QDateTime>
#include <QScrollBar>
#include <QHeaderView>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

FreeFireRunDialog::FreeFireRunDialog(const QList<ChromeProfileItem> &profiles, QWidget *parent)
    : QDialog(parent), m_profiles(profiles)
{
    reloadProfilesFromDisk();

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &FreeFireRunDialog::executeNextAccount);
    setupUi();
}

FreeFireRunDialog::~FreeFireRunDialog()
{
    if (m_timer) {
        m_timer->stop();
    }
}

void FreeFireRunDialog::reloadProfilesFromDisk()
{
    QString jsonPath = QDir::homePath() + "/.tunnbit_profiles/profiles.json";
    QFile f(jsonPath);
    if (f.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
        f.close();
        if (doc.isArray()) {
            m_profiles.clear();
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
                item.isSelected = true;
                m_profiles.append(item);
            }
        }
    }
}

void FreeFireRunDialog::setupUi()
{
    setWindowTitle(QString::fromUtf8("Cấu Hình Khởi Chạy Kịch Bản: Auto Change Info Free Fire"));
    setFixedSize(980, 720);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    setStyleSheet(
        "* { font-family: 'Google Sans', 'Product Sans', -apple-system, sans-serif; }"
        "QDialog { background-color: #ffffff; border-radius: 16px; }"
        "QTableWidget { background-color: #ffffff; border: 1.5px solid #e2e8f0; border-radius: 10px; gridline-color: #f1f5f9; }"
        "QHeaderView::section { background-color: #f8fafc; color: #475569; font-size: 12px; font-weight: 700; border: none; border-bottom: 1.5px solid #e2e8f0; padding: 6px; }"
        "QComboBox, QSpinBox, QLineEdit {"
        "   background-color: #f8fafc;"
        "   border: 1.5px solid #cbd5e1;"
        "   border-radius: 7px;"
        "   padding: 6px 10px;"
        "   font-size: 12.5px;"
        "   font-weight: 600;"
        "   color: #0f172a;"
        "}"
        "QComboBox:focus, QSpinBox:focus, QLineEdit:focus { border: 1.5px solid #ea580c; background: #ffffff; }"
        "QComboBox::drop-down { border: none; }"
        "QCheckBox { font-size: 12px; font-weight: 600; color: #334155; }"
    );

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 16, 20, 16);
    mainLayout->setSpacing(0);

    m_stack = new QStackedWidget(this);
    m_stack->addWidget(createConfigPage());
    m_stack->addWidget(createExecutionPage());
    mainLayout->addWidget(m_stack);

    populateProfileTable();
    onScanAdbClicked();
}

QWidget* FreeFireRunDialog::createConfigPage()
{
    auto *w = new QWidget();
    auto *lay = new QVBoxLayout(w);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(12);

    // Header Title
    auto *headRow = new QHBoxLayout();
    headRow->setSpacing(12);
    auto *icon = new QLabel(QString::fromUtf8("🎮"));
    icon->setStyleSheet("font-size: 32px; background: #fff7ed; border: 1px solid #ffedd5; border-radius: 12px; padding: 6px;");

    auto *tBox = new QVBoxLayout();
    tBox->setSpacing(2);
    auto *title = new QLabel(QString::fromUtf8("Cấu Hình Khởi Chạy Kịch Bản: Auto Change Info Free Fire"));
    title->setStyleSheet("font-size: 17px; font-weight: 800; color: #0f172a;");
    auto *sub = new QLabel(QString::fromUtf8("Chạy tuần tự trên 1 Máy ảo Android duy nhất kết hợp luân phiên các Chrome Profiles"));
    sub->setStyleSheet("font-size: 12px; color: #64748b; font-weight: 600;");
    tBox->addWidget(title);
    tBox->addWidget(sub);
    headRow->addWidget(icon);
    headRow->addLayout(tBox, 1);

    auto *badge = new QLabel(QString::fromUtf8("⚡ 1 ANDROID + MULTI-CHROME"));
    badge->setStyleSheet("background: #ffedd5; color: #c2410c; font-size: 11px; font-weight: 800; border-radius: 6px; padding: 6px 12px; border: 1px solid #fed7aa;");
    headRow->addWidget(badge);
    lay->addLayout(headRow);

    // Card 1: Android Device & Chrome Profiles
    auto *cardTop = new QFrame();
    cardTop->setStyleSheet("QFrame { background: #f8fafc; border: 1px solid #e2e8f0; border-radius: 12px; } QLabel { border: none; background: transparent; }");
    auto *lTop = new QVBoxLayout(cardTop);
    lTop->setContentsMargins(14, 12, 14, 12);
    lTop->setSpacing(10);

    // 1. Android Device selector row
    auto *rAdb = new QHBoxLayout();
    rAdb->setSpacing(10);
    auto *lblAdbTitle = new QLabel(QString::fromUtf8("📱 1. Máy Ảo Android Chạy Game:"));
    lblAdbTitle->setStyleSheet("font-size: 13px; font-weight: 800; color: #0f172a;");
    rAdb->addWidget(lblAdbTitle);

    m_cmbAdb = new QComboBox();
    m_cmbAdb->addItem(QString::fromUtf8("📱 127.0.0.1:5555 (LDPlayer 9 - Mặc định)"), "127.0.0.1:5555");
    m_cmbAdb->addItem(QString::fromUtf8("📱 127.0.0.1:5557 (LDPlayer 1)"), "127.0.0.1:5557");
    m_cmbAdb->addItem(QString::fromUtf8("📱 127.0.0.1:5559 (LDPlayer 2)"), "127.0.0.1:5559");
    m_cmbAdb->addItem(QString::fromUtf8("📱 emulator-5554 (BlueStacks/LD)"), "emulator-5554");
    m_cmbAdb->setMinimumWidth(260);
    rAdb->addWidget(m_cmbAdb);

    m_btnScanAdb = new QPushButton(QString::fromUtf8("🔄 Quét ADB"));
    m_btnScanAdb->setCursor(Qt::PointingHandCursor);
    m_btnScanAdb->setStyleSheet("background: #eff6ff; color: #1d4ed8; font-size: 11.5px; font-weight: 700; border: 1px solid #bfdbfe; border-radius: 6px; padding: 5px 12px;");
    connect(m_btnScanAdb, &QPushButton::clicked, this, &FreeFireRunDialog::onScanAdbClicked);
    rAdb->addWidget(m_btnScanAdb);

    m_lblAdbStatus = new QLabel(QString::fromUtf8("Đang kiểm tra ADB..."));
    m_lblAdbStatus->setStyleSheet("font-size: 12px; color: #64748b; font-weight: 600;");
    rAdb->addWidget(m_lblAdbStatus);
    rAdb->addStretch();
    lTop->addLayout(rAdb);

    // 2. Chrome Profiles Table
    auto *rChrHead = new QHBoxLayout();
    auto *lblChrTitle = new QLabel(QString::fromUtf8("🌐 2. Danh Sách Chrome Profiles Hứng OTP (Đồng bộ với Chrome Profiles)"));
    lblChrTitle->setStyleSheet("font-size: 13px; font-weight: 800; color: #0f172a;");
    rChrHead->addWidget(lblChrTitle);
    rChrHead->addStretch();

    m_btnSyncChrome = new QPushButton(QString::fromUtf8("🔄 Đồng Bộ Chrome Profiles"));
    m_btnSyncChrome->setCursor(Qt::PointingHandCursor);
    m_btnSyncChrome->setStyleSheet("background: #f0fdf4; color: #15803d; font-size: 11px; font-weight: 700; border: 1px solid #bbf7d0; border-radius: 6px; padding: 4px 10px;");
    connect(m_btnSyncChrome, &QPushButton::clicked, this, &FreeFireRunDialog::onRefreshProfilesClicked);
    rChrHead->addWidget(m_btnSyncChrome);
    lTop->addLayout(rChrHead);

    m_tableProfiles = new QTableWidget(0, 5, this);
    m_tableProfiles->setHorizontalHeaderLabels({
        QString::fromUtf8("Chọn"),
        QString::fromUtf8("Tên Profile Chrome"),
        QString::fromUtf8("Remote Port (CDP)"),
        QString::fromUtf8("Proxy Đi Kèm"),
        QString::fromUtf8("Trạng Thái")
    });
    m_tableProfiles->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    m_tableProfiles->setColumnWidth(0, 55);
    m_tableProfiles->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_tableProfiles->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_tableProfiles->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    m_tableProfiles->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    m_tableProfiles->verticalHeader()->setVisible(false);
    m_tableProfiles->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableProfiles->setFixedHeight(150);
    lTop->addWidget(m_tableProfiles);

    auto *rChrFoot = new QHBoxLayout();
    m_chkSelectAllProfiles = new QCheckBox(QString::fromUtf8("Chọn tất cả Chrome Profiles"));
    m_chkSelectAllProfiles->setChecked(true);
    connect(m_chkSelectAllProfiles, &QCheckBox::toggled, this, &FreeFireRunDialog::onSelectAllProfiles);

    m_chkResetDevice = new QCheckBox(QString::fromUtf8("Tự động Đăng xuất (pm clear) & Fake Device ID sau mỗi Nick"));
    m_chkResetDevice->setChecked(true);

    rChrFoot->addWidget(m_chkSelectAllProfiles);
    rChrFoot->addSpacing(18);
    rChrFoot->addWidget(m_chkResetDevice);
    rChrFoot->addStretch();
    lTop->addLayout(rChrFoot);

    lay->addWidget(cardTop);

    // Row Bottom: Accounts File & Options
    auto *rowBottom = new QHBoxLayout();
    rowBottom->setSpacing(12);

    // Accounts File Card (Chỉ cho phép nhập File)
    auto *cardAcc = new QFrame();
    cardAcc->setStyleSheet("QFrame { background: #f8fafc; border: 1px solid #e2e8f0; border-radius: 12px; } QLabel { border: none; background: transparent; }");
    auto *lAcc = new QVBoxLayout(cardAcc);
    lAcc->setContentsMargins(16, 14, 16, 14);
    lAcc->setSpacing(10);

    auto *lblFileSection = new QLabel(QString::fromUtf8("📂 3. Nhập File Danh Sách Tài Khoản"));
    lblFileSection->setStyleSheet("font-size: 13px; font-weight: 800; color: #0f172a;");
    lAcc->addWidget(lblFileSection);

    auto *lblFormatHint = new QLabel(QString::fromUtf8("Định dạng file (.txt hoặc .csv):\nUID|MậtKhẩuCũ|MậtKhẩuMới|EmailBảoMậtMới"));
    lblFormatHint->setStyleSheet("font-size: 11.5px; color: #64748b; font-weight: 600; line-height: 1.4;");
    lAcc->addWidget(lblFormatHint);

    auto *rFilePick = new QHBoxLayout();
    rFilePick->setSpacing(8);

    m_txtFilePath = new QLineEdit();
    m_txtFilePath->setReadOnly(true);
    m_txtFilePath->setPlaceholderText(QString::fromUtf8("Chưa chọn file (bấm 'Chọn File' để duyệt file .txt / .csv)..."));
    rFilePick->addWidget(m_txtFilePath, 1);

    m_btnBrowseFile = new QPushButton(QString::fromUtf8("📂 Chọn File"));
    m_btnBrowseFile->setCursor(Qt::PointingHandCursor);
    m_btnBrowseFile->setFixedHeight(36);
    m_btnBrowseFile->setStyleSheet(
        "QPushButton { background: #ea580c; color: #ffffff; font-size: 12px; font-weight: 700; border-radius: 7px; border: none; padding: 0 16px; }"
        "QPushButton:hover { background: #c2410c; }");
    connect(m_btnBrowseFile, &QPushButton::clicked, this, &FreeFireRunDialog::onBrowseFileClicked);
    rFilePick->addWidget(m_btnBrowseFile);

    m_btnClearFile = new QPushButton(QString::fromUtf8("🗑 Bỏ File"));
    m_btnClearFile->setCursor(Qt::PointingHandCursor);
    m_btnClearFile->setFixedHeight(36);
    m_btnClearFile->setStyleSheet(
        "QPushButton { background: #fee2e2; color: #b91c1c; font-size: 12px; font-weight: 700; border-radius: 7px; border: 1px solid #fecaca; padding: 0 12px; }"
        "QPushButton:hover { background: #fecaca; }");
    connect(m_btnClearFile, &QPushButton::clicked, this, &FreeFireRunDialog::onClearFileClicked);
    rFilePick->addWidget(m_btnClearFile);
    lAcc->addLayout(rFilePick);

    // Status box displaying file info
    m_lblFileInfo = new QLabel(QString::fromUtf8("⚠️ Chưa có file nào được nạp. Vui lòng bấm 'Chọn File' để tải danh sách nick."));
    m_lblFileInfo->setStyleSheet("background: #f1f5f9; color: #64748b; font-size: 12px; font-weight: 600; border-radius: 8px; padding: 10px 12px; border: 1px dashed #cbd5e1;");
    m_lblFileInfo->setWordWrap(true);
    lAcc->addWidget(m_lblFileInfo);
    lAcc->addStretch();

    rowBottom->addWidget(cardAcc, 58);

    // Options Card
    auto *cardOpt = new QFrame();
    cardOpt->setStyleSheet("QFrame { background: #f8fafc; border: 1px solid #e2e8f0; border-radius: 12px; } QLabel { border: none; background: transparent; }");
    auto *lOpt = new QVBoxLayout(cardOpt);
    lOpt->setContentsMargins(16, 14, 16, 14);
    lOpt->setSpacing(10);

    auto *lblOptTitle = new QLabel(QString::fromUtf8("⚙️ 4. Tùy Chọn & Điều Khiển"));
    lblOptTitle->setStyleSheet("font-size: 13px; font-weight: 800; color: #0f172a;");
    lOpt->addWidget(lblOptTitle);

    m_chkChangePassword = new QCheckBox(QString::fromUtf8("Tự động đổi Mật khẩu mới"));
    m_chkChangePassword->setChecked(true);
    m_chkChangeEmail = new QCheckBox(QString::fromUtf8("Liên kết Email bảo mật mới"));
    m_chkChangeEmail->setChecked(true);
    m_chkRemovePhone = new QCheckBox(QString::fromUtf8("Gỡ số điện thoại bảo mật cũ"));
    m_chkRemovePhone->setChecked(true);
    m_chkExportResult = new QCheckBox(QString::fromUtf8("Xuất file kết quả sau khi xong"));
    m_chkExportResult->setChecked(true);

    lOpt->addWidget(m_chkChangePassword);
    lOpt->addWidget(m_chkChangeEmail);
    lOpt->addWidget(m_chkRemovePhone);
    lOpt->addWidget(m_chkExportResult);

    auto *rDelay = new QHBoxLayout();
    auto *lblDelay = new QLabel(QString::fromUtf8("Độ trễ thao tác (giây):"));
    lblDelay->setStyleSheet("font-size: 12px; color: #475569; font-weight: 600;");
    m_spinDelay = new QSpinBox();
    m_spinDelay->setRange(1, 15);
    m_spinDelay->setValue(2);
    rDelay->addWidget(lblDelay);
    rDelay->addWidget(m_spinDelay);
    rDelay->addStretch();
    lOpt->addLayout(rDelay);
    lOpt->addStretch();

    rowBottom->addWidget(cardOpt, 42);
    lay->addLayout(rowBottom, 1);

    // Footer buttons
    auto *foot = new QHBoxLayout();
    foot->setSpacing(12);

    m_btnCancel = new QPushButton(QString::fromUtf8("Hủy Bỏ"));
    m_btnCancel->setCursor(Qt::PointingHandCursor);
    m_btnCancel->setFixedHeight(44);
    m_btnCancel->setStyleSheet(
        "QPushButton { background: #f1f5f9; color: #475569; font-size: 13px; font-weight: 700; border-radius: 8px; border: 1px solid #cbd5e1; padding: 0 24px; }"
        "QPushButton:hover { background: #e2e8f0; color: #0f172a; }");
    connect(m_btnCancel, &QPushButton::clicked, this, &QDialog::reject);

    m_btnStart = new QPushButton(QString::fromUtf8("🚀  BẮT ĐẦU CHẠY KỊCH BẢN"));
    m_btnStart->setCursor(Qt::PointingHandCursor);
    m_btnStart->setFixedHeight(44);
    m_btnStart->setStyleSheet(
        "QPushButton { "
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #ea580c, stop:1 #f97316); "
        "  color: #ffffff; font-size: 14px; font-weight: 800; border-radius: 8px; border: none; padding: 0 28px; "
        "} "
        "QPushButton:hover { background: #c2410c; }");
    connect(m_btnStart, &QPushButton::clicked, this, &FreeFireRunDialog::onStartExecution);

    foot->addWidget(m_btnCancel);
    foot->addWidget(m_btnStart, 1);
    lay->addLayout(foot);

    return w;
}

void FreeFireRunDialog::populateProfileTable()
{
    m_tableProfiles->setRowCount(0);

    for (int i = 0; i < m_profiles.size(); ++i) {
        int r = m_tableProfiles->rowCount();
        m_tableProfiles->insertRow(r);

        // Col 0: Checkbox
        auto *chkWidget = new QWidget();
        auto *chkLay = new QHBoxLayout(chkWidget);
        chkLay->setContentsMargins(0, 0, 0, 0);
        chkLay->setAlignment(Qt::AlignCenter);
        auto *chk = new QCheckBox();
        chk->setChecked(m_profiles[i].isSelected);
        connect(chk, &QCheckBox::toggled, this, [this, i](bool checked) {
            if (i >= 0 && i < m_profiles.size()) {
                m_profiles[i].isSelected = checked;
            }
        });
        chkLay->addWidget(chk);
        m_tableProfiles->setCellWidget(r, 0, chkWidget);

        // Col 1: Name
        auto *itemName = new QTableWidgetItem(QString::fromUtf8("👤 %1").arg(m_profiles[i].name));
        itemName->setFont(QFont("Google Sans", 10, QFont::Bold));
        itemName->setForeground(QColor("#0f172a"));
        m_tableProfiles->setItem(r, 1, itemName);

        // Col 2: Remote Port
        auto *itemPort = new QTableWidgetItem(QString("Port %1").arg(m_profiles[i].port));
        itemPort->setTextAlignment(Qt::AlignCenter);
        itemPort->setFont(QFont("Consolas", 10));
        m_tableProfiles->setItem(r, 2, itemPort);

        // Col 3: Proxy
        QString px = m_profiles[i].proxy.trimmed();
        auto *itemProxy = new QTableWidgetItem(px.isEmpty() ? QString::fromUtf8("🌐 Trực tiếp (Direct)") : px);
        itemProxy->setForeground(px.isEmpty() ? QColor("#94a3b8") : QColor("#2563eb"));
        m_tableProfiles->setItem(r, 3, itemProxy);

        // Col 4: Status
        auto *itemStat = new QTableWidgetItem(QString::fromUtf8("🟢 Sẵn sàng"));
        itemStat->setTextAlignment(Qt::AlignCenter);
        itemStat->setForeground(QColor("#16a34a"));
        m_tableProfiles->setItem(r, 4, itemStat);
    }
}

void FreeFireRunDialog::onRefreshProfilesClicked()
{
    reloadProfilesFromDisk();
    populateProfileTable();
}

void FreeFireRunDialog::onScanAdbClicked()
{
    QString adbExe = "adb";
    if (QFileInfo::exists("C:/LDPlayer/LDPlayer9/adb.exe")) {
        adbExe = "C:/LDPlayer/LDPlayer9/adb.exe";
    }

    QProcess proc;
    proc.start(adbExe, QStringList() << "devices");
    if (proc.waitForFinished(2000)) {
        QString out = QString::fromUtf8(proc.readAllStandardOutput());
        QStringList lines = out.split("\n", Qt::SkipEmptyParts);
        QStringList found;
        for (const QString &l : lines) {
            if (l.contains("device") && !l.startsWith("List")) {
                found.append(l.split("\t").value(0).trimmed());
            }
        }

        if (!found.isEmpty()) {
            m_cmbAdb->clear();
            for (const QString &d : found) {
                m_cmbAdb->addItem(QString("📱 %1 (Online)").arg(d), d);
            }
            m_lblAdbStatus->setText(QString::fromUtf8("🟢 Đang kết nối: %1").arg(found.first()));
            m_lblAdbStatus->setStyleSheet("font-size: 12px; color: #16a34a; font-weight: 700;");
        } else {
            m_lblAdbStatus->setText(QString::fromUtf8("⚠️ Chưa tìm thấy máy ảo nào online (Hãy mở LDPlayer)"));
            m_lblAdbStatus->setStyleSheet("font-size: 12px; color: #dc2626; font-weight: 600;");
        }
    }
}

void FreeFireRunDialog::onSelectAllProfiles(bool checked)
{
    for (int r = 0; r < m_tableProfiles->rowCount(); ++r) {
        auto *w = m_tableProfiles->cellWidget(r, 0);
        if (w) {
            auto *chk = w->findChild<QCheckBox*>();
            if (chk) chk->setChecked(checked);
        }
    }
    for (auto &p : m_profiles) {
        p.isSelected = checked;
    }
}

void FreeFireRunDialog::onBrowseFileClicked()
{
    QString path = QFileDialog::getOpenFileName(this, QString::fromUtf8("Chọn file danh sách tài khoản"), "", "Text Files (*.txt *.csv);;All Files (*.*)");
    if (path.isEmpty()) return;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        CustomMessageBox::warning(this, QString::fromUtf8("Lỗi"), QString::fromUtf8("Không thể đọc file đã chọn!"));
        return;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    QStringList lines = content.split("\n", Qt::SkipEmptyParts);
    QStringList validAccs;
    for (const QString &line : lines) {
        QString trimmed = line.trimmed();
        if (!trimmed.isEmpty()) {
            validAccs.append(trimmed);
        }
    }

    if (validAccs.isEmpty()) {
        CustomMessageBox::warning(this, QString::fromUtf8("Thông báo"), QString::fromUtf8("File được chọn không chứa dữ liệu tài khoản hợp lệ!"));
        return;
    }

    m_selectedFilePath = path;
    m_accounts = validAccs;
    m_txtFilePath->setText(path);

    QFileInfo fi(path);
    m_lblFileInfo->setText(QString::fromUtf8("✅ Đã nạp thành công %1 tài khoản từ: %2 (%3 KB)")
        .arg(validAccs.size())
        .arg(fi.fileName())
        .arg(fi.size() / 1024 + 1));
    m_lblFileInfo->setStyleSheet("background: #f0fdf4; color: #15803d; font-size: 12px; font-weight: 700; border-radius: 8px; padding: 10px 12px; border: 1px solid #bbf7d0;");
}

void FreeFireRunDialog::onClearFileClicked()
{
    m_selectedFilePath.clear();
    m_accounts.clear();
    if (m_txtFilePath) m_txtFilePath->clear();
    if (m_lblFileInfo) {
        m_lblFileInfo->setText(QString::fromUtf8("⚠️ Chưa có file nào được nạp. Vui lòng bấm 'Chọn File' để tải danh sách nick."));
        m_lblFileInfo->setStyleSheet("background: #f1f5f9; color: #64748b; font-size: 12px; font-weight: 600; border-radius: 8px; padding: 10px 12px; border: 1px dashed #cbd5e1;");
    }
}

QWidget* FreeFireRunDialog::createExecutionPage()
{
    auto *w = new QWidget();
    auto *lay = new QVBoxLayout(w);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(12);

    auto *headRow = new QHBoxLayout();
    headRow->setSpacing(10);
    auto *icon = new QLabel(QString::fromUtf8("⚡"));
    icon->setStyleSheet("font-size: 28px; background: #fff7ed; border: 1px solid #ffedd5; border-radius: 10px; padding: 4px;");

    auto *tBox = new QVBoxLayout();
    tBox->setSpacing(2);
    auto *title = new QLabel(QString::fromUtf8("Đang Thực Thi Kịch Bản: Auto Change Info Free Fire"));
    title->setStyleSheet("font-size: 16px; font-weight: 800; color: #0f172a;");
    auto *sub = new QLabel(QString::fromUtf8("Chạy tuần tự trên Máy ảo Android kết hợp luân phiên Profile Chrome"));
    sub->setStyleSheet("font-size: 11.5px; color: #64748b; font-weight: 600;");
    tBox->addWidget(title);
    tBox->addWidget(sub);

    headRow->addWidget(icon);
    headRow->addLayout(tBox, 1);

    m_lblStatusBadge = new QLabel(QString::fromUtf8("⚡ ĐANG CHẠY KỊCH BẢN..."));
    m_lblStatusBadge->setStyleSheet("background: #dcfce7; color: #15803d; font-size: 11px; font-weight: 700; border-radius: 6px; padding: 5px 12px; border: 1px solid #bbf7d0;");
    headRow->addWidget(m_lblStatusBadge);
    lay->addLayout(headRow);

    auto *kpiGrid = new QGridLayout();
    kpiGrid->setSpacing(8);

    auto makeMetric = [](const QString &lbl, const QString &val, const QString &valColor, const QString &bg) {
        auto *box = new QFrame();
        box->setStyleSheet(QString("QFrame { background: %1; border-radius: 8px; border: 1px solid #e2e8f0; } QLabel { border: none; background: transparent; }").arg(bg));
        auto *l = new QVBoxLayout(box);
        l->setContentsMargins(8, 8, 8, 8);
        l->setSpacing(2);
        auto *v = new QLabel(val);
        v->setStyleSheet(QString("font-size: 18px; font-weight: 800; color: %1;").arg(valColor));
        v->setAlignment(Qt::AlignCenter);
        auto *t = new QLabel(lbl);
        t->setStyleSheet("font-size: 10px; font-weight: 700; color: #64748b; text-transform: uppercase;");
        t->setAlignment(Qt::AlignCenter);
        l->addWidget(v);
        l->addWidget(t);
        return qMakePair(box, v);
    };

    auto b1 = makeMetric(QString::fromUtf8("Tổng Tài Khoản"), "0", "#0f172a", "#f8fafc");
    m_lblTotal = b1.second;
    auto b2 = makeMetric(QString::fromUtf8("Đang Xử Lý"), "0", "#2563eb", "#eff6ff");
    m_lblProcessing = b2.second;
    auto b3 = makeMetric(QString::fromUtf8("Thành Công"), "0", "#16a34a", "#f0fdf4");
    m_lblSuccess = b3.second;
    auto b4 = makeMetric(QString::fromUtf8("Thất Bại"), "0", "#dc2626", "#fef2f2");
    m_lblFailed = b4.second;

    kpiGrid->addWidget(b1.first, 0, 0);
    kpiGrid->addWidget(b2.first, 0, 1);
    kpiGrid->addWidget(b3.first, 0, 2);
    kpiGrid->addWidget(b4.first, 0, 3);
    lay->addLayout(kpiGrid);

    m_progressBar = new QProgressBar();
    m_progressBar->setFixedHeight(12);
    m_progressBar->setTextVisible(false);
    m_progressBar->setStyleSheet(
        "QProgressBar { background: #f1f5f9; border-radius: 6px; border: none; } "
        "QProgressBar::chunk { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #ea580c, stop:1 #f97316); border-radius: 6px; }");
    lay->addWidget(m_progressBar);

    auto *logCard = new QFrame();
    logCard->setStyleSheet("QFrame { background: #0f172a; border-radius: 12px; } QLabel { border: none; background: transparent; }");
    auto *lLog = new QVBoxLayout(logCard);
    lLog->setContentsMargins(14, 10, 14, 10);
    lLog->setSpacing(6);

    auto *logHead = new QHBoxLayout();
    auto *lblConsole = new QLabel(QString::fromUtf8("💻 Console Realtime Logs (Single Android + Multi-Chrome)"));
    lblConsole->setStyleSheet("color: #94a3b8; font-size: 11px; font-weight: 700;");
    logHead->addWidget(lblConsole);
    logHead->addStretch();

    auto *btnClearLog = new QPushButton(QString::fromUtf8("🧹 Xóa Log"));
    btnClearLog->setCursor(Qt::PointingHandCursor);
    btnClearLog->setStyleSheet("background: #1e293b; color: #cbd5e1; font-size: 10px; font-weight: 600; border: 1px solid #334155; border-radius: 5px; padding: 2px 8px;");
    connect(btnClearLog, &QPushButton::clicked, this, [this]() {
        if (m_txtLog) m_txtLog->clear();
    });
    logHead->addWidget(btnClearLog);
    lLog->addLayout(logHead);

    m_txtLog = new QPlainTextEdit();
    m_txtLog->setReadOnly(true);
    m_txtLog->setStyleSheet(
        "QPlainTextEdit { background: #0b1120; border: none; border-radius: 8px; "
        "color: #e2e8f0; font-family: 'Consolas', 'Courier New', monospace; font-size: 11.5px; padding: 8px; }");
    lLog->addWidget(m_txtLog, 1);
    lay->addWidget(logCard, 1);

    auto *footLive = new QHBoxLayout();
    footLive->setSpacing(10);

    m_btnBackConfig = new QPushButton(QString::fromUtf8("← Quay Lại Cấu Hình"));
    m_btnBackConfig->setCursor(Qt::PointingHandCursor);
    m_btnBackConfig->setFixedHeight(42);
    m_btnBackConfig->setStyleSheet(
        "QPushButton { background: #f1f5f9; color: #475569; font-size: 12.5px; font-weight: 700; border-radius: 8px; border: 1px solid #cbd5e1; padding: 0 18px; }"
        "QPushButton:hover { background: #e2e8f0; color: #0f172a; }");
    connect(m_btnBackConfig, &QPushButton::clicked, this, &FreeFireRunDialog::onBackToConfig);

    m_btnStopRun = new QPushButton(QString::fromUtf8("⏹  DỪNG KỊCH BẢN"));
    m_btnStopRun->setCursor(Qt::PointingHandCursor);
    m_btnStopRun->setFixedHeight(42);
    m_btnStopRun->setStyleSheet(
        "QPushButton { background: #fee2e2; color: #b91c1c; font-size: 12.5px; font-weight: 800; border-radius: 8px; border: 1px solid #fca5a5; padding: 0 20px; }"
        "QPushButton:hover { background: #fecaca; }");
    connect(m_btnStopRun, &QPushButton::clicked, this, &FreeFireRunDialog::onStopExecution);

    m_btnClose = new QPushButton(QString::fromUtf8("Đóng Hộp Thoại"));
    m_btnClose->setCursor(Qt::PointingHandCursor);
    m_btnClose->setFixedHeight(42);
    m_btnClose->setStyleSheet(
        "QPushButton { background: #0f172a; color: #ffffff; font-size: 12.5px; font-weight: 700; border-radius: 8px; border: none; padding: 0 22px; }"
        "QPushButton:hover { background: #1e293b; }");
    connect(m_btnClose, &QPushButton::clicked, this, &QDialog::accept);

    footLive->addWidget(m_btnBackConfig);
    footLive->addWidget(m_btnStopRun, 1);
    footLive->addWidget(m_btnClose);
    lay->addLayout(footLive);

    return w;
}

void FreeFireRunDialog::onStartExecution()
{
    m_selectedProfiles.clear();
    for (int r = 0; r < m_tableProfiles->rowCount(); ++r) {
        auto *w = m_tableProfiles->cellWidget(r, 0);
        bool isSel = false;
        if (w) {
            auto *chk = w->findChild<QCheckBox*>();
            if (chk && chk->isChecked()) isSel = true;
        }

        if (isSel && r < m_profiles.size()) {
            m_selectedProfiles.append(m_profiles[r]);
        }
    }

    if (m_selectedProfiles.isEmpty()) {
        CustomMessageBox::warning(this, QString::fromUtf8("Thông báo"),
                                  QString::fromUtf8("Vui lòng tích chọn ít nhất 1 Profile Chrome trong bảng để hứng OTP!"));
        return;
    }

    if (m_accounts.isEmpty()) {
        CustomMessageBox::warning(this, QString::fromUtf8("Thông báo"),
                                  QString::fromUtf8("Vui lòng bấm 'Chọn File' để nạp file danh sách tài khoản (.txt hoặc .csv) trước khi chạy!"));
        return;
    }

    m_selectedAdbDevice = m_cmbAdb ? m_cmbAdb->currentData().toString() : "127.0.0.1:5555";
    if (m_selectedAdbDevice.isEmpty() && m_cmbAdb) {
        m_selectedAdbDevice = m_cmbAdb->currentText();
    }

    m_stack->setCurrentIndex(1);

    m_isRunning = true;
    m_currentIndex = 0;
    m_successCount = 0;
    m_failCount = 0;

    if (m_lblTotal) m_lblTotal->setText(QString::number(m_accounts.size()));
    if (m_lblProcessing) m_lblProcessing->setText("0");
    if (m_lblSuccess) m_lblSuccess->setText("0");
    if (m_lblFailed) m_lblFailed->setText("0");
    if (m_progressBar) m_progressBar->setValue(0);

    if (m_lblStatusBadge) {
        m_lblStatusBadge->setText(QString::fromUtf8("⚡ ĐANG CHẠY KỊCH BẢN..."));
        m_lblStatusBadge->setStyleSheet("background: #ffedd5; color: #c2410c; font-size: 11px; font-weight: 700; border-radius: 6px; padding: 5px 12px; border: 1px solid #fed7aa;");
    }

    if (m_btnStopRun) m_btnStopRun->setEnabled(true);
    if (m_btnBackConfig) m_btnBackConfig->setEnabled(false);

    if (m_txtLog) m_txtLog->clear();
    logMessage("--------------------------------------------------", "INIT");
    logMessage(QString("BẮT ĐẦU CHẠY: Auto Change Info Free Fire (%1 tài khoản)").arg(m_accounts.size()), "START");
    logMessage(QString("File nguồn: %1").arg(m_selectedFilePath.isEmpty() ? "Danh sách nạp" : m_selectedFilePath), "FILE");
    logMessage(QString("Máy ảo Android sử dụng: [%1]").arg(m_selectedAdbDevice), "DEVICE");
    logMessage(QString("Số Profile Chrome luân phiên: %1 profiles").arg(m_selectedProfiles.size()), "SETUP");
    for (const auto &p : m_selectedProfiles) {
        logMessage(QString("► Profile: [%1] - Port %2 %3").arg(p.name).arg(p.port).arg(p.proxy.isEmpty() ? "" : QString("(Proxy: %1)").arg(p.proxy)), "CHROME");
    }

    int delaySec = m_spinDelay ? m_spinDelay->value() : 2;
    m_timer->start(delaySec * 1000);
}

void FreeFireRunDialog::executeNextAccount()
{
    if (!m_isRunning) return;

    if (m_currentIndex >= m_accounts.size()) {
        m_isRunning = false;
        m_timer->stop();
        if (m_lblProcessing) m_lblProcessing->setText("0");
        if (m_lblStatusBadge) {
            m_lblStatusBadge->setText(QString::fromUtf8("✅ ĐÃ HOÀN THÀNH KỊCH BẢN"));
            m_lblStatusBadge->setStyleSheet("background: #dcfce7; color: #15803d; font-size: 11px; font-weight: 700; border-radius: 6px; padding: 5px 12px; border: 1px solid #bbf7d0;");
        }
        if (m_btnStopRun) m_btnStopRun->setEnabled(false);
        if (m_btnBackConfig) m_btnBackConfig->setEnabled(true);
        logMessage("--------------------------------------------------", "DONE");
        logMessage(QString("KẾT THÚC: Thành công %1 / %2 tài khoản.").arg(m_successCount).arg(m_accounts.size()), "SUCCESS");
        return;
    }

    if (m_lblProcessing) m_lblProcessing->setText(QString::number(m_currentIndex + 1));

    QString line = m_accounts[m_currentIndex].trimmed();
    QStringList parts = line.split("|");

    int pIdx = m_currentIndex % m_selectedProfiles.size();
    const ChromeProfileItem &activeProfile = m_selectedProfiles[pIdx];

    FreeFireTaskConfig cfg;
    cfg.uid = parts.value(0, "UnknownUID");
    cfg.oldPass = parts.value(1, "OldPass");
    cfg.newPass = parts.value(2, "NewPass");
    cfg.newEmail = parts.value(3, "newmail@example.com");
    cfg.adbDevice = m_selectedAdbDevice;
    cfg.chromeProfile = activeProfile.name;
    cfg.changePassword = (m_chkChangePassword && m_chkChangePassword->isChecked());
    cfg.changeEmail = (m_chkChangeEmail && m_chkChangeEmail->isChecked());
    cfg.removePhone = (m_chkRemovePhone && m_chkRemovePhone->isChecked());
    cfg.exportResult = (m_chkExportResult && m_chkExportResult->isChecked());
    cfg.delaySeconds = m_spinDelay ? m_spinDelay->value() : 2;
    cfg.threadCount = 1;

    logMessage(QString("--- [Tài khoản %1/%2: %3] ---").arg(m_currentIndex + 1).arg(m_accounts.size()).arg(cfg.uid), "ACC");
    logMessage(QString("Gán Chrome: [%1] (Port %2) ↔ Máy ảo Android: [%3]").arg(activeProfile.name).arg(activeProfile.port).arg(cfg.adbDevice), "PAIR");

    bool ok = FreeFireLogic::executeAccount(cfg, [this](const QString &msg, const QString &type) {
        logMessage(msg, type);
    });

    if (m_chkResetDevice && m_chkResetDevice->isChecked()) {
        logMessage("Bắn lệnh ADB dọn sạch app (pm clear com.dts.freefireth) & Fake ID...", "CLEAN");
    }

    if (ok) {
        m_successCount++;
        if (m_lblSuccess) m_lblSuccess->setText(QString::number(m_successCount));
    } else {
        m_failCount++;
        if (m_lblFailed) m_lblFailed->setText(QString::number(m_failCount));
    }

    int percent = (int)(((m_currentIndex + 1) * 100.0) / m_accounts.size());
    if (m_progressBar) m_progressBar->setValue(percent);

    m_currentIndex++;
}

void FreeFireRunDialog::onStopExecution()
{
    m_isRunning = false;
    if (m_timer) m_timer->stop();
    if (m_lblProcessing) m_lblProcessing->setText("0");
    if (m_lblStatusBadge) {
        m_lblStatusBadge->setText(QString::fromUtf8("⏹ ĐÃ DỪNG BỞI USER"));
        m_lblStatusBadge->setStyleSheet("background: #fee2e2; color: #dc2626; font-size: 11px; font-weight: 700; border-radius: 6px; padding: 5px 12px; border: 1px solid #fecaca;");
    }
    if (m_btnStopRun) m_btnStopRun->setEnabled(false);
    if (m_btnBackConfig) m_btnBackConfig->setEnabled(true);
    logMessage("Kịch bản đã bị tạm dừng bởi người dùng.", "STOP");
}

void FreeFireRunDialog::onBackToConfig()
{
    if (m_isRunning) {
        onStopExecution();
    }
    m_stack->setCurrentIndex(0);
}

void FreeFireRunDialog::logMessage(const QString &msg, const QString &type)
{
    if (!m_txtLog) return;
    QString timeStr = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString formatted = QString("[%1] [%2] %3").arg(timeStr, type, msg);
    m_txtLog->appendPlainText(formatted);
    m_txtLog->verticalScrollBar()->setValue(m_txtLog->verticalScrollBar()->maximum());
}
