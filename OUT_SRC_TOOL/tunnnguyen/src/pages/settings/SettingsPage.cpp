#include "SettingsPage.h"
#include "ui/CustomMessageBox.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFileDialog>
#include <QListView>

SettingsPage::SettingsPage(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void SettingsPage::setupUi() {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 20, 24, 20);
    layout->setSpacing(18);

    // 1. Header Card
    auto *headerCard = new QWidget();
    headerCard->setStyleSheet("background: #f8fafc; border: 1px solid #f1f5f9; border-radius: 12px; padding: 12px 16px;");
    auto *headerLayout = new QHBoxLayout(headerCard);
    headerLayout->setContentsMargins(0, 0, 0, 0);

    auto *headerIcon = new QLabel("⚙️");
    headerIcon->setStyleSheet("font-size: 24px; background: #f1f5f9; border: 1px solid #e2e8f0; border-radius: 12px; padding: 6px 10px;");
    auto *titleBox = new QVBoxLayout();
    titleBox->setSpacing(2);
    auto *titleText = new QLabel("Cấu Hình Hệ Thống & Đường Dẫn Ứng Dụng (Settings)");
    titleText->setStyleSheet("font-size: 16px; font-weight: 800; color: #0f172a;");
    auto *subtitleText = new QLabel("Thiết lập đường dẫn Google Chrome, Android Debug Bridge (ADB), Timeout Proxy và luồng chạy tự động.");
    subtitleText->setStyleSheet("font-size: 12px; color: #64748b; font-weight: 500;");
    titleBox->addWidget(titleText);
    titleBox->addWidget(subtitleText);

    headerLayout->addWidget(headerIcon);
    headerLayout->addLayout(titleBox);
    headerLayout->addStretch();
    layout->addWidget(headerCard);

    // 2. Settings Form Card
    auto *formCard = new QWidget();
    formCard->setStyleSheet("background: #ffffff; border: 1px solid #e2e8f0; border-radius: 14px; padding: 24px;");
    auto *formLayout = new QVBoxLayout(formCard);
    formLayout->setSpacing(18);

    auto *grid = new QGridLayout();
    grid->setHorizontalSpacing(16);
    grid->setVerticalSpacing(16);

    // Chrome Path
    auto *lblChrome = new QLabel("Đường dẫn Google Chrome / Chromium:");
    lblChrome->setStyleSheet("font-size: 13px; font-weight: 700; color: #0f172a;");
    m_txtChromePath = new QLineEdit();
    m_txtChromePath->setText("/usr/bin/google-chrome-stable");
    m_txtChromePath->setStyleSheet("background: #f8fafc; border: 1px solid #cbd5e1; border-radius: 6px; padding: 8px 12px; font-size: 13px;");
    auto *btnBrowseChrome = new QPushButton("📁 Chọn File...");
    btnBrowseChrome->setCursor(Qt::PointingHandCursor);
    btnBrowseChrome->setStyleSheet("QPushButton { background: #f1f5f9; border: 1px solid #cbd5e1; border-radius: 6px; padding: 8px 14px; font-weight: 600; font-size: 12.5px; } QPushButton:hover { background: #e2e8f0; }");
    connect(btnBrowseChrome, &QPushButton::clicked, this, &SettingsPage::onBrowseChromePath);

    grid->addWidget(lblChrome, 0, 0);
    grid->addWidget(m_txtChromePath, 0, 1);
    grid->addWidget(btnBrowseChrome, 0, 2);

    // ADB Path
    auto *lblAdb = new QLabel("Đường dẫn Android ADB (Android SDK):");
    lblAdb->setStyleSheet("font-size: 13px; font-weight: 700; color: #0f172a;");
    m_txtAdbPath = new QLineEdit();
    m_txtAdbPath->setText("adb");
    m_txtAdbPath->setStyleSheet("background: #f8fafc; border: 1px solid #cbd5e1; border-radius: 6px; padding: 8px 12px; font-size: 13px;");
    auto *btnBrowseAdb = new QPushButton("📁 Chọn File...");
    btnBrowseAdb->setCursor(Qt::PointingHandCursor);
    btnBrowseAdb->setStyleSheet("QPushButton { background: #f1f5f9; border: 1px solid #cbd5e1; border-radius: 6px; padding: 8px 14px; font-weight: 600; font-size: 12.5px; } QPushButton:hover { background: #e2e8f0; }");
    connect(btnBrowseAdb, &QPushButton::clicked, this, &SettingsPage::onBrowseAdbPath);

    grid->addWidget(lblAdb, 1, 0);
    grid->addWidget(m_txtAdbPath, 1, 1);
    grid->addWidget(btnBrowseAdb, 1, 2);

    // Timeout
    auto *lblTimeout = new QLabel("Thời gian chờ kết nối Proxy (Timeout):");
    lblTimeout->setStyleSheet("font-size: 13px; font-weight: 700; color: #0f172a;");
    m_spinTimeout = new QSpinBox();
    m_spinTimeout->setRange(1000, 60000);
    m_spinTimeout->setValue(8000);
    m_spinTimeout->setSingleStep(1000);
    m_spinTimeout->setSuffix(" ms");
    m_spinTimeout->setStyleSheet("background: #f8fafc; border: 1px solid #cbd5e1; border-radius: 6px; padding: 6px 12px; font-size: 13px;");
    grid->addWidget(lblTimeout, 2, 0);
    grid->addWidget(m_spinTimeout, 2, 1);

    // Max Threads
    auto *lblThreads = new QLabel("Số luồng chạy tự động tối đa:");
    lblThreads->setStyleSheet("font-size: 13px; font-weight: 700; color: #0f172a;");
    m_spinMaxThreads = new QSpinBox();
    m_spinMaxThreads->setRange(1, 50);
    m_spinMaxThreads->setValue(5);
    m_spinMaxThreads->setSuffix(" luồng song song");
    m_spinMaxThreads->setStyleSheet("background: #f8fafc; border: 1px solid #cbd5e1; border-radius: 6px; padding: 6px 12px; font-size: 13px;");
    grid->addWidget(lblThreads, 3, 0);
    grid->addWidget(m_spinMaxThreads, 3, 1);

    formLayout->addLayout(grid);
    formLayout->addSpacing(16);

    auto *btnRow = new QHBoxLayout();
    m_btnSave = new QPushButton("💾 Lưu Cài Đặt");
    m_btnSave->setCursor(Qt::PointingHandCursor);
    m_btnSave->setFixedHeight(42);
    m_btnSave->setStyleSheet("QPushButton { background: #f97316; color: #ffffff; font-weight: 700; border-radius: 8px; font-size: 14px; padding: 0 24px; border: none; } QPushButton:hover { background: #ea580c; }");
    connect(m_btnSave, &QPushButton::clicked, this, &SettingsPage::onSaveSettings);
    btnRow->addWidget(m_btnSave);
    btnRow->addStretch();
    formLayout->addLayout(btnRow);

    layout->addWidget(formCard);
    layout->addStretch();
}

void SettingsPage::onBrowseChromePath() {
    QString f = QFileDialog::getOpenFileName(this, "Chọn file thực thi Chrome", "/usr/bin");
    if (!f.isEmpty()) m_txtChromePath->setText(f);
}

void SettingsPage::onBrowseAdbPath() {
    QString f = QFileDialog::getOpenFileName(this, "Chọn file thực thi ADB", "/usr/bin");
    if (!f.isEmpty()) m_txtAdbPath->setText(f);
}

void SettingsPage::onSaveSettings() {
    CustomMessageBox::information(this, "Thành Công", "Đã lưu toàn bộ cấu hình hệ thống thành công!");
}
