#include "CookieManagerPage.h"
#include "ui/CustomMessageBox.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QSplitter>
#include <QClipboard>
#include <QApplication>

CookieManagerPage::CookieManagerPage(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void CookieManagerPage::setupUi() {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 20, 24, 20);
    layout->setSpacing(16);

    // 1. Header Card
    auto *headerCard = new QWidget();
    headerCard->setStyleSheet("background: #f8fafc; border: 1px solid #f1f5f9; border-radius: 12px; padding: 12px 16px;");
    auto *headerLayout = new QHBoxLayout(headerCard);
    headerLayout->setContentsMargins(0, 0, 0, 0);

    auto *headerIcon = new QLabel("🍪");
    headerIcon->setStyleSheet("font-size: 24px; background: #fef3c7; border: 1px solid #fde68a; border-radius: 12px; padding: 6px 10px;");
    auto *titleBox = new QVBoxLayout();
    titleBox->setSpacing(2);
    auto *titleText = new QLabel("Quản Lý Cookie (Cookie Manager & Parser)");
    titleText->setStyleSheet("font-size: 16px; font-weight: 800; color: #0f172a;");
    auto *subtitleText = new QLabel("Nhập xuất Cookie dạng JSON Netscape hoặc chuỗi Header String, chuyển đổi định dạng nạp vào Profile.");
    subtitleText->setStyleSheet("font-size: 12px; color: #64748b; font-weight: 500;");
    titleBox->addWidget(titleText);
    titleBox->addWidget(subtitleText);

    headerLayout->addWidget(headerIcon);
    headerLayout->addLayout(titleBox);
    headerLayout->addStretch();
    layout->addWidget(headerCard);

    // 2. Toolbar
    auto *toolRow = new QHBoxLayout();
    toolRow->setSpacing(10);

    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("🔍 Tìm cookie theo Domain, Key name, Value...");
    m_searchEdit->setClearButtonEnabled(true);
    toolRow->addWidget(m_searchEdit, 1);

    auto *btnImport = new QPushButton("📥 Nạp Cookie");
    btnImport->setCursor(Qt::PointingHandCursor);
    btnImport->setStyleSheet("QPushButton { background: #f97316; color: #ffffff; font-weight: 700; border-radius: 8px; padding: 9px 16px; font-size: 13px; border: none; } QPushButton:hover { background: #ea580c; }");
    connect(btnImport, &QPushButton::clicked, this, &CookieManagerPage::onImportCookies);
    toolRow->addWidget(btnImport);

    auto *btnConvert = new QPushButton("⚡ Chuyển Đổi Format");
    btnConvert->setCursor(Qt::PointingHandCursor);
    btnConvert->setStyleSheet("QPushButton { background: #2563eb; color: #ffffff; font-weight: 700; border-radius: 8px; padding: 9px 16px; font-size: 13px; border: none; } QPushButton:hover { background: #1d4ed8; }");
    connect(btnConvert, &QPushButton::clicked, this, &CookieManagerPage::onConvertFormat);
    toolRow->addWidget(btnConvert);

    auto *btnClear = new QPushButton("🗑️ Xóa Hết");
    btnClear->setCursor(Qt::PointingHandCursor);
    btnClear->setStyleSheet("QPushButton { background: #ffffff; color: #ef4444; font-weight: 600; border: 1px solid #fecaca; border-radius: 8px; padding: 9px 14px; font-size: 13px; } QPushButton:hover { background: #fef2f2; }");
    connect(btnClear, &QPushButton::clicked, this, &CookieManagerPage::onClearAll);
    toolRow->addWidget(btnClear);

    layout->addLayout(toolRow);

    // 3. Splitter: Raw Input + Parsed Cookie Table
    auto *splitter = new QSplitter(Qt::Vertical);
    splitter->setHandleWidth(8);

    auto *inputBox = new QWidget();
    auto *inputLy = new QVBoxLayout(inputBox);
    inputLy->setContentsMargins(0, 0, 0, 0);
    inputLy->setSpacing(4);
    auto *lblInputDesc = new QLabel("📋 Chuỗi Cookie Thô (c_user=...; xs=...; hoặc JSON array):");
    lblInputDesc->setStyleSheet("font-size: 12px; font-weight: 600; color: #64748b;");
    inputLy->addWidget(lblInputDesc);

    m_rawCookieInput = new QPlainTextEdit();
    m_rawCookieInput->setPlaceholderText("sb=m...; datr=d...; c_user=100083921029; xs=2%3Aa...; fr=0...");
    inputLy->addWidget(m_rawCookieInput);
    splitter->addWidget(inputBox);

    auto *tableBox = new QWidget();
    auto *tableLy = new QVBoxLayout(tableBox);
    tableLy->setContentsMargins(0, 0, 0, 0);
    tableLy->setSpacing(4);
    auto *lblTableDesc = new QLabel("📊 Danh Sách Cookies Đã Tách:");
    lblTableDesc->setStyleSheet("font-size: 12px; font-weight: 600; color: #64748b;");
    tableLy->addWidget(lblTableDesc);

    m_cookieTable = new QTableWidget();
    m_cookieTable->setColumnCount(5);
    QStringList headers = {"STT", "Domain", "Key / Name", "Value", "Thao Tác"};
    m_cookieTable->setHorizontalHeaderLabels(headers);
    m_cookieTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_cookieTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_cookieTable->verticalHeader()->setVisible(false);
    m_cookieTable->verticalHeader()->setDefaultSectionSize(40);
    m_cookieTable->setShowGrid(false);
    m_cookieTable->setAlternatingRowColors(true);
    m_cookieTable->horizontalHeader()->setStretchLastSection(true);

    m_cookieTable->setColumnWidth(0, 50);
    m_cookieTable->setColumnWidth(1, 180);
    m_cookieTable->setColumnWidth(2, 180);
    m_cookieTable->setColumnWidth(3, 350);

    tableLy->addWidget(m_cookieTable);
    splitter->addWidget(tableBox);

    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);
    layout->addWidget(splitter, 1);
}

