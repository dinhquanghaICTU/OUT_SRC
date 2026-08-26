#include "SensorDetailDialog.h"
#include "VirtualKeyboard.h"

#include <QChart>
#include <QChartView>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QLineSeries>
#include <QPainter>
#include <QPushButton>
#include <QSpinBox>
#include <QStackedWidget>
#include <QTableWidget>
#include <QTimer>
#include <QValueAxis>
#include <QVBoxLayout>

SensorDetailDialog::SensorDetailDialog(const QString &sensorName,
                                       const QString &unit,
                                       const QString &accentColor,
                                       const QVector<SensorDataPoint> &history,
                                       QWidget *parent,
                                       double minThreshold,
                                       double maxThreshold)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("Chi tiết cảm biến: %1").arg(sensorName));
    resize(700, 390);
    setMinimumSize(480, 300);
    setStyleSheet(
        "QDialog { background-color: #0c1218; color: #e2e8f0; font-family: 'Segoe UI', sans-serif; } "
        "QLabel { color: #e2e8f0; } "
        "QHeaderView::section { background-color: #111c26; color: #00f0ff; font-weight: 800; font-size: 11px; padding: 6px; border: none; } "
        "QTableWidget { background-color: #070b10; color: #ffffff; gridline-color: #1e293b; border: 1px solid #1e293b; border-radius: 8px; font-size: 12px; } "
        "QTableWidget::item { padding: 4px; } "
        "QTableWidget::item:selected { background-color: #0284c7; color: #ffffff; } "
        "QDoubleSpinBox, QSpinBox { background-color: #070b10; color: #ffffff; border: 1.5px solid #1e293b; border-radius: 6px; font-size: 12px; font-weight: 700; padding: 4px 8px; min-height: 28px; } "
        "QDoubleSpinBox:focus, QSpinBox:focus { border: 1.5px solid #00f0ff; background-color: #0c141d; } "
        "QCheckBox { color: #94a3b8; font-size: 11px; font-weight: 700; spacing: 8px; }"
    );

    setupUI(sensorName, unit, accentColor, minThreshold, maxThreshold);
    populateData(history, unit, accentColor);
}

