#include "DashboardPage.h"
#include "ui_DashboardPage.h"

#include <QChart>
#include <QChartView>
#include <QDate>
#include <QDateTime>
#include <QFrame>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QLineSeries>
#include <QLocale>
#include <QPainter>
#include <QPushButton>
#include <QStyle>
#include <QTableWidget>
#include <QValueAxis>
#include <QVBoxLayout>

namespace {

QLabel *label(const QString &text, const char *name = nullptr)
{
    auto *result = new QLabel(text);
    if (name)
        result->setObjectName(QString::fromLatin1(name));
    result->setWordWrap(true);
    return result;
}

QFrame *card(const char *name = "dashboardSmartCard")
{
    auto *result = new QFrame;
    result->setObjectName(QString::fromLatin1(name));
    result->setFrameShape(QFrame::NoFrame);
    result->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    return result;
}

QLabel *chip(const QString &text, const QString &icon)
{
    auto *result = label(icon + QStringLiteral("  ") + text, "dashboardWeatherChip");
    result->setAlignment(Qt::AlignCenter);
    result->setMinimumHeight(62);
    return result;
}

QPushButton *tabButton(const QString &text, bool active)
{
    auto *button = new QPushButton(text);
    button->setObjectName(active ? QStringLiteral("dashboardTabActive")
                                 : QStringLiteral("dashboardTab"));
    button->setCursor(Qt::PointingHandCursor);
    button->setCheckable(true);
    button->setChecked(active);
    return button;
}

QPushButton *sceneButton(const QString &icon, const QString &text, const QString &type)
{
    auto *button = new QPushButton(icon + QStringLiteral("  ") + text);
    button->setObjectName(QStringLiteral("dashboardScene_%1").arg(type));
    button->setCursor(Qt::PointingHandCursor);
    button->setCheckable(true);
    button->setMinimumHeight(48);
    return button;
}

QChartView *chartView(QLineSeries *series, const QString &accent, double minimum, double maximum)
{
    auto *chart = new QChart;
    chart->addSeries(series);
    chart->legend()->hide();
    chart->setBackgroundVisible(false);
    chart->setMargins(QMargins(0, 0, 0, 0));

    auto *axisX = new QValueAxis(chart);
    axisX->setRange(0, 20);
    axisX->setTickCount(5);
    axisX->setLabelFormat(QStringLiteral("%d"));
    axisX->setGridLineVisible(false);
    axisX->setLabelsColor(QColor(QStringLiteral("#83918b")));

    auto *axisY = new QValueAxis(chart);
    axisY->setRange(minimum, maximum);
    axisY->setTickCount(4);
    axisY->setGridLineColor(QColor(QStringLiteral("#e4ebe7")));
    axisY->setLabelsColor(QColor(QStringLiteral("#83918b")));

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisX);
    series->attachAxis(axisY);
    series->setPen(QPen(QColor(accent), 2.8));

    auto *view = new QChartView(chart);
    view->setRenderHint(QPainter::Antialiasing);
    view->setStyleSheet(QStringLiteral("background: transparent; border: none;"));
    return view;
}

QWidget *metricPill(const QString &title, QLabel **valueLabel, const QString &unit)
{
    auto *pill = new QFrame;
    pill->setObjectName(QStringLiteral("dashboardMetricPill"));
    auto *layout = new QVBoxLayout(pill);
    layout->setContentsMargins(14, 10, 14, 10);
    layout->setSpacing(3);
    layout->addWidget(label(title, "dashboardHint"));
    auto *row = new QHBoxLayout;
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(4);
    *valueLabel = label(QStringLiteral("--"), "dashboardMetricSmallValue");
    row->addWidget(*valueLabel);
    row->addWidget(label(unit, "dashboardMetricSmallUnit"), 0, Qt::AlignBottom);
    row->addStretch();
    layout->addLayout(row);
    return pill;
}

} // namespace

