#include "AccountParserPage.h"
#include "ui/CustomMessageBox.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QHeaderView>
#include <QListView>
#include <QFileDialog>
#include <QClipboard>
#include <QApplication>
#include <QTextStream>
#include <QSet>

AccountParserPage::AccountParserPage(QWidget *parent)
    : QObject(parent), m_parentWidget(parent)
{
    setupCenterPanel();
    setupRightSummaryPanel();
}

void AccountParserPage::setupCenterPanel() {
    m_centerWidget = new QWidget(m_parentWidget);
    auto *layout = new QVBoxLayout(m_centerWidget);
    layout->setContentsMargins(24, 20, 24, 20);
    layout->setSpacing(16);

    // 1. Search Bar & Top Action Buttons (Same Row)
    auto *titleRow = new QHBoxLayout();
    titleRow->setSpacing(12);

    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("🔍  Tìm kiếm tài khoản (UID, Email, Pass, 2FA)...");
    m_searchEdit->setClearButtonEnabled(true);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &AccountParserPage::onSearchFilterChanged);
    titleRow->addWidget(m_searchEdit, 1);

    titleRow->addSpacing(8);

    auto *btnImport = new QPushButton("+  Mở File Text");
    btnImport->setCursor(Qt::PointingHandCursor);
    btnImport->setStyleSheet(
        "QPushButton { background: #f97316; color: #ffffff; font-weight: 700; "
        "border: none; border-radius: 8px; padding: 9px 18px; font-size: 13px; } "
        "QPushButton:hover { background: #ea580c; }");
    connect(btnImport, &QPushButton::clicked, this, &AccountParserPage::onImportFileClicked);

    auto *btnProcess = new QPushButton("⚡ Tách Chuỗi");
    btnProcess->setCursor(Qt::PointingHandCursor);
    btnProcess->setStyleSheet(
        "QPushButton { background: #ffffff; color: #0f172a; font-weight: 600; "
        "border: 1px solid #e2e8f0; border-radius: 8px; padding: 9px 14px; "
        "font-size: 13px; } QPushButton:hover { background: #f8fafc; }");
    connect(btnProcess, &QPushButton::clicked, this, &AccountParserPage::onProcessLinesClicked);

    auto *btnClear = new QPushButton("🗑️ Xóa Hết");
    btnClear->setCursor(Qt::PointingHandCursor);
    btnClear->setStyleSheet(
        "QPushButton { background: #ffffff; color: #ef4444; font-weight: 600; "
        "border: 1px solid #fecaca; border-radius: 8px; padding: 9px 14px; "
        "font-size: 13px; } QPushButton:hover { background: #fef2f2; }");
    connect(btnClear, &QPushButton::clicked, this, &AccountParserPage::onClearAllClicked);

    titleRow->addWidget(btnImport);
    titleRow->addWidget(btnProcess);
    titleRow->addWidget(btnClear);

    layout->addLayout(titleRow);

    // 2. Delimiter & Category Filter Row
    auto *filterRow = new QHBoxLayout();
    filterRow->setSpacing(12);

    m_delimiterCombo = new QComboBox();
    m_delimiterCombo->setView(new QListView(m_delimiterCombo));
    m_delimiterCombo->addItem("Phân cách: | (Pipe)", "|");
    m_delimiterCombo->addItem("Phân cách: : (Colon)", ":");
    m_delimiterCombo->addItem("Phân cách: , (Comma)", ",");
    m_delimiterCombo->addItem("Phân cách: ; (Semicolon)", ";");
    m_delimiterCombo->addItem("Phân cách: Tab", "\t");
    filterRow->addWidget(m_delimiterCombo, 1);

    m_formatCombo = new QComboBox();
    m_formatCombo->setView(new QListView(m_formatCombo));
    m_formatCombo->addItem("Mẫu: UID|Pass|2FA|Email|PassMail|Cookie|Extra");
    m_formatCombo->addItem("Mẫu: UID|Pass|2FA");
    m_formatCombo->addItem("Mẫu: Email|Pass|Recovery");
    m_formatCombo->addItem("Mẫu: User|Pass|Proxy");
    filterRow->addWidget(m_formatCombo, 1);

    connect(m_formatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AccountParserPage::onFormatChanged);
    connect(m_delimiterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this]() {
                if (!m_rawInputEdit->toPlainText().trimmed().isEmpty()) {
                    onProcessLinesClicked();
                }
            });

    filterRow->addStretch(2);
    layout->addLayout(filterRow);

    // 3. Pill Filter Category Buttons
    auto *pillRow = new QHBoxLayout();
    pillRow->setSpacing(8);

    QStringList pillTitles = {"Show All", "Hợp Lệ", "Lỗi Định Dạng", "Trùng Lặp"};
    m_pillButtons.clear();
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
        connect(btn, &QPushButton::clicked, [this, i]() { onPillFilterClicked(i); });
        m_pillButtons.append(btn);
        pillRow->addWidget(btn);
    }
    pillRow->addStretch();
    layout->addLayout(pillRow);

    // 4. Splitter: Raw Input Box + Table Preview
    auto *splitter = new QSplitter(Qt::Vertical);
    splitter->setHandleWidth(8);

    auto *inputBox = new QWidget();
    auto *inputBoxLayout = new QVBoxLayout(inputBox);
    inputBoxLayout->setContentsMargins(0, 0, 0, 0);
    inputBoxLayout->setSpacing(4);

    auto *lblInputDesc = new QLabel("📋 Dán danh sách tài khoản thô vào đây (Mỗi tài khoản 1 dòng):");
    lblInputDesc->setStyleSheet("font-size: 12px; font-weight: 600; color: #64748b;");
    inputBoxLayout->addWidget(lblInputDesc);

    m_rawInputEdit = new QPlainTextEdit();
    m_rawInputEdit->setPlaceholderText(
        "100083921029|Matkhau@123|JBSWY3DPEHPK3PXP|user1@gmail.com|passMail1|c_user=100083921029;...|TokenEAAB...\n"
        "100083921030|Matkhau@456|KZXW63TPMQQM2LXP|user2@gmail.com|passMail2|c_user=100083921030;...|TokenEAAB...\n"
        "100083921029|Matkhau@123|JBSWY3DPEHPK3PXP|user1@gmail.com|passMail1|c_user=100083921029;...|TokenEAAB...");
    inputBoxLayout->addWidget(m_rawInputEdit);
    splitter->addWidget(inputBox);

    auto *tableBox = new QWidget();
    auto *tableBoxLayout = new QVBoxLayout(tableBox);
    tableBoxLayout->setContentsMargins(0, 0, 0, 0);
    tableBoxLayout->setSpacing(4);

    auto *lblTableDesc = new QLabel("📊 Bảng dữ liệu sau khi tách chuỗi:");
    lblTableDesc->setStyleSheet("font-size: 12px; font-weight: 600; color: #64748b;");
    tableBoxLayout->addWidget(lblTableDesc);

    m_tableWidget = new QTableWidget();
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableWidget->horizontalHeader()->setStretchLastSection(true);
    m_tableWidget->verticalHeader()->setVisible(false);
    m_tableWidget->verticalHeader()->setDefaultSectionSize(42);
    m_tableWidget->setShowGrid(false);
    m_tableWidget->setAlternatingRowColors(true);

    onFormatChanged(0);

    tableBoxLayout->addWidget(m_tableWidget);
    splitter->addWidget(tableBox);

    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);

    layout->addWidget(splitter, 1);
}