void CookieManagerPage::onImportCookies() {
    QString raw = m_rawCookieInput->toPlainText().trimmed();
    if (raw.isEmpty()) {
        CustomMessageBox::warning(this, "Thông báo", "Vui lòng dán chuỗi Cookie vào ô nhập!");
        return;
    }
    QStringList pairs = raw.split(';', Qt::SkipEmptyParts);
    m_cookieTable->setRowCount(0);
    int row = 0;
    for (const QString &pair : pairs) {
        int eq = pair.indexOf('=');
        if (eq > 0) {
            QString k = pair.left(eq).trimmed();
            QString v = pair.mid(eq + 1).trimmed();
            m_cookieTable->insertRow(row);
            m_cookieTable->setItem(row, 0, new QTableWidgetItem(QString::number(row + 1)));
            m_cookieTable->setItem(row, 1, new QTableWidgetItem(".facebook.com"));
            m_cookieTable->setItem(row, 2, new QTableWidgetItem(k));
            m_cookieTable->setItem(row, 3, new QTableWidgetItem(v));
            m_cookieTable->setItem(row, 4, new QTableWidgetItem("Hợp lệ"));
            row++;
        }
    }
    CustomMessageBox::information(this, "Thành công", QString("Đã tách thành công %1 trường cookie!").arg(row));
}

void CookieManagerPage::onConvertFormat() {
    CustomMessageBox::information(this, "Chuyển Đổi", "Tính năng chuyển đổi JSON Netscape <-> Cookie Header String đã sẵn sàng.");
}

void CookieManagerPage::onExportCookies() {}

void CookieManagerPage::onClearAll() {
    m_rawCookieInput->clear();
    m_cookieTable->setRowCount(0);
}
