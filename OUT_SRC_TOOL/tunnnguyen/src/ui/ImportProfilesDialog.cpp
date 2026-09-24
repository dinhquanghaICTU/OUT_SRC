#include "ui/ImportProfilesDialog.h"
#include "ui/CustomMessageBox.h"
#include "ui/EditProfileDialog.h"
#include "core/ProxyConfig.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QHeaderView>
#include <QClipboard>
#include <QGuiApplication>
#include <QDateTime>
#include <QUuid>
#include <QSet>
#include <QRegularExpression>

ImportProfilesDialog::ImportProfilesDialog(const QList<ChromeProfileItem> &existingProfiles, QWidget *parent)
    : QDialog(parent), m_existingProfiles(existingProfiles)
{
    setupUi();
}

void ImportProfilesDialog::setupUi()
{
    setWindowTitle("Nạp Danh Sách Profile Hàng Loạt (Excel / CSV)");
    resize(1020, 720);
    setMinimumSize(920, 620);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    setStyleSheet(
        "* { font-family: 'Google Sans', 'Product Sans', -apple-system, sans-serif; }"
        "QDialog { background-color: #ffffff; }"
        "QTableWidget { background-color: #ffffff; border: 1px solid #e2e8f0; border-radius: 10px; font-size: 12.5px; selection-background-color: #fff7ed; selection-color: #9a3412; }"
        "QHeaderView::section { background-color: #f8fafc; color: #475569; font-weight: 700; font-size: 12px; border: none; border-bottom: 2px solid #e2e8f0; padding: 10px 8px; }"
        "QTextEdit { background-color: #f8fafc; border: 1px dashed #cbd5e1; border-radius: 10px; padding: 8px; font-family: monospace; font-size: 12px; color: #1e293b; }"
        "QTextEdit:focus { border: 1.5px solid #f97316; background: #ffffff; }"
        "QLabel { color: #334155; }"
    );

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 20);
    mainLayout->setSpacing(14);

    // 1. Header Banner
    auto *headerCard = new QFrame();
    headerCard->setStyleSheet("background: #f8fafc; border: 1px solid #e2e8f0; border-radius: 12px; padding: 10px;");
    auto *headerLayout = new QHBoxLayout(headerCard);
    headerLayout->setContentsMargins(8, 6, 8, 6);

    auto *iconLbl = new QLabel("📥");
    iconLbl->setStyleSheet("font-size: 26px; background: #ffedd5; padding: 8px; border-radius: 12px;");
    headerLayout->addWidget(iconLbl);

    auto *headerTextLayout = new QVBoxLayout();
    headerTextLayout->setSpacing(2);
    auto *titleLbl = new QLabel("Nạp Hàng Loạt Chrome Profiles Từ Excel / CSV");
    titleLbl->setStyleSheet("font-size: 16px; font-weight: 800; color: #0f172a;");
    auto *descLbl = new QLabel("Hỗ trợ file Excel (.csv, .txt) hoặc copy-paste trực tiếp từ bảng tính. Tự động kiểm tra trùng tên và cấp phát Remote Port.");
    descLbl->setStyleSheet("font-size: 12px; color: #64748b;");
    headerTextLayout->addWidget(titleLbl);
    headerTextLayout->addWidget(descLbl);
    headerLayout->addLayout(headerTextLayout);
    headerLayout->addStretch();

    mainLayout->addWidget(headerCard);

    // 2. Action Buttons Row (Import File + Download Template + Paste)
    auto *actionRow = new QHBoxLayout();
    actionRow->setSpacing(10);

    auto *btnSelectFile = new QPushButton("📂  Chọn File (.csv, .txt, .tsv)");
    btnSelectFile->setCursor(Qt::PointingHandCursor);
    btnSelectFile->setStyleSheet(
        "QPushButton { background: #2563eb; color: #ffffff; font-weight: 700; "
        "border: none; border-radius: 8px; padding: 9px 16px; font-size: 13px; } "
        "QPushButton:hover { background: #1d4ed8; }");
    connect(btnSelectFile, &QPushButton::clicked, this, &ImportProfilesDialog::onSelectFileClicked);
    actionRow->addWidget(btnSelectFile);

    auto *btnTemplate = new QPushButton("📥  Tải File Mẫu (.csv)");
    btnTemplate->setCursor(Qt::PointingHandCursor);
    btnTemplate->setStyleSheet(
        "QPushButton { background: #f0fdf4; color: #16a34a; font-weight: 700; "
        "border: 1px solid #bbf7d0; border-radius: 8px; padding: 9px 14px; font-size: 13px; } "
        "QPushButton:hover { background: #dcfce7; }");
    connect(btnTemplate, &QPushButton::clicked, this, &ImportProfilesDialog::onExportTemplateClicked);
    actionRow->addWidget(btnTemplate);

    auto *btnPaste = new QPushButton("📋  Dán Clipboard");
    btnPaste->setCursor(Qt::PointingHandCursor);
    btnPaste->setStyleSheet(
        "QPushButton { background: #ffffff; color: #475569; font-weight: 600; "
        "border: 1px solid #cbd5e1; border-radius: 8px; padding: 9px 14px; font-size: 13px; } "
        "QPushButton:hover { background: #f1f5f9; }");
    connect(btnPaste, &QPushButton::clicked, this, &ImportProfilesDialog::onPasteFromClipboardClicked);
    actionRow->addWidget(btnPaste);

    m_btnTogglePasteBox = new QPushButton("✏️ Nhập Text");
    m_btnTogglePasteBox->setCursor(Qt::PointingHandCursor);
    m_btnTogglePasteBox->setStyleSheet(
        "QPushButton { background: transparent; color: #f97316; font-weight: 700; "
        "border: none; padding: 9px 8px; font-size: 12.5px; } "
        "QPushButton:hover { text-decoration: underline; }");
    connect(m_btnTogglePasteBox, &QPushButton::clicked, [this]() {
        bool vis = !m_pasteBoxWidget->isVisible();
        m_pasteBoxWidget->setVisible(vis);
        m_btnTogglePasteBox->setText(vis ? "🔼 Đóng Khung Nhập Text" : "✏️ Nhập Text");
    });
    actionRow->addWidget(m_btnTogglePasteBox);

    actionRow->addStretch();
    mainLayout->addLayout(actionRow);

    // 3. Collapsible Manual Paste Box
    m_pasteBoxWidget = new QWidget();
    auto *pasteLayout = new QVBoxLayout(m_pasteBoxWidget);
    pasteLayout->setContentsMargins(0, 0, 0, 0);
    pasteLayout->setSpacing(6);

    auto *pasteHeader = new QHBoxLayout();
    pasteHeader->addWidget(new QLabel("Dán nội dung từ Excel/Notepad vào đây (Mỗi dòng 1 profile: Tên,Proxy,ThiếtBị,TỉLệ,Port):"));
    pasteHeader->addStretch();

    auto *btnParse = new QPushButton("🔍 Phân Tích Dữ Liệu");
    btnParse->setCursor(Qt::PointingHandCursor);
    btnParse->setStyleSheet("QPushButton { background: #f97316; color: #ffffff; font-weight: 700; border-radius: 6px; padding: 4px 10px; font-size: 12px; border: none; } QPushButton:hover { background: #ea580c; }");
    connect(btnParse, &QPushButton::clicked, this, &ImportProfilesDialog::onParseTextClicked);
    pasteHeader->addWidget(btnParse);
    pasteLayout->addLayout(pasteHeader);

    m_rawTextEdit = new QTextEdit();
    m_rawTextEdit->setFixedHeight(100);
    m_rawTextEdit->setPlaceholderText("Ví dụ:\nProfile Nuôi Nick 01,103.149.28.12:8080,windows,380x680,9222\nProfile Nuôi Nick 02,socks5://1.2.3.4:1080:user:pass,iphone,450x700,9223\nProfile TikTok 03,,android,600x800");
    pasteLayout->addWidget(m_rawTextEdit);

    m_pasteBoxWidget->setVisible(false);
    mainLayout->addWidget(m_pasteBoxWidget);

    // 4. Preview Table
    auto *previewLbl = new QLabel("📋 Danh sách Profile nhận diện được (Xem trước khi lưu):");
    previewLbl->setStyleSheet("font-weight: 700; font-size: 13px; color: #1e293b;");
    mainLayout->addWidget(previewLbl);

    m_previewTable = new QTableWidget();
    m_previewTable->setColumnCount(7);
    QStringList headers = {"STT", "Tên Profile", "Proxy Gán Kèm", "Thiết Bị", "Tỉ Lệ Mở", "Port (CDP)", "Trạng Thái"};
    m_previewTable->setHorizontalHeaderLabels(headers);
    m_previewTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_previewTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_previewTable->verticalHeader()->setVisible(false);
    m_previewTable->verticalHeader()->setDefaultSectionSize(44);
    m_previewTable->setShowGrid(false);
    m_previewTable->setAlternatingRowColors(true);

    m_previewTable->setColumnWidth(0, 55);
    m_previewTable->setColumnWidth(3, 140);
    m_previewTable->setColumnWidth(4, 115);
    m_previewTable->setColumnWidth(5, 95);
    m_previewTable->setColumnWidth(6, 160);
    m_previewTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_previewTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);

    mainLayout->addWidget(m_previewTable, 1);

    // 5. Options Checkbox Row (standalone, no collision)
    auto *optionsLayout = new QHBoxLayout();
    m_autoRenameDuplicatesCheck = new QCheckBox("Tự động đổi tên nếu trùng với Profile đã có (Thêm đuôi _1, _2...)");
    m_autoRenameDuplicatesCheck->setChecked(true);
    m_autoRenameDuplicatesCheck->setStyleSheet("QCheckBox { font-size: 13px; font-weight: 600; color: #334155; }");
    connect(m_autoRenameDuplicatesCheck, &QCheckBox::toggled, this, &ImportProfilesDialog::updatePreviewTable);
    optionsLayout->addWidget(m_autoRenameDuplicatesCheck);
    optionsLayout->addStretch();
    mainLayout->addLayout(optionsLayout);

    // 6. Bottom Row: Status on left, buttons on right
    auto *btnRow = new QHBoxLayout();
    btnRow->setSpacing(12);

    m_statusSummaryLabel = new QLabel("Chưa có profile nào. Hãy chọn file hoặc bấm 'Dán Clipboard'!");
    m_statusSummaryLabel->setStyleSheet("font-size: 13px; font-weight: 700; color: #64748b;");
    btnRow->addWidget(m_statusSummaryLabel, 1);

    auto *btnCancel = new QPushButton("Đóng");
    btnCancel->setCursor(Qt::PointingHandCursor);
    btnCancel->setStyleSheet(
        "QPushButton { background: #f1f5f9; color: #475569; font-weight: 600; "
        "border: none; border-radius: 8px; padding: 10px 22px; font-size: 13px; } "
        "QPushButton:hover { background: #e2e8f0; }");
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
    btnRow->addWidget(btnCancel);

    m_btnImport = new QPushButton("✨  Nạp Profile Vào Hệ Thống (0)");
    m_btnImport->setCursor(Qt::PointingHandCursor);
    m_btnImport->setEnabled(false);
    m_btnImport->setStyleSheet(
        "QPushButton { background: #f97316; color: #ffffff; font-weight: 700; "
        "border: none; border-radius: 8px; padding: 10px 24px; font-size: 13px; } "
        "QPushButton:hover { background: #ea580c; } "
        "QPushButton:disabled { background: #cbd5e1; color: #94a3b8; }");
    connect(m_btnImport, &QPushButton::clicked, this, &ImportProfilesDialog::onConfirmImportClicked);
    btnRow->addWidget(m_btnImport);

    mainLayout->addLayout(btnRow);
}