void SensorDetailDialog::setupUI(const QString &sensorName, const QString &unit, const QString &accentColor,
                                double minThreshold, double maxThreshold)
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 10, 12, 10);
    mainLayout->setSpacing(8);

    // --- Header Row ---
    auto *headerRow = new QHBoxLayout;
    auto *titleLbl = new QLabel(QStringLiteral("⬢ Cảm biến: %1 (%2)").arg(sensorName, unit));
    titleLbl->setStyleSheet(QStringLiteral("color: %1; font-size: 14px; font-weight: 900;").arg(accentColor));
    headerRow->addWidget(titleLbl);
    headerRow->addStretch();

    m_chartModeBtn = new QPushButton(QStringLiteral("📈 Biểu đồ"));
    m_tableModeBtn = new QPushButton(QStringLiteral("📋 Dạng Bảng"));
    m_thresholdModeBtn = new QPushButton(QStringLiteral("⚙ Cài Đặt Ngưỡng"));
    m_chartModeBtn->setCheckable(true);
    m_tableModeBtn->setCheckable(true);
    m_thresholdModeBtn->setCheckable(true);
    m_chartModeBtn->setChecked(true);

    const QString btnStyle =
        "QPushButton { background: #0c1218; color: #64748b; border: 1px solid #162430; border-radius: 5px; padding: 5px 10px; font-size: 10px; font-weight: 800; } "
        "QPushButton:checked { background: #0284c7; color: #ffffff; border: 1px solid #38bdf8; font-weight: 900; } "
        "QPushButton:hover { background: #131d26; color: #ffffff; }";

    m_chartModeBtn->setStyleSheet(btnStyle);
    m_tableModeBtn->setStyleSheet(btnStyle);
    m_thresholdModeBtn->setStyleSheet(btnStyle);

    headerRow->addWidget(m_chartModeBtn);
    headerRow->addWidget(m_tableModeBtn);
    headerRow->addWidget(m_thresholdModeBtn);

    auto *closeBtn = new QPushButton(QStringLiteral("✕ ĐÓNG"));
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet("QPushButton { background: #dc2626; color: #ffffff; border: none; border-radius: 5px; font-weight: 900; font-size: 10px; padding: 5px 12px; } QPushButton:hover { background: #b91c1c; }");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    headerRow->addWidget(closeBtn);

    mainLayout->addLayout(headerRow);

    // --- View Stack ---
    m_viewStack = new QStackedWidget;

    // 1. Chart View
    auto *chartContainer = new QFrame;
    chartContainer->setStyleSheet("background: #141024; border: 1px solid #382c5c; border-radius: 8px;");
    auto *chartLayout = new QVBoxLayout(chartContainer);
    chartLayout->setContentsMargins(8, 8, 8, 8);
    m_chartView = new QChartView;
    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_chartView->setStyleSheet("background: transparent; border: none;");
    chartLayout->addWidget(m_chartView);
    m_viewStack->addWidget(chartContainer);

    // 2. Table View
    m_tableWidget = new QTableWidget;
    m_tableWidget->setColumnCount(4);
    m_tableWidget->setHorizontalHeaderLabels({QStringLiteral("STT"), QStringLiteral("Thời gian ghi"), QStringLiteral("Giá trị"), QStringLiteral("Trạng thái")});
    m_tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_tableWidget->verticalHeader()->hide();
    m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_viewStack->addWidget(m_tableWidget);

    // 3. Threshold Config View
    auto *threshContainer = new QFrame;
    threshContainer->setStyleSheet("background: #141024; border: 1px solid #382c5c; border-radius: 10px;");
    auto *threshLayout = new QVBoxLayout(threshContainer);
    threshLayout->setContentsMargins(24, 18, 24, 18);
    threshLayout->setSpacing(14);

    auto *threshHead = new QLabel(QStringLiteral("⚙ Cài đặt ngưỡng tự động kích hoạt Đèn (%1)").arg(sensorName));
    threshHead->setStyleSheet("color: #f59e0b; font-size: 14px; font-weight: 900;");
    threshLayout->addWidget(threshHead);

    auto *formGrid = new QGridLayout;
    formGrid->setHorizontalSpacing(16);
    formGrid->setVerticalSpacing(12);

    formGrid->addWidget(new QLabel(QStringLiteral("Ngưỡng Bật Đèn (Ánh sáng < Min %1):").arg(unit)), 0, 0);
    m_minThresholdSpin = new QDoubleSpinBox;
    m_minThresholdSpin->setRange(0.0, 99999.0);
    m_minThresholdSpin->setValue(minThreshold);
    m_minThresholdSpin->setDecimals(1);
    if (auto *le = m_minThresholdSpin->findChild<QLineEdit*>()) {
        VirtualKeyboardDialog::attachToLineEdit(le, QStringLiteral("Nhập Ngưỡng Bật Đèn (Min Lux)"));
    }
    formGrid->addWidget(m_minThresholdSpin, 0, 1);

    formGrid->addWidget(new QLabel(QStringLiteral("Ngưỡng Tắt Đèn (Ánh sáng > Max %1):").arg(unit)), 1, 0);
    m_maxThresholdSpin = new QDoubleSpinBox;
    m_maxThresholdSpin->setRange(0.0, 99999.0);
    m_maxThresholdSpin->setValue(maxThreshold);
    m_maxThresholdSpin->setDecimals(1);
    if (auto *le = m_maxThresholdSpin->findChild<QLineEdit*>()) {
        VirtualKeyboardDialog::attachToLineEdit(le, QStringLiteral("Nhập Ngưỡng Tắt Đèn (Max Lux)"));
    }
    formGrid->addWidget(m_maxThresholdSpin, 1, 1);

    formGrid->addWidget(new QLabel(QStringLiteral("Chế độ thông minh kết hợp PIR:")), 2, 0);
    m_autoLightCheck = new QCheckBox(QStringLiteral("Chỉ bật đèn khi phòng tối + có người chuyển động"));
    m_autoLightCheck->setChecked(true);
    formGrid->addWidget(m_autoLightCheck, 2, 1);

    formGrid->addWidget(new QLabel(QStringLiteral("Thời gian duy trì sáng sau khi rời đi:")), 3, 0);
    m_intervalSpin = new QSpinBox;
    m_intervalSpin->setRange(5, 300);
    m_intervalSpin->setValue(30);
    m_intervalSpin->setSuffix(QStringLiteral(" giây"));
    if (auto *le = m_intervalSpin->findChild<QLineEdit*>()) {
        VirtualKeyboardDialog::attachToLineEdit(le, QStringLiteral("Thời gian duy trì sáng (giây)"));
    }
    formGrid->addWidget(m_intervalSpin, 3, 1);

    threshLayout->addLayout(formGrid);

    // Action buttons
    auto *btnRow = new QHBoxLayout;
    auto *saveBtn = new QPushButton(QStringLiteral("💾 Lưu Cấu Hình"));
    saveBtn->setCursor(Qt::PointingHandCursor);
    saveBtn->setStyleSheet("QPushButton { background: #f59e0b; color: #000000; border: none; border-radius: 6px; font-size: 12px; font-weight: 900; padding: 8px 18px; } QPushButton:hover { background: #fbbf24; }");

    m_saveStatusLbl = new QLabel;
    m_saveStatusLbl->setStyleSheet("color: #10b981; font-weight: 800; font-size: 11px;");

    connect(saveBtn, &QPushButton::clicked, this, [this] {
        emit thresholdChanged(m_minThresholdSpin->value(), m_maxThresholdSpin->value());
        m_saveStatusLbl->setText(QStringLiteral("✓ Đã lưu cấu hình ngưỡng thành công!"));
        QTimer::singleShot(2000, this, [this] { m_saveStatusLbl->clear(); });
    });

    btnRow->addWidget(saveBtn);
    btnRow->addWidget(m_saveStatusLbl);
    btnRow->addStretch();
    threshLayout->addLayout(btnRow);
    threshLayout->addStretch();

    m_viewStack->addWidget(threshContainer);
    mainLayout->addWidget(m_viewStack, 1);

    connect(m_chartModeBtn, &QPushButton::clicked, this, [this] {
        m_viewStack->setCurrentIndex(0);
        m_chartModeBtn->setChecked(true);
        m_tableModeBtn->setChecked(false);
        m_thresholdModeBtn->setChecked(false);
    });

    connect(m_tableModeBtn, &QPushButton::clicked, this, [this] {
        m_viewStack->setCurrentIndex(1);
        m_chartModeBtn->setChecked(false);
        m_tableModeBtn->setChecked(true);
        m_thresholdModeBtn->setChecked(false);
    });

    connect(m_thresholdModeBtn, &QPushButton::clicked, this, [this] {
        m_viewStack->setCurrentIndex(2);
        m_chartModeBtn->setChecked(false);
        m_tableModeBtn->setChecked(false);
        m_thresholdModeBtn->setChecked(true);
    });
}

