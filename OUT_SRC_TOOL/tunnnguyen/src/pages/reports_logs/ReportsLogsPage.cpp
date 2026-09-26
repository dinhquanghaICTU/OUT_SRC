#include "ReportsLogsPage.h"
#include "ui/CustomMessageBox.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDateTime>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QListView>

ReportsLogsPage::ReportsLogsPage(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void ReportsLogsPage::setupUi() {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 20, 24, 20);
    layout->setSpacing(16);

    // 1. Header Card
    auto *headerCard = new QWidget();
    headerCard->setStyleSheet("background: #f8fafc; border: 1px solid #f1f5f9; border-radius: 12px; padding: 12px 16px;");
    auto *headerLayout = new QHBoxLayout(headerCard);
    headerLayout->setContentsMargins(0, 0, 0, 0);

    auto *headerIcon = new QLabel("📈");
    headerIcon->setStyleSheet("font-size: 24px; background: #e0e7ff; border: 1px solid #c7d2fe; border-radius: 12px; padding: 6px 10px;");
    auto *titleBox = new QVBoxLayout();
    titleBox->setSpacing(2);
    auto *titleText = new QLabel("Báo Cáo Tiến Độ & Nhật Ký Hoạt Động (Reports & Logs)");
    titleText->setStyleSheet("font-size: 16px; font-weight: 800; color: #0f172a;");
    auto *subtitleText = new QLabel("Theo dõi toàn bộ luồng thực thi CDP, ADB Emulator, đổi thông tin tài khoản và kiểm tra Proxy theo thời gian thực.");
    subtitleText->setStyleSheet("font-size: 12px; color: #64748b; font-weight: 500;");
    titleBox->addWidget(titleText);
    titleBox->addWidget(subtitleText);

    headerLayout->addWidget(headerIcon);
    headerLayout->addLayout(titleBox);
    headerLayout->addStretch();

    m_lblTotalLogs = new QLabel("0 sự kiện");
    m_lblTotalLogs->setStyleSheet("background: #ffffff; border: 1px solid #e2e8f0; border-radius: 8px; padding: 6px 14px; font-size: 13px; font-weight: 700; color: #0f172a;");
    headerLayout->addWidget(m_lblTotalLogs);
    layout->addWidget(headerCard);

    // 2. Toolbar
    auto *toolRow = new QHBoxLayout();
    toolRow->setSpacing(10);

    m_filterLevelCombo = new QComboBox();
    m_filterLevelCombo->setView(new QListView(m_filterLevelCombo));
    m_filterLevelCombo->addItem("Tất cả mức độ (ALL LOGS)");
    m_filterLevelCombo->addItem("🟢 SUCCESS");
    m_filterLevelCombo->addItem("🔵 INFO");
    m_filterLevelCombo->addItem("🟡 WARNING");
    m_filterLevelCombo->addItem("🔴 ERROR");
    toolRow->addWidget(m_filterLevelCombo);

    auto *btnExport = new QPushButton("💾 Xuất File Log (.log)");
    btnExport->setCursor(Qt::PointingHandCursor);
    btnExport->setStyleSheet("QPushButton { background: #0f172a; color: #ffffff; font-weight: 700; border-radius: 8px; padding: 9px 16px; font-size: 13px; } QPushButton:hover { background: #1e293b; }");
    connect(btnExport, &QPushButton::clicked, this, &ReportsLogsPage::onExportLogs);
    toolRow->addWidget(btnExport);

    auto *btnClear = new QPushButton("🗑️ Xóa Màn Hình");
    btnClear->setCursor(Qt::PointingHandCursor);
    btnClear->setStyleSheet("QPushButton { background: #ffffff; color: #ef4444; font-weight: 600; border: 1px solid #fecaca; border-radius: 8px; padding: 9px 14px; font-size: 13px; } QPushButton:hover { background: #fef2f2; }");
    connect(btnClear, &QPushButton::clicked, this, &ReportsLogsPage::onClearLogs);
    toolRow->addWidget(btnClear);

    toolRow->addStretch();
    layout->addLayout(toolRow);

    // 3. Log Console
    m_logConsole = new QPlainTextEdit();
    m_logConsole->setReadOnly(true);
    m_logConsole->setStyleSheet("background-color: #0f172a; color: #38bdf8; font-family: monospace; font-size: 13px; border-radius: 12px; padding: 16px;");
    layout->addWidget(m_logConsole, 1);

    appendLog("INFO", "Hệ thống ghi log khởi tạo thành công.");
    appendLog("INFO", "Sẵn sàng nhận sự kiện tự động từ Task Runner & Profile Engine.");
}

void ReportsLogsPage::appendLog(const QString &level, const QString &message) {
    if (!m_logConsole) return;
    QString timeStr = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    QString color = "#38bdf8";
    if (level == "SUCCESS") color = "#4ade80";
    else if (level == "ERROR") color = "#f87171";
    else if (level == "WARN") color = "#fbbf24";

    QString htmlLine = QString("<span style='color: #64748b;'>[%1]</span> <span style='color: %2; font-weight: bold;'>[%3]</span> <span style='color: #f8fafc;'>%4</span>")
                           .arg(timeStr, color, level, message);
    m_logConsole->appendHtml(htmlLine);
    m_logCount++;
    if (m_lblTotalLogs) m_lblTotalLogs->setText(QString("%1 sự kiện").arg(m_logCount));
}

void ReportsLogsPage::onExportLogs() {
    QString path = QFileDialog::getSaveFileName(this, "Xuất file Log", "system_activity.log", "Log Files (*.log *.txt)");
    if (path.isEmpty()) return;
    QFile f(path);
    if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream ts(&f);
        ts << m_logConsole->toPlainText();
        f.close();
        CustomMessageBox::information(this, "Thành công", "Đã xuất file log thành công!");
    }
}

void ReportsLogsPage::onClearLogs() {
    m_logConsole->clear();
    m_logCount = 0;
    if (m_lblTotalLogs) m_lblTotalLogs->setText("0 sự kiện");
}

void ReportsLogsPage::onFilterLevelChanged(int index) {
    Q_UNUSED(index);
}