DashboardPage::DashboardPage(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::DashboardPage),
      m_pressureSeries(new QLineSeries(this)),
      m_distanceSeries(new QLineSeries(this))
{
    ui->setupUi(this);

    auto *header = new QHBoxLayout;
    header->setContentsMargins(0, 0, 0, 0);
    header->setSpacing(12);
    auto *titleBlock = new QVBoxLayout;
    titleBlock->setContentsMargins(0, 0, 0, 0);
    titleBlock->setSpacing(6);
    m_titleLabel = label(tr("Nhà của bạn"), "dashboardTitle");
    titleBlock->addWidget(m_titleLabel);
    titleBlock->addWidget(label(tr("Tổng quan hệ thống môi trường ICTU"), "dashboardSubtitle"));
    header->addLayout(titleBlock);
    header->addStretch();
    m_updatedAt = label(tr("Chưa có dữ liệu"), "dashboardUpdatedAt");
    header->addWidget(m_updatedAt, 0, Qt::AlignTop);
    ui->verticalLayout->addLayout(header);

    auto *tabs = new QHBoxLayout;
    tabs->setContentsMargins(0, 0, 0, 4);
    tabs->setSpacing(8);
    auto *tabOverview = tabButton(tr("Tổng hợp"), true);
    auto *tabFavorite = tabButton(tr("Yêu thích"), false);
    auto *tabHistory = tabButton(tr("Lịch sử"), false);
    const auto fakeTabClick = [tabOverview, tabFavorite, tabHistory](QPushButton *active) {
        for (QPushButton *button : {tabOverview, tabFavorite, tabHistory})
            button->setChecked(button == active);
    };
    connect(tabOverview, &QPushButton::clicked, this, [=] { fakeTabClick(tabOverview); });
    connect(tabFavorite, &QPushButton::clicked, this, [=] { fakeTabClick(tabFavorite); });
    connect(tabHistory, &QPushButton::clicked, this, [=] { fakeTabClick(tabHistory); });
    tabs->addWidget(tabOverview);
    tabs->addWidget(tabFavorite);
    tabs->addWidget(tabHistory);
    tabs->addStretch();
    ui->verticalLayout->addLayout(tabs);

    auto *grid = new QGridLayout;
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setHorizontalSpacing(16);
    grid->setVerticalSpacing(16);
    grid->setColumnStretch(0, 1);
    grid->setColumnStretch(1, 1);

    auto *weather = card("dashboardWeatherCard");
    auto *weatherLayout = new QVBoxLayout(weather);
    weatherLayout->setContentsMargins(22, 20, 22, 20);
    weatherLayout->setSpacing(14);
    auto *weatherTop = new QHBoxLayout;
    weatherTop->setContentsMargins(0, 0, 0, 0);
    auto *weatherIcon = label(QStringLiteral("☁"), "dashboardWeatherIcon");
    weatherIcon->setAlignment(Qt::AlignCenter);
    weatherTop->addWidget(weatherIcon);
    weatherTop->addStretch();
    m_temperatureChip = chip(tr("-- °C"), QStringLiteral("🌡"));
    m_pressureChip = chip(tr("-- hPa"), QStringLiteral("☁"));
    m_distanceChip = chip(tr("-- cm"), QStringLiteral("💧"));
    weatherTop->addWidget(m_temperatureChip);
    weatherTop->addWidget(m_pressureChip);
    weatherTop->addWidget(m_distanceChip);
    weatherLayout->addLayout(weatherTop);
    m_clockValue = label(QDateTime::currentDateTime().toString(QStringLiteral("H:mm")),
                         "dashboardClock");
    m_dateValue = label(QLocale(QLocale::Vietnamese, QLocale::Vietnam)
                            .toString(QDate::currentDate(), QStringLiteral("dddd, dd/MM/yyyy")),
                        "dashboardDate");
    weatherLayout->addWidget(m_clockValue);
    weatherLayout->addWidget(m_dateValue);
    grid->addWidget(weather, 0, 0);

    auto *scenes = card("dashboardSceneCard");
    auto *scenesLayout = new QGridLayout(scenes);
    scenesLayout->setContentsMargins(18, 18, 18, 18);
    scenesLayout->setHorizontalSpacing(12);
    scenesLayout->setVerticalSpacing(12);
    auto *sleepScene = sceneButton(QStringLiteral("◐"), tr("Đi ngủ"), QStringLiteral("sleep"));
    auto *autoScene = sceneButton(QStringLiteral("◈"), tr("Tự động"), QStringLiteral("auto"));
    auto *securityScene = sceneButton(QStringLiteral("!"), tr("An ninh"), QStringLiteral("security"));
    auto *travelScene = sceneButton(QStringLiteral("▰"), tr("Du lịch"), QStringLiteral("travel"));
    auto *rainScene = sceneButton(QStringLiteral("☔"), tr("Ngày mưa"), QStringLiteral("rain"));
    auto *readScene = sceneButton(QStringLiteral("▣"), tr("Đọc sách"), QStringLiteral("read"));
    autoScene->setChecked(true);
    scenesLayout->addWidget(sleepScene, 0, 0);
    scenesLayout->addWidget(autoScene, 0, 1);
    scenesLayout->addWidget(securityScene, 1, 0);
    scenesLayout->addWidget(travelScene, 1, 1);
    scenesLayout->addWidget(rainScene, 2, 0);
    scenesLayout->addWidget(readScene, 2, 1);
    grid->addWidget(scenes, 0, 1);

    auto *media = card("dashboardMediaCard");
    auto *mediaLayout = new QVBoxLayout(media);
    mediaLayout->setContentsMargins(18, 18, 18, 18);
    mediaLayout->setSpacing(12);
    auto *mediaHeader = new QHBoxLayout;
    auto *cover = label(QStringLiteral("↯"), "dashboardCoverArt");
    cover->setAlignment(Qt::AlignCenter);
    mediaHeader->addWidget(cover);
    auto *mediaText = new QVBoxLayout;
    mediaText->addWidget(label(tr("Giám sát môi trường"), "dashboardCardTitle"));
    mediaText->addWidget(label(tr("Dữ liệu realtime từ cảm biến"), "dashboardHint"));
    mediaHeader->addLayout(mediaText, 1);
    mediaLayout->addLayout(mediaHeader);
    auto *metricRow = new QHBoxLayout;
    metricRow->setSpacing(10);
    metricRow->addWidget(metricPill(tr("Áp suất"), &m_pressureValue, tr("hPa")));
    metricRow->addWidget(metricPill(tr("Khoảng cách"), &m_distanceValue, tr("cm")));
    mediaLayout->addLayout(metricRow);
    auto *chartRow = new QHBoxLayout;
    chartRow->setContentsMargins(0, 0, 0, 0);
    chartRow->setSpacing(8);
    chartRow->addWidget(chartView(m_pressureSeries, QStringLiteral("#3566c5"), 985, 1035), 1);
    chartRow->addWidget(chartView(m_distanceSeries, QStringLiteral("#15945a"), 0, 100), 1);
    mediaLayout->addLayout(chartRow, 1);
    grid->addWidget(media, 1, 0);

    auto *camera = card("dashboardCameraCard");
    auto *cameraLayout = new QVBoxLayout(camera);
    cameraLayout->setContentsMargins(20, 18, 20, 18);
    cameraLayout->setSpacing(12);
    auto *cameraHead = new QHBoxLayout;
    cameraHead->addWidget(label(QStringLiteral("←  ") + tr("Phòng thiết bị"), "dashboardCameraTitle"));
    cameraHead->addStretch();
    cameraHead->addWidget(label(QStringLiteral("LIVE"), "dashboardLiveBadge"));
    cameraLayout->addLayout(cameraHead);
    cameraLayout->addStretch();
    m_alertValue = label(tr("Hệ thống ổn định"), "dashboardCameraStatus");
    cameraLayout->addWidget(m_alertValue);
    auto *cameraActions = new QHBoxLayout;
    cameraActions->addStretch();
    auto *deviceDemo = tabButton(tr("Thiết bị"), true);
    auto *cameraDemo = tabButton(tr("Cameras"), false);
    connect(deviceDemo, &QPushButton::clicked, this, [=] {
        deviceDemo->setChecked(true);
        cameraDemo->setChecked(false);
        m_alertValue->setText(tr("Hệ thống ổn định"));
    });
    connect(cameraDemo, &QPushButton::clicked, this, [=] {
        deviceDemo->setChecked(false);
        cameraDemo->setChecked(true);
        m_alertValue->setText(tr("Camera demo đang sẵn sàng"));
    });
    cameraActions->addWidget(deviceDemo);
    cameraActions->addWidget(cameraDemo);
    cameraLayout->addLayout(cameraActions);
    grid->addWidget(camera, 1, 1);

    auto *historyCard = card("dashboardHistoryCard");
    auto *historyLayout = new QVBoxLayout(historyCard);
    historyLayout->setContentsMargins(18, 16, 18, 14);
    historyLayout->setSpacing(10);
    auto *historyHeader = new QHBoxLayout;
    historyHeader->addWidget(label(tr("Lịch sử dữ liệu"), "dashboardCardTitle"));
    historyHeader->addStretch();
    auto *search = new QLineEdit;
    search->setObjectName(QStringLiteral("dashboardSearch"));
    search->setPlaceholderText(tr("⌕  Tìm dữ liệu"));
    historyHeader->addWidget(search);
    historyLayout->addLayout(historyHeader);
    m_history = new QTableWidget(0, 3);
    m_history->setObjectName(QStringLiteral("dashboardHistory"));
    m_history->setHorizontalHeaderLabels({tr("Thời gian"), tr("Áp suất"), tr("Khoảng cách")});
    m_history->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_history->verticalHeader()->hide();
    m_history->setShowGrid(false);
    m_history->setSelectionMode(QAbstractItemView::NoSelection);
    historyLayout->addWidget(m_history, 1);
    grid->addWidget(historyCard, 2, 0, 1, 2);

    ui->verticalLayout->addLayout(grid, 1);

    const QList<QPair<double, double>> demo = {
        {1008,25},{1016,31},{1005,28},{1010,29},{1008,24},{1015,30},
        {1021,38},{1017,33},{1024,33},{1014,39},{1012,35},{1017,44},{1011,39}
    };
    for (const auto &point : demo) {
        m_pressureSeries->append(m_sampleIndex, point.first);
        m_distanceSeries->append(m_sampleIndex, point.second);
        ++m_sampleIndex;
    }

    m_updatedAt->setText(tr("Cập nhật: %1").arg(
        QLocale(QLocale::Vietnamese, QLocale::Vietnam)
            .toString(QDateTime::currentDateTime(), QStringLiteral("HH:mm, dddd, dd/MM/yyyy"))));
}