void ImportProfilesDialog::onSelectFileClicked()
{
    QString filePath = QFileDialog::getOpenFileName(
        this, "Chọn File Excel / CSV Chứa Danh Sách Profile",
        QDir::homePath(),
        "Tất cả định dạng (*.csv *.txt *.tsv);;CSV File (*.csv);;Text File (*.txt);;Mọi file (*.*)");

    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        CustomMessageBox::warning(this, "Lỗi đọc file", "Không thể mở file được chọn: " + filePath);
        return;
    }

    QByteArray rawData = file.readAll();
    file.close();

    // Decode UTF-8 (handling BOM automatically if present)
    QString content = QString::fromUtf8(rawData);
    parseRawContent(content);
}

void ImportProfilesDialog::onPasteFromClipboardClicked()
{
    QClipboard *clip = QGuiApplication::clipboard();
    if (!clip || clip->text().trimmed().isEmpty()) {
        CustomMessageBox::information(this, "Thông báo", "Clipboard hiện đang trống. Hãy sao chép (Ctrl+C) các dòng từ Excel trước!");
        return;
    }

    QString text = clip->text();
    m_rawTextEdit->setText(text);
    m_pasteBoxWidget->setVisible(true);
    m_btnTogglePasteBox->setText("🔼 Đóng Khung Nhập Text");
    parseRawContent(text);
}