void SensorDetailDialog::populateData(const QVector<SensorDataPoint> &history, const QString &unit, const QString &accentColor)
{
    m_tableWidget->setRowCount(0);
    int rowIdx = 0;
    for (int i = history.size() - 1; i >= 0; --i) {
        const auto &pt = history[i];
        m_tableWidget->insertRow(rowIdx);
        m_tableWidget->setRowHeight(rowIdx, 30);

        m_tableWidget->setItem(rowIdx, 0, new QTableWidgetItem(QString::number(rowIdx + 1)));
        m_tableWidget->setItem(rowIdx, 1, new QTableWidgetItem(pt.timestamp.toString(QStringLiteral("HH:mm:ss dd/MM/yyyy"))));
        m_tableWidget->setItem(rowIdx, 2, new QTableWidgetItem(QStringLiteral("%1 %2").arg(QString::number(pt.value, 'f', 1), unit)));

        auto *statusItem = new QTableWidgetItem(QStringLiteral("✓ Chuẩn"));
        statusItem->setForeground(QColor(QStringLiteral("#10b981")));
        m_tableWidget->setItem(rowIdx, 3, statusItem);
        rowIdx++;
    }

    if (history.isEmpty()) {
        m_tableWidget->insertRow(0);
        auto *emptyItem = new QTableWidgetItem(QStringLiteral("Chưa có dữ liệu từ cảm biến ESP32 Tuấn Anh"));
        emptyItem->setTextAlignment(Qt::AlignCenter);
        m_tableWidget->setItem(0, 1, emptyItem);
    }

    auto *chart = new QChart;
    chart->setBackgroundBrush(Qt::transparent);
    chart->legend()->hide();

    auto *series = new QLineSeries;
    series->setColor(QColor(accentColor));
    QPen pen(QColor(accentColor), 2.5);
    series->setPen(pen);

    double minVal = 999999.0;
    double maxVal = -999999.0;

    for (int i = 0; i < history.size(); ++i) {
        series->append(i, history[i].value);
        if (history[i].value < minVal) minVal = history[i].value;
        if (history[i].value > maxVal) maxVal = history[i].value;
    }

    if (history.isEmpty()) {
        series->append(0, 0);
        minVal = 0;
        maxVal = 500;
    }

    chart->addSeries(series);

    auto *axisX = new QValueAxis;
    axisX->setRange(0, qMax(10, history.size() - 1));
    axisX->setLabelFormat("%d");
    axisX->setTitleText(QStringLiteral("Mẫu đo gần nhất"));
    axisX->setLabelsColor(QColor(QStringLiteral("#94a3b8")));
    axisX->setTitleBrush(QColor(QStringLiteral("#94a3b8")));
    axisX->setGridLineColor(QColor(QStringLiteral("#382c5c")));

    auto *axisY = new QValueAxis;
    axisY->setRange(qMax(0.0, minVal - 10.0), maxVal + 20.0);
    axisY->setTitleText(QStringLiteral("Giá trị (%1)").arg(unit));
    axisY->setLabelsColor(QColor(QStringLiteral("#94a3b8")));
    axisY->setTitleBrush(QColor(QStringLiteral("#94a3b8")));
    axisY->setGridLineColor(QColor(QStringLiteral("#382c5c")));

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisX);
    series->attachAxis(axisY);

    m_chartView->setChart(chart);
}