DashboardPage::~DashboardPage()
{
    delete ui;
}

void DashboardPage::setUsername(const QString &username)
{
    const QString displayName = username.trimmed().isEmpty() ? tr("bạn") : username.trimmed();
    m_titleLabel->setText(tr("Nhà của %1").arg(displayName));
}

void DashboardPage::appendSeriesPoint(QLineSeries *series, double value,
                                      double fallbackMin, double fallbackMax)
{
    series->append(m_sampleIndex, value);
    while (series->count() > 24)
        series->remove(0);

    if (auto *axisX = qobject_cast<QValueAxis *>(series->chart()->axes(Qt::Horizontal).value(0)))
        axisX->setRange(qMax(0, m_sampleIndex - 23), qMax(23, m_sampleIndex));

    if (auto *axisY = qobject_cast<QValueAxis *>(series->chart()->axes(Qt::Vertical).value(0))) {
        double minValue = value;
        double maxValue = value;
        const auto points = series->points();
        for (const QPointF &point : points) {
            minValue = qMin(minValue, point.y());
            maxValue = qMax(maxValue, point.y());
        }
        if (qFuzzyCompare(minValue, maxValue)) {
            minValue = fallbackMin;
            maxValue = fallbackMax;
        } else {
            const double pad = qMax(1.0, (maxValue - minValue) * 0.18);
            minValue -= pad;
            maxValue += pad;
        }
        axisY->setRange(minValue, maxValue);
    }
}