void ImportProfilesDialog::onParseTextClicked()
{
    parseRawContent(m_rawTextEdit->toPlainText());
}

void ImportProfilesDialog::parseRawContent(const QString &content)
{
    m_parsedProfiles.clear();
    QStringList lines = content.split(QRegularExpression("[\r\n]+"), Qt::SkipEmptyParts);
    if (lines.isEmpty()) {
        updatePreviewTable();
        return;
    }

    int nextPort = findNextAvailablePort();

    for (int lineIdx = 0; lineIdx < lines.size(); ++lineIdx) {
        QString line = lines[lineIdx].trimmed();
        if (line.isEmpty() || line.startsWith("#")) continue;

        // Skip header if lineIdx == 0 and contains keywords
        if (lineIdx == 0) {
            QString lower = line.toLower();
            if (lower.contains("tên profile") || lower.contains("profile name") || lower.contains("proxy") || lower.contains("remote port")) {
                continue;
            }
        }

        // Determine separator: comma, semicolon, tab, pipe
        QChar sep = ',';
        if (line.contains('\t')) sep = '\t';
        else if (line.contains(';')) sep = ';';
        else if (line.contains('|')) sep = '|';
        else if (line.contains(',')) sep = ',';

        QStringList tokens = line.split(sep);
        for (QString &tok : tokens) {
            tok = tok.trimmed();
            if (tok.startsWith("\"") && tok.endsWith("\"") && tok.length() >= 2) {
                tok = tok.mid(1, tok.length() - 2).trimmed();
            }
        }

        if (tokens.isEmpty()) continue;

        QString name = tokens.value(0).trimmed();
        if (name.isEmpty()) continue;

        QString proxy = tokens.value(1).trimmed();
        QString device = tokens.value(2).toLower().trimmed();
        if (device.isEmpty()) device = "windows";

        QString sizeStr = tokens.value(3).trimmed();
        int width = 0;
        int height = 0;
        if (!sizeStr.isEmpty()) {
            QRegularExpression sizeRx("(\\d+)[xX*](\\d+)");
            auto match = sizeRx.match(sizeStr);
            if (match.hasMatch()) {
                width = match.captured(1).toInt();
                height = match.captured(2).toInt();
            }
        }

        int port = tokens.value(4).toInt();
        if (port <= 0 || port > 65535) {
            port = nextPort++;
        } else {
            if (port >= nextPort) nextPort = port + 1;
        }

        ChromeProfileItem item;
        item.id = "profile_" + QUuid::createUuid().toString(QUuid::WithoutBraces).left(8);
        item.name = name;
        item.proxy = proxy;
        item.deviceType = device;
        item.customUserAgent = EditProfileDialog::getUserAgentForDevice(device);
        item.windowWidth = width;
        item.windowHeight = height;
        item.port = port;
        item.isRunning = false;
        item.isSelected = false;

        m_parsedProfiles.append(item);
    }

    updatePreviewTable();
}