void AccountParserPage::setupRightSummaryPanel() {
    m_rightPanelWidget = new QWidget(m_parentWidget);
    m_rightPanelWidget->setFixedWidth(370);
    m_rightPanelWidget->setStyleSheet("background-color: #ffffff; border-top-right-radius: "
                                      "20px; border-bottom-right-radius: 20px;");

    auto *layout = new QVBoxLayout(m_rightPanelWidget);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(14);

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

    auto *orderTitleRow = new QHBoxLayout();
    auto *orderIcon = new QLabel("👜");
    orderIcon->setStyleSheet("font-size: 16px;");
    auto *orderTitle = new QLabel("Order / Summary #01");
    orderTitle->setStyleSheet("font-size: 15px; font-weight: 800; color: #0f172a;");
    orderTitleRow->addWidget(orderIcon);
    orderTitleRow->addWidget(orderTitle);
    orderTitleRow->addStretch();
    layout->addLayout(orderTitleRow);

    auto *previewCard = new QWidget();
    previewCard->setStyleSheet("background: #f8fafc; border-radius: 12px; padding: 10px;");
    auto *previewCardLayout = new QVBoxLayout(previewCard);
    previewCardLayout->setSpacing(8);

    auto addSummaryItem = [](const QString &name, const QString &count, const QString &status) -> QWidget * {
        auto *item = new QWidget();
        auto *l = new QHBoxLayout(item);
        l->setContentsMargins(0, 0, 0, 0);
        auto *nameLbl = new QLabel(name);
        nameLbl->setStyleSheet("font-size: 12px; font-weight: 600; color: #334155;");
        auto *valLbl = new QLabel(count);
        valLbl->setStyleSheet(QString("font-size: 12px; font-weight: 700; color: %1;").arg(status));
        l->addWidget(nameLbl);
        l->addStretch();
        l->addWidget(valLbl);
        return item;
    };

    previewCardLayout->addWidget(addSummaryItem("File Nguồn:", "TXT / Clipboard", "#64748b"));
    previewCardLayout->addWidget(addSummaryItem("Mẫu Tách:", "UID|Pass|2FA...", "#64748b"));
    previewCardLayout->addWidget(addSummaryItem("Ký Tự Ngăn:", "| (Gạch đứng)", "#f97316"));
    layout->addWidget(previewCard);

    layout->addStretch();

    auto *summaryBox = new QVBoxLayout();
    summaryBox->setSpacing(8);

    auto addStatRow = [&](const QString &label, const QString &val, QLabel *&outLabel, const QString &color = "#0f172a") {
        auto *row = new QHBoxLayout();
        auto *lbl = new QLabel(label);
        lbl->setStyleSheet("font-size: 13px; color: #64748b; font-weight: 500;");
        outLabel = new QLabel(val);
        outLabel->setStyleSheet(QString("font-size: 13.5px; font-weight: 700; color: %1;").arg(color));
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

    auto *totalRow = new QHBoxLayout();
    auto *lblTotalText = new QLabel("Total Export :");
    lblTotalText->setStyleSheet("font-size: 15px; font-weight: 800; color: #0f172a;");
    m_lblTotalExport = new QLabel("0");
    m_lblTotalExport->setStyleSheet("font-size: 19px; font-weight: 800; color: #0f172a;");
    totalRow->addWidget(lblTotalText);
    totalRow->addStretch();
    totalRow->addWidget(m_lblTotalExport);
    summaryBox->addLayout(totalRow);

    layout->addLayout(summaryBox);
    layout->addSpacing(10);

    auto *btnKot = new QPushButton("⚡  Phân Tích & Tách Chuỗi");
    btnKot->setCursor(Qt::PointingHandCursor);
    btnKot->setFixedHeight(42);
    btnKot->setStyleSheet(
        "QPushButton { background-color: #0f172a; color: #ffffff; font-weight: "
        "700; border-radius: 10px; font-size: 13px; } QPushButton:hover { "
        "background-color: #1e293b; }");
    connect(btnKot, &QPushButton::clicked, this, &AccountParserPage::onProcessLinesClicked);
    layout->addWidget(btnKot);

    auto *btnRow = new QHBoxLayout();
    btnRow->setSpacing(10);

    auto *btnBill = new QPushButton("📊 Xuất Excel");
    btnBill->setCursor(Qt::PointingHandCursor);
    btnBill->setFixedHeight(40);
    btnBill->setStyleSheet(
        "QPushButton { background-color: #f97316; color: #ffffff; font-weight: "
        "700; border-radius: 10px; font-size: 13px; } QPushButton:hover { "
        "background-color: #ea580c; }");
    connect(btnBill, &QPushButton::clicked, this, &AccountParserPage::onExportExcelClicked);

    auto *btnPrint = new QPushButton("📋 Copy All");
    btnPrint->setCursor(Qt::PointingHandCursor);
    btnPrint->setFixedHeight(40);
    btnPrint->setStyleSheet(
        "QPushButton { background-color: #16a34a; color: #ffffff; font-weight: "
        "700; border-radius: 10px; font-size: 13px; } QPushButton:hover { "
        "background-color: #15803d; }");
    connect(btnPrint, &QPushButton::clicked, this, &AccountParserPage::onCopyValidClicked);

    btnRow->addWidget(btnBill, 1);
    btnRow->addWidget(btnPrint, 1);
    layout->addLayout(btnRow);
}

QString AccountParserPage::getDelimiter() const {
    return m_delimiterCombo ? m_delimiterCombo->currentData().toString() : "|";
}

QStringList AccountParserPage::getCurrentHeaders() const {
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
        return {"STT", "UID / Username", "Password", "2FA Secret", "Email", "Pass Mail", "Cookie / Extra", "Trạng Thái"};
    }
}

