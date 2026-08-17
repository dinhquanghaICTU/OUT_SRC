#include "SensorDetailDialog.h"

#include <QChart>
#include <QChartView>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
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
    setWindowTitle(QStringLiteral("Chi tiết & Cài đặt ngưỡng: %1").arg(sensorName));
    resize(700, 390);
    setMinimumSize(480, 300);
    setStyleSheet(
        "QDialog { background-color: #070d1e; color: #ecf2ff; font-family: sans-serif; } "
        "QLabel { color: #f1f5f9; } "
        "QHeaderView::section { background-color: #111d3d; color: #94a3b8; font-weight: 800; font-size: 11px; padding: 6px; border: none; } "
        "QTableWidget { background-color: #0c1630; color: #ffffff; gridline-color: #1c2b54; border: 1px solid #1c2b54; border-radius: 8px; font-size: 12px; } "
        "QTableWidget::item { padding: 4px; } "
        "QTableWidget::item:selected { background-color: #1e3a8a; color: #ffffff; } "
        "QDoubleSpinBox, QSpinBox { background-color: #0f1c3f; color: #ffffff; border: 1.5px solid #233870; border-radius: 8px; font-size: 13px; font-weight: 700; padding: 4px 8px; min-height: 28px; } "
        "QDoubleSpinBox:focus, QSpinBox:focus { border: 2px solid #38bdf8; background-color: #162447; } "
        "QCheckBox { color: #cbd5e1; font-size: 11px; font-weight: 700; spacing: 8px; }"
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
    auto *titleLbl = new QLabel(QStringLiteral("Cảm biến: %1 (%2)").arg(sensorName, unit));
    titleLbl->setStyleSheet(QStringLiteral("color: %1; font-size: 14px; font-weight: 900;").arg(accentColor));
    headerRow->addWidget(titleLbl);
    headerRow->addStretch();

    // Mode Toggle Buttons (Biểu đồ / Bảng / Chỉnh ngưỡng)
    m_chartModeBtn = new QPushButton(QStringLiteral("📈 Biểu đồ"));
    m_tableModeBtn = new QPushButton(QStringLiteral("📋 Dạng Bảng"));
    m_thresholdModeBtn = new QPushButton(QStringLiteral("⚙ Chỉnh Ngưỡng"));
    m_chartModeBtn->setCheckable(true);
    m_tableModeBtn->setCheckable(true);
    m_thresholdModeBtn->setCheckable(true);
    m_chartModeBtn->setChecked(true);

    const QString btnStyle =
        "QPushButton { background: #131d3d; color: #94a3b8; border: 1px solid #233565; border-radius: 6px; padding: 5px 10px; font-size: 10px; font-weight: 800; } "
        "QPushButton:checked { background: #10b981; color: #ffffff; border: 1px solid #34d399; font-weight: 900; } "
        "QPushButton:hover { background: #1c2b54; color: #ffffff; }";

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

    // --- Stack for Chart / Table / Threshold Config ---
    m_viewStack = new QStackedWidget;

    // 1. Chart View
    auto *chartContainer = new QFrame;
    chartContainer->setStyleSheet("background: #0d1733; border: 1px solid #1c2b54; border-radius: 8px;");
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
    m_tableWidget->setHorizontalHeaderLabels({QStringLiteral("STT"), QStringLiteral("Thời gian ghi"), QStringLiteral("Giá trị"), QStringLiteral("Đánh giá")});
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
    threshContainer->setStyleSheet("background: #0d1733; border: 1px solid #1c2b54; border-radius: 10px;");
    auto *threshLayout = new QVBoxLayout(threshContainer);
    threshLayout->setContentsMargins(24, 18, 24, 18);
    threshLayout->setSpacing(14);

    auto *threshHead = new QLabel(QStringLiteral("⚙ Cài đặt ngưỡng cảnh báo & tự động kích hoạt Bơm (%1)").arg(sensorName));
    threshHead->setStyleSheet("color: #38bdf8; font-size: 14px; font-weight: 900;");
    threshLayout->addWidget(threshHead);

    auto *formGrid = new QGridLayout;
    formGrid->setHorizontalSpacing(16);
    formGrid->setVerticalSpacing(12);

    // Min Threshold
    formGrid->addWidget(new QLabel(QStringLiteral("Ngưỡng Dưới (Min %1):").arg(unit)), 0, 0);
    m_minThresholdSpin = new QDoubleSpinBox;
    m_minThresholdSpin->setRange(0.0, 9999.0);
    m_minThresholdSpin->setValue(minThreshold);
    m_minThresholdSpin->setDecimals(1);
    formGrid->addWidget(m_minThresholdSpin, 0, 1);

    // Max Threshold
    formGrid->addWidget(new QLabel(QStringLiteral("Ngưỡng Trên (Max %1):").arg(unit)), 1, 0);
    m_maxThresholdSpin = new QDoubleSpinBox;
    m_maxThresholdSpin->setRange(0.0, 9999.0);
    m_maxThresholdSpin->setValue(maxThreshold);
    m_maxThresholdSpin->setDecimals(1);
    formGrid->addWidget(m_maxThresholdSpin, 1, 1);

    // Sampling Interval
    formGrid->addWidget(new QLabel(QStringLiteral("Chu kỳ lấy mẫu (giây):")), 2, 0);
    m_intervalSpin = new QSpinBox;
    m_intervalSpin->setRange(1, 60);
    m_intervalSpin->setValue(2);
    formGrid->addWidget(m_intervalSpin, 2, 1);

    threshLayout->addLayout(formGrid);

    // Auto Pump Trigger Checkbox
    m_autoPumpCheck = new QCheckBox(QStringLiteral("Tự động BẬT máy bơm khi vượt ra ngoài khoảng ngưỡng"));
    m_autoPumpCheck->setChecked(true);
    threshLayout->addWidget(m_autoPumpCheck);

    // Save Button & Status Label
    auto *btnRow = new QHBoxLayout;
    auto *saveBtn = new QPushButton(QStringLiteral("💾 Lưu Cấu Hình Ngưỡng"));
    saveBtn->setCursor(Qt::PointingHandCursor);
    saveBtn->setMinimumHeight(36);
    saveBtn->setStyleSheet(
        "QPushButton { background: #10b981; color: #ffffff; border: none; border-radius: 8px; font-size: 13px; font-weight: 900; padding: 0 18px; } "
        "QPushButton:hover { background: #059669; } "
        "QPushButton:pressed { background: #047857; }"
    );
    m_saveStatusLbl = new QLabel(QStringLiteral(""));
    m_saveStatusLbl->setStyleSheet("color: #10b981; font-weight: 800; font-size: 12px;");

    connect(saveBtn, &QPushButton::clicked, this, [this] {
        emit thresholdChanged(m_minThresholdSpin->value(), m_maxThresholdSpin->value());
        m_saveStatusLbl->setText(QStringLiteral("✓ Đã lưu cấu hình ngưỡng thành công!"));
        QTimer::singleShot(3000, this, [this] { m_saveStatusLbl->clear(); });
    });

    btnRow->addWidget(saveBtn);
    btnRow->addWidget(m_saveStatusLbl);
    btnRow->addStretch();
    threshLayout->addLayout(btnRow);
    threshLayout->addStretch();

    m_viewStack->addWidget(threshContainer);

    mainLayout->addWidget(m_viewStack, 1);

    // Button toggle logic
    connect(m_chartModeBtn, &QPushButton::clicked, this, [this] {
        m_chartModeBtn->setChecked(true);
        m_tableModeBtn->setChecked(false);
        m_thresholdModeBtn->setChecked(false);
        m_viewStack->setCurrentIndex(0);
    });
    connect(m_tableModeBtn, &QPushButton::clicked, this, [this] {
        m_tableModeBtn->setChecked(true);
        m_chartModeBtn->setChecked(false);
        m_thresholdModeBtn->setChecked(false);
        m_viewStack->setCurrentIndex(1);
    });
    connect(m_thresholdModeBtn, &QPushButton::clicked, this, [this] {
        m_thresholdModeBtn->setChecked(true);
        m_chartModeBtn->setChecked(false);
        m_tableModeBtn->setChecked(false);
        m_viewStack->setCurrentIndex(2);
    });
}