void ImportProfilesDialog::updatePreviewTable()
{
    m_previewTable->setRowCount(0);
    if (m_parsedProfiles.isEmpty()) {
        m_statusSummaryLabel->setText("Chưa có profile nào. Hãy chọn file Excel hoặc bấm 'Dán Từ Clipboard'!");
        m_statusSummaryLabel->setStyleSheet("font-size: 12.5px; font-weight: 700; color: #64748b;");
        m_btnImport->setEnabled(false);
        m_btnImport->setText("✨  Nạp Profile Vào Hệ Thống (0)");
        return;
    }

    bool autoRename = m_autoRenameDuplicatesCheck->isChecked();
    QSet<QString> existingNamesSet;
    for (const auto &p : m_existingProfiles) {
        existingNamesSet.insert(p.name.trimmed().toLower());
    }

    QSet<QString> currentBatchNames;
    m_previewTable->setRowCount(m_parsedProfiles.size());

    int validCount = 0;

    for (int i = 0; i < m_parsedProfiles.size(); ++i) {
        auto &item = m_parsedProfiles[i];
        QString lowerName = item.name.trimmed().toLower();
        bool isDuplicate = existingNamesSet.contains(lowerName) || currentBatchNames.contains(lowerName);

        QString statusText = "🟢 Hợp lệ";
        QColor statusColor("#16a34a");

        if (isDuplicate) {
            if (autoRename) {
                // Generate a unique name
                int counter = 1;
                QString newName = QString("%1 (%2)").arg(item.name).arg(counter);
                while (existingNamesSet.contains(newName.toLower()) || currentBatchNames.contains(newName.toLower())) {
                    counter++;
                    newName = QString("%1 (%2)").arg(item.name).arg(counter);
                }
                item.name = newName;
                statusText = "🟡 Đã đổi tên tránh trùng";
                statusColor = QColor("#ea580c");
            } else {
                statusText = "🔴 Trùng tên (Bị từ chối)";
                statusColor = QColor("#ef4444");
            }
        }

        currentBatchNames.insert(item.name.trimmed().toLower());

        // 0. STT
        auto *sttItem = new QTableWidgetItem(QString::number(i + 1));
        sttItem->setTextAlignment(Qt::AlignCenter);
        m_previewTable->setItem(i, 0, sttItem);

        // 1. Tên Profile
        auto *nameItem = new QTableWidgetItem(item.name);
        nameItem->setFont(QFont("Google Sans", 10, QFont::Bold));
        m_previewTable->setItem(i, 1, nameItem);

        // 2. Proxy
        ProxyConfig pCfg = ProxyConfig::fromString(item.proxy);
        auto *proxyItem = new QTableWidgetItem(pCfg.toDisplayString());
        proxyItem->setForeground(pCfg.isEmpty() ? QColor("#94a3b8") : QColor("#ea580c"));
        m_previewTable->setItem(i, 2, proxyItem);

        // 3. Thiết Bị
        QString devStr = QString("%1 %2").arg(EditProfileDialog::getDeviceIcon(item.deviceType), EditProfileDialog::getDeviceDisplayName(item.deviceType));
        auto *devItem = new QTableWidgetItem(devStr);
        m_previewTable->setItem(i, 3, devItem);

        // 4. Tỉ Lệ Mở
        QString sizeStr = (item.windowWidth > 0 && item.windowHeight > 0)
            ? QString("%1 x %2").arg(item.windowWidth).arg(item.windowHeight)
            : "Tool Auto";
        auto *sizeItem = new QTableWidgetItem(sizeStr);
        sizeItem->setTextAlignment(Qt::AlignCenter);
        m_previewTable->setItem(i, 4, sizeItem);

        // 5. Port
        auto *portItem = new QTableWidgetItem(QString::number(item.port));
        portItem->setTextAlignment(Qt::AlignCenter);
        m_previewTable->setItem(i, 5, portItem);

        // 6. Trạng Thái
        auto *statItem = new QTableWidgetItem(statusText);
        statItem->setTextAlignment(Qt::AlignCenter);
        statItem->setForeground(statusColor);
        statItem->setFont(QFont("Google Sans", 9, QFont::Bold));
        m_previewTable->setItem(i, 6, statItem);

        if (!statusText.contains("🔴")) {
            validCount++;
        }
    }

    m_statusSummaryLabel->setText(QString("📊 Tìm thấy %1 profile hợp lệ sẵn sàng nạp!").arg(validCount));
    m_statusSummaryLabel->setStyleSheet("font-size: 13px; font-weight: 700; color: #16a34a;");
    m_btnImport->setEnabled(validCount > 0);
    m_btnImport->setText(QString("✨  Nạp %1 Profile Vào Hệ Thống").arg(validCount));
}