void AccountParserPage::onFormatChanged(int index) {
    Q_UNUSED(index);
    if (!m_tableWidget) return;

    QStringList headers = getCurrentHeaders();
    m_tableWidget->setColumnCount(headers.size());
    m_tableWidget->setHorizontalHeaderLabels(headers);

    m_tableWidget->setColumnWidth(0, 50);
    for (int i = 1; i < headers.size() - 1; ++i) {
        m_tableWidget->setColumnWidth(i, 130);
    }
    m_tableWidget->horizontalHeader()->setStretchLastSection(true);

    if (m_rawInputEdit && !m_rawInputEdit->toPlainText().trimmed().isEmpty()) {
        onProcessLinesClicked();
    } else if (!m_allAccounts.isEmpty()) {
        onPillFilterClicked(m_currentFilter);
    }
}

void AccountParserPage::onImportFileClicked() {
    QString filePath = QFileDialog::getOpenFileName(m_parentWidget, "Chọn file tài khoản MMO", "", "Text Files (*.txt *.csv);;All Files (*.*)");
    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        CustomMessageBox::critical(m_parentWidget, "Lỗi", "Không thể đọc file đã chọn!");
        return;
    }

    QTextStream in(&file);
    m_rawInputEdit->setPlainText(in.readAll());
    file.close();

    onProcessLinesClicked();
}