void SensorDetailDialog::populateData(const QVector<SensorDataPoint> &history,
                                      const QString &unit,
                                      const QString &accentColor)
{
    // 1. Populate Table
    m_tableWidget->setRowCount(history.size());
    double minVal = 999999.0;
    double maxVal = -999999.0;

    auto *series = new QLineSeries;
    series->setPen(QPen(QColor(accentColor), 2.8));

    for (int i = 0; i < history.size(); ++i) {
        const auto &pt = history[i];
        minVal = qMin(minVal, pt.value);
        maxVal = qMax(maxVal, pt.value);

        // Table Rows
        auto *itemIdx = new QTableWidgetItem(QString::number(i + 1));
        itemIdx->setTextAlignment(Qt::AlignCenter);
        auto *itemTime = new QTableWidgetItem(pt.timestamp.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")));
        auto *itemVal = new QTableWidgetItem(QStringLiteral("%1 %2").arg(pt.value, 0, 'f', 2).arg(unit));
        itemVal->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        auto *itemStatus = new QTableWidgetItem(QStringLiteral("Bình thường"));
        itemStatus->setTextAlignment(Qt::AlignCenter);
        itemStatus->setForeground(QBrush(QColor(QStringLiteral("#22c55e"))));

        m_tableWidget->setItem(i, 0, itemIdx);
        m_tableWidget->setItem(i, 1, itemTime);
        m_tableWidget->setItem(i, 2, itemVal);
        m_tableWidget->setItem(i, 3, itemStatus);

        series->append(i, pt.value);
    }

    if (history.isEmpty()) {
        minVal = 0.0;
        maxVal = 100.0;
    }

    // 2. Populate Chart
    auto *chart = new QChart;
    chart->setBackgroundVisible(false);
    chart->setMargins(QMargins(10, 10, 10, 10));
    chart->legend()->hide();

    auto *axisX = new QValueAxis(chart);
    axisX->setRange(0, qMax(10, history.size() - 1));
    axisX->setTickCount(5);
    axisX->setLabelFormat(QStringLiteral("%d"));
    axisX->setGridLineColor(QColor(QStringLiteral("#1e293b")));
    axisX->setLabelsColor(QColor(QStringLiteral("#94a3b8")));

    auto *axisY = new QValueAxis(chart);
    axisY->setRange(minVal - 2.0, maxVal + 2.0);
    axisY->setTickCount(5);
    axisY->setLabelFormat(QStringLiteral("%.1f"));
    axisY->setTitleText(unit);
    axisY->setTitleBrush(QBrush(QColor(QStringLiteral("#94a3b8"))));
    axisY->setGridLineColor(QColor(QStringLiteral("#1e293b")));
    axisY->setLabelsColor(QColor(QStringLiteral("#94a3b8")));

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    chart->addSeries(series);
    series->attachAxis(axisX);
    series->attachAxis(axisY);

    m_chartView->setChart(chart);
}