int ImportProfilesDialog::findNextAvailablePort(int basePort)
{
    int maxPort = basePort - 1;
    for (const auto &p : m_existingProfiles) {
        if (p.port > maxPort) maxPort = p.port;
    }
    return maxPort + 1;
}

QList<ChromeProfileItem> ImportProfilesDialog::getImportedProfiles() const
{
    QList<ChromeProfileItem> result;
    for (int i = 0; i < m_parsedProfiles.size(); ++i) {
        auto *statItem = m_previewTable->item(i, 6);
        if (statItem && !statItem->text().contains("🔴")) {
            result.append(m_parsedProfiles[i]);
        }
    }
    return result;
}

void ImportProfilesDialog::onConfirmImportClicked()
{
    auto list = getImportedProfiles();
    if (list.isEmpty()) {
        CustomMessageBox::warning(this, "Thông báo", "Không có profile hợp lệ nào để nạp!");
        return;
    }
    accept();
}

void ImportProfilesDialog::onExportTemplateClicked()
{
    exportTemplateCsv(this);
}

bool ImportProfilesDialog::exportTemplateCsv(QWidget *parent)
{
    QString savePath = QFileDialog::getSaveFileName(
        parent, "Lưu File Mẫu Excel Chuẩn (.csv)",
        QDir::homePath() + "/tunnbit_profiles_template.csv",
        "CSV File Excel (*.csv);;Text File (*.txt)");

    if (savePath.isEmpty()) return false;

    QFile file(savePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        CustomMessageBox::warning(parent, "Lỗi ghi file", "Không thể tạo file tại đường dẫn:\n" + savePath);
        return false;
    }

    // Write UTF-8 BOM so Microsoft Excel opens Vietnamese text without mojibake/garbled text
    const char bom[] = "\xEF\xBB\xBF";
    file.write(bom, 3);

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    // Header row
    out << "Tên Profile,Proxy,Thiết Bị,Tỉ Lệ Mở,Remote Port (CDP)\n";

    // Sample data rows
    out << "Profile MMO 01,103.149.28.12:8080,windows,380x680,9222\n";
    out << "Profile Nuôi Nick 02,socks5://1.2.3.4:1080:user:pass,iphone,450x700,9223\n";
    out << "Profile TikTok 03,45.76.12.9:3128:mmo:123456,android,600x800,9224\n";
    out << "Profile MacOS 04,103.149.28.15:8080,macos,1280x720,9225\n";
    out << "Profile Mạng Trực Tiếp 05,,windows,0x0,\n";

    file.close();

    CustomMessageBox::information(
        parent, "Tải File Mẫu Thành Công",
        QString("Đã lưu file mẫu Excel thành công tại:\n%1\n\nBạn có thể mở file này bằng Microsoft Excel, Google Sheets để điền danh sách Profile và chọn 'Import File' để nạp hàng loạt!").arg(savePath));

    return true;
}