void DashboardPage::updateReading(const SensorReading &reading)
{
    const QDateTime measured = reading.measuredAt.isValid()
        ? reading.measuredAt.toLocalTime()
        : QDateTime::currentDateTime();

    m_clockValue->setText(measured.toString(QStringLiteral("H:mm")));
    m_dateValue->setText(QLocale(QLocale::Vietnamese, QLocale::Vietnam)
                             .toString(measured.date(), QStringLiteral("dddd, dd/MM/yyyy")));
    m_pressureChip->setText(QStringLiteral("☁  %1 hPa").arg(reading.pressureHpa, 0, 'f', 1));
    m_distanceChip->setText(QStringLiteral("💧  %1 cm").arg(reading.distanceCm, 0, 'f', 1));
    m_temperatureChip->setText(QStringLiteral("🌡  %1 °C").arg(reading.temperatureC, 0, 'f', 1));
    m_pressureValue->setText(QString::number(reading.pressureHpa, 'f', 1));
    m_distanceValue->setText(QString::number(reading.distanceCm, 'f', 1));
    m_updatedAt->setText(tr("Cập nhật: %1").arg(
        QLocale(QLocale::Vietnamese, QLocale::Vietnam)
            .toString(measured, QStringLiteral("HH:mm, dddd, dd/MM/yyyy"))));

    const bool warning = reading.pressureHpa < 990.0 || reading.pressureHpa > 1030.0
        || reading.distanceCm < 20.0;
    m_alertValue->setText(warning ? tr("Có cảnh báo cần kiểm tra")
                                  : tr("Hệ thống ổn định"));
    m_alertValue->setProperty("warning", warning);
    m_alertValue->style()->unpolish(m_alertValue);
    m_alertValue->style()->polish(m_alertValue);

    appendSeriesPoint(m_pressureSeries, reading.pressureHpa, 985, 1035);
    appendSeriesPoint(m_distanceSeries, reading.distanceCm, 0, 100);
    ++m_sampleIndex;
    addHistory(reading);
}

void DashboardPage::addHistory(const SensorReading &reading)
{
    const QDateTime measured = reading.measuredAt.isValid()
        ? reading.measuredAt.toLocalTime()
        : QDateTime::currentDateTime();

    m_history->insertRow(0);
    m_history->setItem(0, 0, new QTableWidgetItem(
        measured.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"))));
    m_history->setItem(0, 1, new QTableWidgetItem(
        QString::number(reading.pressureHpa, 'f', 1) + QStringLiteral(" hPa")));
    m_history->setItem(0, 2, new QTableWidgetItem(
        QString::number(reading.distanceCm, 'f', 1) + QStringLiteral(" cm")));
    while (m_history->rowCount() > 5)
        m_history->removeRow(m_history->rowCount() - 1);
}