void AccountParserPage::onProcessLinesClicked() {
    QString content = m_rawInputEdit->toPlainText().trimmed();
    if (content.isEmpty()) {
        CustomMessageBox::warning(m_parentWidget, "Thông báo", "Vui lòng nhập hoặc dán danh sách tài khoản trước!");
        return;
    }

    QString delimiter = getDelimiter();
    QStringList headers = getCurrentHeaders();
    int numFields = headers.size() - 2;

    QStringList lines = content.split('\n');
    m_allAccounts.clear();

    QSet<QString> seenKeys;
    int totalCount = 0;
    int validCount = 0;
    int errorCount = 0;
    int duplicateCount = 0;

    for (const QString &rawLine : lines) {
        QString line = rawLine.trimmed();
        if (line.isEmpty()) continue;

        totalCount++;
        QStringList parts = line.split(delimiter);

        MmoAccount acc;
        for (int f = 0; f < numFields; ++f) {
            acc.fields.append(parts.value(f).trimmed());
        }

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

    CustomMessageBox::information(
        m_parentWidget, "Thành công",
        QString("Đã phân tích xong %1 dòng theo %2!\n- Hợp lệ: %3\n- Lỗi: %4\n- Trùng lặp: %5")
            .arg(totalCount)
            .arg(m_formatCombo ? m_formatCombo->currentText() : "Mẫu đã chọn")
            .arg(validCount)
            .arg(errorCount)
            .arg(duplicateCount));
}

void AccountParserPage::updateStats(int total, int valid, int error, int duplicates) {
    if (m_lblTotal) m_lblTotal->setText(QString::number(total));
    if (m_lblValid) m_lblValid->setText(QString::number(valid));
    if (m_lblError) m_lblError->setText(QString::number(error));
    if (m_lblDuplicates) m_lblDuplicates->setText(QString::number(duplicates));
    if (m_lblTotalExport) m_lblTotalExport->setText(QString("%1 acc").arg(valid));
}

void AccountParserPage::onPillFilterClicked(int filterIndex) {
    m_currentFilter = filterIndex;
    for (int i = 0; i < m_pillButtons.size(); ++i) {
        if (i == filterIndex) {
            m_pillButtons[i]->setStyleSheet("QPushButton { background-color: #f97316; color: "
                                           "#ffffff; font-weight: 700; border-radius: 8px; "
                                           "padding: 0 16px; border: none; }");
        } else {
            m_pillButtons[i]->setStyleSheet("QPushButton { background-color: #ffffff; color: "
                                           "#64748b; font-weight: 600; border: 1px solid "
                                           "#e2e8f0; border-radius: 8px; padding: 0 16px; } "
                                           "QPushButton:hover { background-color: #f8fafc; }");
        }
    }

    QList<MmoAccount> filtered;
    for (const auto &acc : m_allAccounts) {
        if (filterIndex == 1 && !acc.isValid) continue;
        if (filterIndex == 2 && (acc.isValid || acc.statusMsg == "Trùng lặp")) continue;
        if (filterIndex == 3 && acc.statusMsg != "Trùng lặp") continue;
        filtered.append(acc);
    }
    renderTable(filtered);
}

void AccountParserPage::renderTable(const QList<MmoAccount> &accounts) {
    m_tableWidget->setRowCount(0);
    int row = 0;
    QString search = m_searchEdit ? m_searchEdit->text().trimmed().toLower() : "";

    for (const auto &acc : accounts) {
        if (!search.isEmpty()) {
            bool match = false;
            for (const auto &f : acc.fields) {
                if (f.toLower().contains(search)) {
                    match = true;
                    break;
                }
            }
            if (!match) continue;
        }

        m_tableWidget->insertRow(row);
        auto *sttItem = new QTableWidgetItem(QString::number(row + 1));
        sttItem->setTextAlignment(Qt::AlignCenter);
        sttItem->setFont(QFont("Google Sans", 10, QFont::Bold));
        m_tableWidget->setItem(row, 0, sttItem);

        for (int c = 0; c < acc.fields.size(); ++c) {
            auto *item = new QTableWidgetItem(acc.fields[c]);
            if (c == 0) item->setFont(QFont("Google Sans", 10, QFont::Bold));
            m_tableWidget->setItem(row, c + 1, item);
        }

        int statusCol = m_tableWidget->columnCount() - 1;
        auto *statusItem = new QTableWidgetItem(acc.statusMsg);
        statusItem->setTextAlignment(Qt::AlignCenter);
        if (acc.isValid) {
            statusItem->setForeground(QColor("#16a34a"));
            statusItem->setFont(QFont("Google Sans", 10, QFont::Bold));
        } else {
            statusItem->setForeground(QColor("#ef4444"));
        }
        m_tableWidget->setItem(row, statusCol, statusItem);
        row++;
    }
}

void AccountParserPage::onSearchFilterChanged(const QString &text) {
    Q_UNUSED(text);
    onPillFilterClicked(m_currentFilter);
}

void AccountParserPage::onExportExcelClicked() {
    if (m_allAccounts.isEmpty()) {
        CustomMessageBox::warning(m_parentWidget, "Thông báo", "Không có dữ liệu để xuất file!");
        return;
    }

    int mode = m_exportFormatCombo ? m_exportFormatCombo->currentIndex() : 0;
    QString filter;
    QString defaultExt;
    if (mode == 0 || mode == 1) {
        filter = "CSV Files (*.csv);;All Files (*.*)";
        defaultExt = ".csv";
    } else if (mode == 2) {
        filter = "Tab Separated Values (*.tsv *.txt);;All Files (*.*)";
        defaultExt = ".tsv";
    } else {
        filter = "Text Files (*.txt);;All Files (*.*)";
        defaultExt = ".txt";
    }

    QString filePath = QFileDialog::getSaveFileName(m_parentWidget, "Lưu file kết quả xuất", "accounts_export" + defaultExt, filter);
    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        CustomMessageBox::critical(m_parentWidget, "Lỗi", "Không thể ghi file!");
        return;
    }

    QTextStream out(&file);
    QString delim = ",";
    if (mode == 1) delim = ";";
    else if (mode == 2) delim = "\t";
    else if (mode == 3) delim = "|";

    QStringList headers = getCurrentHeaders();
    headers.removeFirst();
    out << headers.join(delim) << "\n";

    int count = 0;
    for (const auto &acc : m_allAccounts) {
        if (!acc.isValid) continue;
        QStringList rowData = acc.fields;
        rowData.append(acc.statusMsg);
        out << rowData.join(delim) << "\n";
        count++;
    }

    file.close();
    CustomMessageBox::information(
        m_parentWidget, "Thành công",
        QString("Đã xuất thành công %1 tài khoản hợp lệ ra file:\n%2").arg(count).arg(filePath));
}

void AccountParserPage::onCopyValidClicked() {
    QString delim = getDelimiter();
    QStringList lines;
    for (const auto &acc : m_allAccounts) {
        if (acc.isValid) lines.append(acc.fields.join(delim));
    }

    if (lines.isEmpty()) {
        CustomMessageBox::warning(m_parentWidget, "Thông báo", "Không có tài khoản hợp lệ nào để copy!");
        return;
    }

    QApplication::clipboard()->setText(lines.join("\n"));
    CustomMessageBox::information(m_parentWidget, "Đã Copy", QString("Đã copy %1 tài khoản hợp lệ vào Clipboard!").arg(lines.size()));
}

void AccountParserPage::onClearAllClicked() {
    m_rawInputEdit->clear();
    m_tableWidget->setRowCount(0);
    m_allAccounts.clear();
    updateStats(0, 0, 0, 0);
}
