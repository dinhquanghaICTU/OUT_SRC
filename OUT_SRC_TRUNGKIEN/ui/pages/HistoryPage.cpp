#include "HistoryPage.h"

#include "ui_HistoryPage.h"

#include <QBarCategoryAxis>
#include <QBarSeries>
#include <QBarSet>
#include <QButtonGroup>
#include <QChart>
#include <QChartView>
#include <QDate>
#include <QDateTimeAxis>
#include <QDialog>
#include <QEvent>
#include <QFrame>
#include <QGridLayout>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QLegend>
#include <QLegendMarker>
#include <QLineSeries>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QResizeEvent>
#include <QStackedWidget>
#include <QTableWidgetItem>
#include <QTimer>
#include <QVBoxLayout>
#include <QValueAxis>

#include <limits>

namespace {
static QColor getColorForKey(const QString &k, int fallbackIdx = 0) {
  if (k == QStringLiteral("pressure_hpa"))
    return QColor("#38bdf8"); // Cyan Áp suất
  if (k == QStringLiteral("uv_index"))
    return QColor("#f59e0b"); // Cam UV Index
  if (k == QStringLiteral("uv_voltage"))
    return QColor("#a855f7"); // Tím Điện áp UV
  if (k == QStringLiteral("temperature_c"))
    return QColor("#10b981"); // Xanh lục Nhiệt độ
  static const QList<QColor> fallback{QColor("#38bdf8"), QColor("#f59e0b"),
                                      QColor("#a855f7"), QColor("#10b981")};
  return fallback.at(fallbackIdx % fallback.size());
}

static QString metricShortName(const QString &key) {
  static const QHash<QString, QString> names{
      {QStringLiteral("pressure_hpa"), QObject::tr("Áp suất")},
      {QStringLiteral("uv_index"), QObject::tr("Tia UV")},
      {QStringLiteral("uv_voltage"), QObject::tr("Điện áp UV")},
      {QStringLiteral("temperature_c"), QObject::tr("Nhiệt độ")}};
  return names.value(key, key);
}
} // namespace

HistoryPage::HistoryPage(QWidget *parent)
    : QWidget(parent), ui(new Ui::HistoryPage), m_chart(new QChart),
      m_chartView(new QChartView(m_chart, this)),
      m_primaryStat(new QLabel(this)), m_secondaryStat(new QLabel(this)),
      m_thirdStat(new QLabel(this)), m_chartHint(new QLabel(this)),
      m_headerSubtitle(new QLabel(this)), m_analyticsGrid(new QGridLayout),
      m_chartCard(new QFrame(this)), m_primaryStatCard(nullptr),
      m_secondaryStatCard(nullptr), m_summaryStatCard(nullptr) {
  ui->setupUi(this);
  m_headerSubtitle->hide();
  setStyleSheet(
      "QWidget#HistoryPage { background-color: #070d1e; color: #ecf2ff; "
      "font-family: sans-serif; } "
      "QLabel#historyRecordBadge { background: #0e1938; color: #38bdf8; "
      "border: 1px solid #1e293b; border-radius: 4px; padding: 2px 6px; "
      "font-size: 10px; font-weight: 700; } "
      "QPushButton#deviceViewTabButton { background: #0e1938; color: #94a3b8; "
      "border: 1px solid #223565; border-radius: 6px; padding: 3px 6px; "
      "font-size: 10px; font-weight: 800; } "
      "QPushButton#deviceViewTabButton:checked { background: #0284c7; color: "
      "#ffffff; border-color: #38bdf8; font-weight: 900; } "
      "QPushButton#historySearchButton { background: #10b981; color: #ffffff; "
      "border: none; border-radius: 6px; font-size: 10px; font-weight: 900; "
      "padding: 3px 8px; } "
      "QComboBox { background-color: #0f1c3f; color: #ffffff; border: 1px "
      "solid #233870; border-radius: 6px; padding: 2px 4px; font-size: 10px; } "
      "QDateEdit { background-color: #0f1c3f; color: #ffffff; border: 1px "
      "solid #233870; border-radius: 6px; padding: 2px 4px; font-size: 10px; } "
      "QFrame#historyChartCard { background-color: #0d1733; border: 1px solid "
      "#1c2b54; border-radius: 8px; } "
      "QFrame#historyStatCard { background-color: #0e1938; border: 1px solid "
      "#223565; border-radius: 6px; } "
      "QLabel#historyStatTitle { color: #94a3b8; font-size: 9px; font-weight: "
      "700; } "
      "QLabel#historyStatValue { color: #38bdf8; font-size: 12px; font-weight: "
      "900; } "
      "QTableWidget#historyTableSmart { background-color: #0c1630; color: "
      "#ffffff; gridline-color: #1c2b54; border: 1px solid #1c2b54; "
      "border-radius: 8px; font-size: 10px; } "
      "QTableWidget#historyTableSmart QHeaderView::section { background-color: "
      "#111d3d; color: #94a3b8; font-weight: 800; font-size: 10px; padding: "
      "4px; border: none; }");

  ui->recordCountLabel->setObjectName(QStringLiteral("historyRecordBadge"));
  ui->chartTabButton->setObjectName(QStringLiteral("deviceViewTabButton"));
  ui->tableTabButton->setObjectName(QStringLiteral("deviceViewTabButton"));
  ui->deviceCombo->setObjectName(QStringLiteral("historyDeviceCombo"));
  ui->periodCombo->setObjectName(QStringLiteral("historyPeriodCombo"));
  ui->dateCombo->setObjectName(QStringLiteral("historyDateCombo"));
  ui->searchButton->setObjectName(QStringLiteral("historySearchButton"));
  ui->navBarLayout->setSpacing(8);
  ui->filterLayout->setSpacing(8);

  auto *tabGroup = new QButtonGroup(this);
  tabGroup->addButton(ui->chartTabButton);
  tabGroup->addButton(ui->tableTabButton);
  tabGroup->setExclusive(true);

  m_metricCombo = new QComboBox(this);
  m_metricCombo->setObjectName(QStringLiteral("historyMetricCombo"));
  m_metricCombo->setMinimumWidth(125);
  m_metricCombo->hide();

  m_zoomBtn = new QPushButton(tr("Phóng to"), this);
  m_zoomBtn->setObjectName(QStringLiteral("historyZoomButton"));
  m_zoomBtn->setCursor(Qt::PointingHandCursor);
  m_zoomBtn->setToolTip(tr("Phóng to biểu đồ"));
  connect(m_zoomBtn, &QPushButton::clicked, this,
          [this] { openChartZoomDialog(m_selectedMetricKey); });

  ui->filterLayout->addWidget(m_metricCombo);
  ui->filterLayout->addWidget(m_zoomBtn);

  connect(ui->chartTabButton, &QPushButton::clicked, this, [this] {
    ui->viewStack->setCurrentIndex(0);
    if (m_metricCombo && m_metricCombo->count() > 0)
      m_metricCombo->show();
    if (m_zoomBtn)
      m_zoomBtn->show();
  });
  connect(ui->tableTabButton, &QPushButton::clicked, this, [this] {
    ui->viewStack->setCurrentIndex(1);
    if (m_metricCombo)
      m_metricCombo->hide();
    if (m_zoomBtn)
      m_zoomBtn->hide();
  });

  ui->historyTable->horizontalHeader()->setSectionResizeMode(
      QHeaderView::Stretch);
  ui->historyTable->verticalHeader()->hide();
  ui->historyTable->setObjectName(QStringLiteral("historyTableSmart"));

  auto makeStatCard = [this](QLabel *&titleOut, const QString &defaultTitle,
                             QLabel *value) {
    auto *card = new QFrame(this);
    card->setObjectName(QStringLiteral("historyStatCard"));
    card->setCursor(Qt::PointingHandCursor);
    card->setFixedHeight(48);
    card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(10, 4, 10, 4);
    layout->setSpacing(2);

    titleOut = new QLabel(defaultTitle, card);
    titleOut->setObjectName(QStringLiteral("historyStatTitle"));
    value->setObjectName(QStringLiteral("historyStatValue"));
    value->setText(QStringLiteral("--"));
    value->setWordWrap(false);
    layout->addWidget(titleOut);
    layout->addWidget(value);
    card->installEventFilter(this);
    return card;
  };

  m_chartView->setObjectName(QStringLiteral("historyChart"));
  m_chartView->setMinimumHeight(190);
  m_chartView->setRenderHint(QPainter::Antialiasing);
  m_chartView->setCursor(Qt::PointingHandCursor);
  m_chartView->setToolTip(tr("Chạm vào biểu đồ để phóng to"));
  m_chartView->setStyleSheet(
      QStringLiteral("background: #070d1e; border: none;"));
  m_chartView->viewport()->installEventFilter(this);

  m_chart->setBackgroundBrush(QBrush(QColor("#070d1e")));
  m_chart->setPlotAreaBackgroundBrush(QBrush(QColor("#0c1630")));
  m_chart->setPlotAreaBackgroundVisible(true);
  m_chart->setTitle(QString());
  m_chart->setAnimationOptions(QChart::SeriesAnimations);
  m_chart->legend()->setAlignment(Qt::AlignBottom);
  m_chart->legend()->setLabelColor(QColor("#ffffff"));
  m_chart->setMargins(QMargins(2, 2, 2, 2));
  m_chart->setBackgroundRoundness(0);

  QFont legFont;
  legFont.setPixelSize(9);
  m_chart->legend()->setFont(legFont);

  m_chartCard->setObjectName(QStringLiteral("historyChartCard"));
  auto *chartLayout = new QVBoxLayout(m_chartCard);
  chartLayout->setContentsMargins(10, 6, 10, 6);
  chartLayout->setSpacing(4);

  m_chartHeaderLayout = new QHBoxLayout;
  m_chartHeaderLayout->setContentsMargins(2, 0, 2, 0);
  m_chartHeaderLayout->setSpacing(4);

  m_chartTitle = new QLabel(tr("Dữ liệu cảm biến"), m_chartCard);
  m_chartTitle->setObjectName(QStringLiteral("historyCardTitle"));
  m_chartHint = new QLabel(m_chartCard);
  m_chartHint->setObjectName(QStringLiteral("historyChartHint"));
  m_chartHint->hide();
  m_chartHeaderLayout->addWidget(m_chartTitle, 1);
  m_chartHeaderLayout->addWidget(m_chartHint);

  chartLayout->addLayout(m_chartHeaderLayout);
  chartLayout->addWidget(m_chartView, 1);

  connect(m_metricCombo, &QComboBox::currentIndexChanged, this, [this](int) {
    if (m_metricCombo->currentIndex() >= 0) {
      m_selectedMetricKey = m_metricCombo->currentData().toString();
      updateChart();
    }
  });

  m_analyticsGrid->setContentsMargins(0, 0, 0, 0);
  m_analyticsGrid->setHorizontalSpacing(8);
  m_analyticsGrid->setVerticalSpacing(0);
  m_primaryStatCard = makeStatCard(m_primaryStatTitle,
                                   tr("ÁP SUẤT KHÍ QUYỂN (TB)"), m_primaryStat);
  m_secondaryStatCard = makeStatCard(
      m_secondaryStatTitle, tr("CƯỜNG ĐỘ TIA UV (TB)"), m_secondaryStat);
  m_summaryStatCard = makeStatCard(m_summaryStatTitle,
                                   tr("ĐIỆN ÁP CẢM BIẾN (TB)"), m_thirdStat);
  m_analyticsGrid->addWidget(m_primaryStatCard, 0, 0);
  m_analyticsGrid->addWidget(m_secondaryStatCard, 0, 1);
  m_analyticsGrid->addWidget(m_summaryStatCard, 0, 2);

  // Build Chart View Page into ui->chartPage
  auto *chartPageLayout = new QVBoxLayout(ui->chartPage);
  chartPageLayout->setContentsMargins(0, 0, 0, 0);
  chartPageLayout->setSpacing(6);
  chartPageLayout->addWidget(m_chartCard, 1);
  chartPageLayout->addLayout(m_analyticsGrid);

  ui->viewStack->setCurrentIndex(0);
  applyResponsiveLayout();

  m_liveTimer = new QTimer(this);
  m_liveTimer->setInterval(2000);
  connect(m_liveTimer, &QTimer::timeout, this,
          &HistoryPage::requestCurrentHistory);

  m_selectedDate = QDate::currentDate();
  ui->dateCombo->setMinimumWidth(130);
  ui->periodCombo->setMinimumWidth(75);
  ui->periodCombo->setItemData(0, QStringLiteral("day"));
  ui->periodCombo->setItemData(1, QStringLiteral("month"));
  ui->periodCombo->setItemData(2, QStringLiteral("year"));

  rebuildDateOptions();

  connect(ui->searchButton, &QPushButton::clicked, this,
          &HistoryPage::requestCurrentHistory);
  connect(ui->deviceCombo, &QComboBox::currentIndexChanged, this,
          [this](int) { requestCurrentHistory(); });
  connect(ui->periodCombo, &QComboBox::currentIndexChanged, this, [this](int) {
    rebuildDateOptions();
    requestCurrentHistory();
  });
  connect(ui->dateCombo, &QComboBox::currentIndexChanged, this,
          [this](int idx) {
            if (idx >= 0) {
              const QVariant dVal = ui->dateCombo->itemData(idx);
              if (dVal.isValid() && dVal.canConvert<QDate>()) {
                m_selectedDate = dVal.toDate();
              }
              requestCurrentHistory();
            }
          });
}

bool HistoryPage::eventFilter(QObject *watched, QEvent *event) {
  if (event->type() == QEvent::MouseButtonPress ||
      event->type() == QEvent::MouseButtonDblClick) {
    if (m_chartView && watched == m_chartView->viewport()) {
      openChartZoomDialog(m_selectedMetricKey);
      return true;
    }
    if (watched == m_primaryStatCard) {
      QStringList plotable;
      for (const QJsonValue &k : m_cachedKeys)
        if (k.toString() != QStringLiteral("ir_detected"))
          plotable.append(k.toString());
      openChartZoomDialog(plotable.value(0, m_selectedMetricKey));
      return true;
    }
    if (watched == m_secondaryStatCard) {
      QStringList plotable;
      for (const QJsonValue &k : m_cachedKeys)
        if (k.toString() != QStringLiteral("ir_detected"))
          plotable.append(k.toString());
      openChartZoomDialog(plotable.value(1, m_selectedMetricKey));
      return true;
    }
    if (watched == m_summaryStatCard) {
      QStringList plotable;
      for (const QJsonValue &k : m_cachedKeys)
        if (k.toString() != QStringLiteral("ir_detected"))
          plotable.append(k.toString());
      openChartZoomDialog(plotable.value(2, m_selectedMetricKey));
      return true;
    }
  }
  return QWidget::eventFilter(watched, event);
}

void HistoryPage::resizeEvent(QResizeEvent *event) {
  QWidget::resizeEvent(event);
  applyResponsiveLayout();
}

void HistoryPage::applyResponsiveLayout() {
  const int pageWidth = contentsRect().width();
  const bool compact = pageWidth <= 800;

  ui->deviceCombo->setMinimumWidth(compact ? 150 : 220);
  ui->deviceCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  ui->periodCombo->setMinimumWidth(compact ? 65 : 85);
  ui->dateCombo->setMinimumWidth(compact ? 130 : 160);
  ui->searchButton->setMinimumWidth(compact ? 48 : 70);
  if (m_metricCombo)
    m_metricCombo->setMinimumWidth(compact ? 120 : 150);
  if (m_zoomBtn)
    m_zoomBtn->setMinimumWidth(compact ? 75 : 90);
  m_chartView->setMinimumHeight(compact ? 190 : 250);
  ui->historyTable->setMinimumHeight(compact ? 110 : 170);
  ui->verticalLayout->setContentsMargins(compact ? 8 : 14, compact ? 6 : 10,
                                         compact ? 8 : 14, compact ? 6 : 10);
  ui->verticalLayout->setSpacing(compact ? 6 : 8);
}

HistoryPage::~HistoryPage() { delete ui; }

void HistoryPage::setDevices(const QJsonArray &devices) {
  const QString selected = ui->deviceCombo->currentData().toString();
  ui->deviceCombo->blockSignals(true);
  ui->deviceCombo->clear();
  m_deviceOnline.clear();
  for (const QJsonValue &value : devices) {
    const QJsonObject device = value.toObject();
    const QString id = device.value(QStringLiteral("device_id")).toString();
    const QString name = device.value(QStringLiteral("name")).toString();
    const QString type = device.value(QStringLiteral("device_type")).toString();
    const QString addedBy = device.value(QStringLiteral("added_by")).toString();
    const bool online = device.value(QStringLiteral("online")).toBool(false);
    m_deviceOnline.insert(id, online);

    QString itemText = name;
    if (itemText.contains(QStringLiteral("UV"), Qt::CaseInsensitive)) {
      itemText = QStringLiteral("Trạm UV & Áp Suất");
    } else if (itemText.isEmpty()) {
      itemText = id;
    }
    ui->deviceCombo->addItem(itemText, id);
    ui->deviceCombo->setItemData(ui->deviceCombo->count() - 1, type,
                                 Qt::UserRole + 1);
    ui->deviceCombo->setItemData(
        ui->deviceCombo->count() - 1,
        QStringLiteral("%1 (%2)%3 · %4")
            .arg(name, id,
                 addedBy.isEmpty() ? QString()
                                   : tr(" · Thêm bởi: %1").arg(addedBy),
                 online ? tr("Trực tuyến") : tr("Ngoại tuyến")),
        Qt::ToolTipRole);
  }
  const int previous = ui->deviceCombo->findData(selected);
  if (previous >= 0)
    ui->deviceCombo->setCurrentIndex(previous);
  ui->deviceCombo->blockSignals(false);
  requestCurrentHistory();
}

QDate HistoryPage::selectedDate() const {
  if (ui && ui->dateCombo) {
    const QVariant val = ui->dateCombo->currentData();
    if (val.isValid() && val.canConvert<QDate>()) {
      return val.toDate();
    }
  }
  return m_selectedDate.isValid() ? m_selectedDate : QDate::currentDate();
}

void HistoryPage::rebuildDateOptions() {
  if (!ui || !ui->dateCombo)
    return;

  ui->dateCombo->blockSignals(true);
  ui->dateCombo->clear();

  const QString period = ui->periodCombo->currentData().toString();
  const QDate today = QDate::currentDate();
  if (!m_selectedDate.isValid()) {
    m_selectedDate = today;
  }

  if (period == QStringLiteral("year")) {
    const int startYear = today.year();
    for (int y = startYear; y >= startYear - 5; --y) {
      const QDate d(y, 1, 1);
      const QString label = (y == today.year()) ? tr("Năm %1 (Hiện tại)").arg(y)
                                                : tr("Năm %1").arg(y);
      ui->dateCombo->addItem(label, d);
    }
  } else if (period == QStringLiteral("month")) {
    const QDate curMonth(today.year(), today.month(), 1);
    for (int i = 0; i < 24; ++i) {
      const QDate d = curMonth.addMonths(-i);
      const QString label =
          (i == 0) ? tr("Tháng %1 (Hiện tại)")
                         .arg(d.toString(QStringLiteral("MM/yyyy")))
                   : tr("Tháng %1").arg(d.toString(QStringLiteral("MM/yyyy")));
      ui->dateCombo->addItem(label, d);
    }
  } else {
    for (int i = 0; i < 31; ++i) {
      const QDate d = today.addDays(-i);
      QString label;
      if (i == 0) {
        label = tr("Hôm nay (%1)").arg(d.toString(QStringLiteral("dd/MM")));
      } else if (i == 1) {
        label = tr("Hôm qua (%1)").arg(d.toString(QStringLiteral("dd/MM")));
      } else {
        label = d.toString(QStringLiteral("dd/MM/yyyy"));
      }
      ui->dateCombo->addItem(label, d);
    }
  }

  // Find best match for m_selectedDate
  int matchIdx = -1;
  for (int i = 0; i < ui->dateCombo->count(); ++i) {
    const QDate d = ui->dateCombo->itemData(i).toDate();
    if (period == QStringLiteral("year")) {
      if (d.year() == m_selectedDate.year()) {
        matchIdx = i;
        break;
      }
    } else if (period == QStringLiteral("month")) {
      if (d.year() == m_selectedDate.year() &&
          d.month() == m_selectedDate.month()) {
        matchIdx = i;
        break;
      }
    } else {
      if (d == m_selectedDate) {
        matchIdx = i;
        break;
      }
    }
  }

  if (matchIdx >= 0) {
    ui->dateCombo->setCurrentIndex(matchIdx);
  } else {
    QString customLabel;
    if (period == QStringLiteral("year")) {
      customLabel = tr("Năm %1").arg(m_selectedDate.year());
    } else if (period == QStringLiteral("month")) {
      customLabel =
          tr("Tháng %1")
              .arg(m_selectedDate.toString(QStringLiteral("MM/yyyy")));
    } else {
      customLabel = m_selectedDate.toString(QStringLiteral("dd/MM/yyyy"));
    }
    ui->dateCombo->insertItem(0, customLabel, m_selectedDate);
    ui->dateCombo->setCurrentIndex(0);
  }

  ui->dateCombo->blockSignals(false);
}

void HistoryPage::requestCurrentHistory() {
  const QString deviceId = ui->deviceCombo->currentData().toString();
  const QString period = ui->periodCombo->currentData().toString();
  const QDate curDate = selectedDate();
  const bool isToday = (curDate == QDate::currentDate());
  const bool isOnline = m_deviceOnline.value(deviceId, false);

  if (m_liveTimer) {
    if (period == QStringLiteral("day") && isToday && !deviceId.isEmpty() &&
        isOnline) {
      if (!m_liveTimer->isActive())
        m_liveTimer->start();
    } else {
      if (m_liveTimer->isActive())
        m_liveTimer->stop();
    }
  }

  if (deviceId.isEmpty()) {
    ui->historyTable->setRowCount(0);
    ui->recordCountLabel->setText(tr("0 bản ghi"));
    m_primaryStat->setText(QStringLiteral("--"));
    m_secondaryStat->setText(QStringLiteral("--"));
    m_thirdStat->setText(tr("Chưa có thiết bị"));
    m_cachedKeys = {};
    m_cachedRows = {};
    updateMetricSelector();
    updateChart();
    return;
  }
  emit historyRequested(deviceId, period, curDate.toString(Qt::ISODate));
}

void HistoryPage::setPeriod(const QString &period) {
  const int idx = ui->periodCombo->findData(period);
  if (idx >= 0) {
    ui->periodCombo->setCurrentIndex(idx);
  }
}

void HistoryPage::setDate(const QDate &date) {
  if (!date.isValid())
    return;
  m_selectedDate = date;
  rebuildDateOptions();
  requestCurrentHistory();
}

void HistoryPage::setViewTab(int tabIndex) {
  if (tabIndex == 1) {
    ui->tableTabButton->setChecked(true);
    ui->viewStack->setCurrentIndex(1);
  } else {
    ui->chartTabButton->setChecked(true);
    ui->viewStack->setCurrentIndex(0);
  }
}

void HistoryPage::setMetric(const QString &key) {
  m_selectedMetricKey = key;
  const int idx = m_metricCombo->findData(key);
  if (idx >= 0) {
    m_metricCombo->setCurrentIndex(idx);
  } else {
    updateChart();
  }
}

void HistoryPage::updateMetricSelector() {
  m_metricCombo->blockSignals(true);
  m_metricCombo->clear();

  if (m_cachedKeys.isEmpty()) {
    m_metricCombo->hide();
    m_metricCombo->blockSignals(false);
    return;
  }

  QStringList plotableKeys;
  for (const QJsonValue &k : m_cachedKeys) {
    const QString keyStr = k.toString();
    if (keyStr != QStringLiteral("ir_detected"))
      plotableKeys.append(keyStr);
  }

  if (!plotableKeys.isEmpty()) {
    for (const QString &key : plotableKeys) {
      QString cleanName = metricTitle(key);
      if (key == QStringLiteral("uv_index"))
        cleanName = tr("Tia UV (UVI)");
      else if (key == QStringLiteral("pressure_hpa"))
        cleanName = tr("Áp suất (hPa)");
      else if (key == QStringLiteral("uv_voltage"))
        cleanName = tr("Điện áp (V)");
      m_metricCombo->addItem(tr("%1").arg(cleanName), key);
    }
    int idx = m_metricCombo->findData(m_selectedMetricKey);
    if (idx >= 0 && m_selectedMetricKey != QStringLiteral("all")) {
      m_metricCombo->setCurrentIndex(idx);
    } else {
      int defaultIdx = m_metricCombo->findData(QStringLiteral("uv_index"));
      if (defaultIdx < 0)
        defaultIdx = m_metricCombo->findData(QStringLiteral("pressure_hpa"));
      if (defaultIdx < 0)
        defaultIdx = 0;
      m_metricCombo->setCurrentIndex(defaultIdx);
      m_selectedMetricKey = m_metricCombo->itemData(defaultIdx).toString();
    }
    const bool isChartTab = (ui->viewStack->currentIndex() == 0);
    m_metricCombo->setVisible(isChartTab);
    if (m_zoomBtn)
      m_zoomBtn->setVisible(isChartTab);
  } else {
    m_metricCombo->hide();
    if (m_zoomBtn)
      m_zoomBtn->hide();
    m_selectedMetricKey.clear();
  }
  m_metricCombo->blockSignals(false);
}

void HistoryPage::setHistory(const QJsonObject &history) {
  m_cachedPeriod = history.value(QStringLiteral("period")).toString();
  m_cachedSelectedDate =
      history.value(QStringLiteral("selected_date")).toString();
  m_cachedKeys = history.value(QStringLiteral("metric_keys")).toArray();
  m_cachedRows = history.value(QStringLiteral("data")).toArray();
  const QJsonArray keys = m_cachedKeys;
  const QJsonArray rows = m_cachedRows;

  const QString devId = ui->deviceCombo->currentData().toString();
  const bool isOnline = m_deviceOnline.value(devId, false);

  const QString addedBy = history.value(QStringLiteral("added_by")).toString();
  const QString addedAt = history.value(QStringLiteral("added_at")).toString();
  if (!addedBy.isEmpty() && addedBy != QStringLiteral("Chưa gán")) {
    QDateTime addTime = QDateTime::fromString(addedAt, Qt::ISODateWithMs);
    if (!addTime.isValid())
      addTime = QDateTime::fromString(addedAt, Qt::ISODate);
    const QString addTimeStr =
        addTime.isValid()
            ? addTime.toLocalTime().toString(QStringLiteral("dd/MM/yyyy HH:mm"))
            : addedAt;
    const QString statusHint =
        isOnline ? tr("● Trực tuyến (Đang cập nhật thời gian thực)")
                 : tr("Ngoại tuyến (Đã ngắt kết nối · Dừng cập nhật)");
    m_headerSubtitle->setText(tr("Thiết bị: %1 · %2 · Người thêm: %3 (%4) · "
                                 "Bấm vào biểu đồ để phóng to.")
                                  .arg(ui->deviceCombo->currentText(),
                                       statusHint, addedBy, addTimeStr));
  }

  ui->historyTable->clear();
  ui->historyTable->setRowCount(rows.size());
  ui->historyTable->setColumnCount(keys.size() + 1);
  ui->historyTable->setAlternatingRowColors(false);

  QStringList headers;
  if (m_cachedPeriod == QStringLiteral("day")) {
    headers.append(tr("Thời gian (Giây)"));
    for (const QJsonValue &key : keys)
      headers.append(metricTitle(key.toString()));
  } else if (m_cachedPeriod == QStringLiteral("month")) {
    headers.append(tr("Ngày (Tổng hợp)"));
    for (const QJsonValue &key : keys)
      headers.append(tr("%1 (TB)").arg(metricTitle(key.toString())));
  } else if (m_cachedPeriod == QStringLiteral("year")) {
    headers.append(tr("Tháng (Tổng hợp)"));
    for (const QJsonValue &key : keys)
      headers.append(tr("%1 (TB)").arg(metricTitle(key.toString())));
  } else {
    headers.append(tr("Thời gian"));
    for (const QJsonValue &key : keys)
      headers.append(metricTitle(key.toString()));
  }
  ui->historyTable->setHorizontalHeaderLabels(headers);

  for (int row = 0; row < rows.size(); ++row) {
    const QJsonObject entry = rows.at(row).toObject();
    const QString recordedAtStr =
        entry.value(QStringLiteral("recorded_at")).toString();
    QString cellTimeText;

    if (m_cachedPeriod == QStringLiteral("day")) {
      QDateTime time = QDateTime::fromString(recordedAtStr, Qt::ISODateWithMs);
      if (!time.isValid())
        time = QDateTime::fromString(recordedAtStr, Qt::ISODate);
      if (!time.isValid())
        time = QDateTime::fromString(recordedAtStr,
                                     QStringLiteral("yyyy-MM-dd HH:mm:ss"));
      time = time.toLocalTime();
      cellTimeText = time.isValid() ? time.toString(QStringLiteral("HH:mm:ss"))
                                    : recordedAtStr;
    } else if (m_cachedPeriod == QStringLiteral("month")) {
      const QDate d =
          QDate::fromString(recordedAtStr, QStringLiteral("yyyy-MM-dd"));
      cellTimeText =
          d.isValid()
              ? tr("Ngày %1").arg(d.toString(QStringLiteral("dd/MM/yyyy")))
              : recordedAtStr;
    } else if (m_cachedPeriod == QStringLiteral("year")) {
      const QDate m = QDate::fromString(recordedAtStr + QStringLiteral("-01"),
                                        QStringLiteral("yyyy-MM-dd"));
      cellTimeText =
          m.isValid()
              ? tr("Tháng %1").arg(m.toString(QStringLiteral("MM/yyyy")))
              : recordedAtStr;
    } else {
      cellTimeText = recordedAtStr;
    }

    const QColor rowBg(row % 2 == 0 ? "#0c1630" : "#111d3d");
    auto *timeItem = new QTableWidgetItem(cellTimeText);
    timeItem->setForeground(QBrush(QColor("#ffffff")));
    timeItem->setBackground(QBrush(rowBg));
    ui->historyTable->setItem(row, 0, timeItem);

    const QJsonObject metrics =
        entry.value(QStringLiteral("metrics")).toObject();
    for (int column = 0; column < keys.size(); ++column) {
      const QJsonValue value = metrics.value(keys.at(column).toString());
      auto *valItem = new QTableWidgetItem(
          value.isDouble() ? QString::number(value.toDouble(), 'f', 2)
                           : QStringLiteral("—"));
      valItem->setForeground(QBrush(QColor("#38bdf8")));
      valItem->setBackground(QBrush(rowBg));
      ui->historyTable->setItem(row, column + 1, valItem);
    }
  }

  const int total = history.value(QStringLiteral("total")).toInt();
  if (m_cachedPeriod == QStringLiteral("day")) {
    if (isOnline) {
      ui->recordCountLabel->setText(tr("● %1 mẫu (giây)").arg(total));
      ui->recordCountLabel->setObjectName(
          QStringLiteral("historyRecordBadgeOnline"));
    } else {
      ui->recordCountLabel->setText(tr("Ngoại tuyến · %1 mẫu").arg(total));
      ui->recordCountLabel->setObjectName(
          QStringLiteral("historyRecordBadgeOffline"));
    }
  } else if (m_cachedPeriod == QStringLiteral("month")) {
    ui->recordCountLabel->setText(tr("%1 ngày").arg(total));
    ui->recordCountLabel->setObjectName(QStringLiteral("historyRecordBadge"));
  } else if (m_cachedPeriod == QStringLiteral("year")) {
    ui->recordCountLabel->setText(tr("%1 tháng").arg(total));
    ui->recordCountLabel->setObjectName(QStringLiteral("historyRecordBadge"));
  } else {
    ui->recordCountLabel->setText(tr("%1 bản ghi").arg(total));
    ui->recordCountLabel->setObjectName(QStringLiteral("historyRecordBadge"));
  }
  ui->recordCountLabel->style()->unpolish(ui->recordCountLabel);
  ui->recordCountLabel->style()->polish(ui->recordCountLabel);

  const QJsonObject averages =
      history.value(QStringLiteral("averages")).toObject();
  if (averages.value(QStringLiteral("pressure_hpa")).isDouble()) {
    if (m_primaryStatTitle)
      m_primaryStatTitle->setText(tr("ÁP SUẤT KHÍ QUYỂN (TB)"));
    m_primaryStat->setText(QStringLiteral("%1 hPa").arg(QString::number(
        averages.value(QStringLiteral("pressure_hpa")).toDouble(), 'f', 1)));
    m_primaryStat->setStyleSheet(
        QStringLiteral("color: #38bdf8; font-size: 14px; font-weight: 900;"));
  } else {
    if (m_primaryStatTitle)
      m_primaryStatTitle->setText(tr("ÁP SUẤT KHÍ QUYỂN"));
    m_primaryStat->setText(QStringLiteral("-- hPa"));
    m_primaryStat->setStyleSheet(
        QStringLiteral("color: #94a3b8; font-size: 14px; font-weight: 900;"));
  }

  if (averages.value(QStringLiteral("uv_index")).isDouble()) {
    if (m_secondaryStatTitle)
      m_secondaryStatTitle->setText(tr("CƯỜNG ĐỘ TIA UV (TB)"));
    m_secondaryStat->setText(QStringLiteral("%1 UVI").arg(QString::number(
        averages.value(QStringLiteral("uv_index")).toDouble(), 'f', 2)));
    m_secondaryStat->setStyleSheet(
        QStringLiteral("color: #f59e0b; font-size: 14px; font-weight: 900;"));
  } else {
    if (m_secondaryStatTitle)
      m_secondaryStatTitle->setText(tr("CƯỜNG ĐỘ TIA UV"));
    m_secondaryStat->setText(QStringLiteral("-- UVI"));
    m_secondaryStat->setStyleSheet(
        QStringLiteral("color: #94a3b8; font-size: 14px; font-weight: 900;"));
  }

  if (averages.value(QStringLiteral("uv_voltage")).isDouble()) {
    if (m_summaryStatTitle)
      m_summaryStatTitle->setText(tr("ĐIỆN ÁP CẢM BIẾN UV (TB)"));
    m_thirdStat->setText(QStringLiteral("%1 V").arg(QString::number(
        averages.value(QStringLiteral("uv_voltage")).toDouble(), 'f', 3)));
    m_thirdStat->setStyleSheet(
        QStringLiteral("color: #34d399; font-size: 14px; font-weight: 900;"));
  } else {
    if (m_summaryStatTitle)
      m_summaryStatTitle->setText(tr("TỔNG SỐ BẢN GHI"));
    m_thirdStat->setText(tr("%1 mẫu").arg(total));
    m_thirdStat->setStyleSheet(
        QStringLiteral("color: #ecf2ff; font-size: 14px; font-weight: 900;"));
  }

  updateMetricSelector();
  updateChart();
}

void HistoryPage::updateChart() {
  m_chart->removeAllSeries();
  const QList<QAbstractAxis *> oldAxes = m_chart->axes();
  for (QAbstractAxis *axis : oldAxes) {
    m_chart->removeAxis(axis);
    axis->deleteLater();
  }
  const QJsonArray &keys = m_cachedKeys;
  const QJsonArray &rows = m_cachedRows;

  if (keys.isEmpty() || rows.isEmpty()) {
    m_chartTitle->setText(tr("Không có dữ liệu"));
    m_chartHint->setText(
        tr("Không có dữ liệu trong khoảng thời gian đã chọn."));
    return;
  }

  // Determine the single active metric key (no 'all' combination)
  QString activeKey = m_selectedMetricKey;
  if (activeKey.isEmpty() || activeKey == QStringLiteral("all")) {
    for (const QJsonValue &k : keys) {
      const QString keyStr = k.toString();
      if (keyStr != QStringLiteral("ir_detected")) {
        activeKey = keyStr;
        break;
      }
    }
    m_selectedMetricKey = activeKey;
  }

  if (activeKey.isEmpty()) {
    m_chartTitle->setText(tr("Không có dữ liệu biểu đồ"));
    m_chartHint->setText(
        tr("Các bản ghi không chứa giá trị số phù hợp để vẽ biểu đồ."));
    return;
  }

  const QString currentPeriod = m_cachedPeriod.isEmpty()
                                    ? ui->periodCombo->currentData().toString()
                                    : m_cachedPeriod;

  auto *barSeries = new QBarSeries(m_chart);

  QStringList categories;
  QList<QJsonObject> chronologicalRows;
  for (int r = rows.size() - 1; r >= 0; --r) {
    chronologicalRows.append(rows.at(r).toObject());
  }

  double overallMin = std::numeric_limits<double>::max();
  double overallMax = std::numeric_limits<double>::lowest();

  auto *barSet = new QBarSet(metricTitle(activeKey));
  const QColor col = getColorForKey(activeKey, 0);
  barSet->setColor(col);
  barSet->setBorderColor(col.lighter(120));

  if (currentPeriod == QStringLiteral("month")) {
    for (const QJsonObject &entry : chronologicalRows) {
      const QString recStr =
          entry.value(QStringLiteral("recorded_at")).toString();
      const QDate d = QDate::fromString(recStr, QStringLiteral("yyyy-MM-dd"));
      categories << (d.isValid() ? QStringLiteral("Ngày %1").arg(d.day(), 2, 10,
                                                                 QChar('0'))
                                 : recStr);
      const QJsonObject m = entry.value(QStringLiteral("metrics")).toObject();
      const double val = m.value(activeKey).toDouble(0.0);
      *barSet << val;
      overallMin = qMin(overallMin, val);
      overallMax = qMax(overallMax, val);
    }
    m_chartTitle->setText(
        tr("Biểu đồ cột %1 theo ngày · Tháng %2")
            .arg(metricShortName(activeKey), m_cachedSelectedDate.left(7)));
    m_chartHint->setText(
        tr("Mỗi cột đại diện cho giá trị %1 trung bình của một ngày.")
            .arg(metricTitle(activeKey)));
  } else if (currentPeriod == QStringLiteral("year")) {
    for (const QJsonObject &entry : chronologicalRows) {
      const QString recStr =
          entry.value(QStringLiteral("recorded_at")).toString();
      const int mNum = recStr.mid(5, 2).toInt();
      categories << QStringLiteral("Tháng %1").arg(mNum, 2, 10, QChar('0'));
      const QJsonObject m = entry.value(QStringLiteral("metrics")).toObject();
      const double val = m.value(activeKey).toDouble(0.0);
      *barSet << val;
      overallMin = qMin(overallMin, val);
      overallMax = qMax(overallMax, val);
    }
    m_chartTitle->setText(
        tr("Biểu đồ cột %1 theo tháng · Năm %2")
            .arg(metricShortName(activeKey), m_cachedSelectedDate.left(4)));
    m_chartHint->setText(
        tr("Mỗi cột đại diện cho giá trị %1 trung bình của một tháng.")
            .arg(metricTitle(activeKey)));
  } else {
    // currentPeriod == "day" -> Biểu đồ cột theo ngày (tổng hợp theo khoảng
    // thời gian thực)
    struct RawSample {
      QDateTime dt;
      double val;
    };
    QList<RawSample> rawSamples;
    for (const QJsonObject &entry : chronologicalRows) {
      const QJsonValue v =
          entry.value(QStringLiteral("metrics")).toObject().value(activeKey);
      if (!v.isDouble())
        continue;
      QString recStr = entry.value(QStringLiteral("recorded_at")).toString();
      QDateTime dt = QDateTime::fromString(recStr, Qt::ISODateWithMs);
      if (!dt.isValid())
        dt = QDateTime::fromString(recStr, Qt::ISODate);
      if (!dt.isValid())
        dt = QDateTime::fromString(recStr,
                                   QStringLiteral("yyyy-MM-dd HH:mm:ss"));
      dt = dt.toLocalTime();
      if (dt.isValid()) {
        rawSamples.append({dt, v.toDouble()});
      }
    }

    if (rawSamples.isEmpty()) {
      m_chartTitle->setText(tr("Không có dữ liệu biểu đồ"));
      m_chartHint->setText(
          tr("Các bản ghi không chứa giá trị số phù hợp để vẽ biểu đồ."));
      return;
    }

    if (rawSamples.size() <= 20) {
      for (const auto &s : rawSamples) {
        categories.append(s.dt.toString(QStringLiteral("HH:mm:ss")));
        *barSet << s.val;
        overallMin = qMin(overallMin, s.val);
        overallMax = qMax(overallMax, s.val);
      }
    } else {
      int binMinutes = 2;
      QMap<QString, QPair<double, int>> bins;
      QStringList binOrder;

      for (int bm : {2, 5, 10, 15, 30, 60}) {
        binMinutes = bm;
        bins.clear();
        binOrder.clear();
        for (const auto &s : rawSamples) {
          int m = s.dt.time().minute();
          int b = (m / binMinutes) * binMinutes;
          QTime binTime(s.dt.time().hour(), b, 0);
          QString binKey = binTime.toString(QStringLiteral("HH:mm"));
          if (!bins.contains(binKey)) {
            binOrder.append(binKey);
          }
          bins[binKey].first += s.val;
          bins[binKey].second += 1;
        }
        if (binOrder.size() <= 25) {
          break;
        }
      }

      for (const QString &binKey : binOrder) {
        categories.append(binKey);
        double avg = bins[binKey].first / bins[binKey].second;
        *barSet << avg;
        overallMin = qMin(overallMin, avg);
        overallMax = qMax(overallMax, avg);
      }
    }

    m_chartTitle->setText(
        tr("Biểu đồ cột %1 · Ngày %2")
            .arg(metricShortName(activeKey), m_cachedSelectedDate.left(10)));
    m_chartHint->setText(tr("Mỗi cột đại diện cho giá trị %1 trung bình theo "
                            "mốc thời gian trong ngày.")
                             .arg(metricTitle(activeKey)));
  }

  const int numBars = categories.size();
  double barWidth = 0.60;
  if (numBars <= 1) {
    barWidth = 0.08;
  } else if (numBars <= 2) {
    barWidth = 0.14;
  } else if (numBars <= 3) {
    barWidth = 0.20;
  } else if (numBars <= 5) {
    barWidth = 0.32;
  } else if (numBars <= 8) {
    barWidth = 0.45;
  } else {
    barWidth = 0.65;
  }
  barSeries->setBarWidth(barWidth);

  barSeries->append(barSet);
  m_chart->addSeries(barSeries);

  auto *axisX = new QBarCategoryAxis(m_chart);
  QFont axisFont;
  axisFont.setPixelSize(9);
  axisX->setLabelsFont(axisFont);
  axisX->setLabelsColor(QColor("#94a3b8"));
  axisX->setGridLineColor(QColor("#1c2b54"));
  axisX->append(categories);
  m_chart->addAxis(axisX, Qt::AlignBottom);
  barSeries->attachAxis(axisX);

  auto *axisY = new QValueAxis(m_chart);
  axisY->setLabelsFont(axisFont);
  axisY->setLabelsColor(col);
  axisY->setTitleBrush(col);
  axisY->setTitleFont(axisFont);
  axisY->setGridLineColor(QColor("#1c2b54"));
  axisY->setTitleText(compactMetricTitle(activeKey));

  if (overallMin > overallMax) {
    overallMin = 0.0;
    overallMax = 10.0;
  }

  if (activeKey == QStringLiteral("uv_index")) {
    axisY->setRange(0.0, qMax(11.0, overallMax + 1.0));
    axisY->setLabelFormat("%.1f");
  } else if (activeKey == QStringLiteral("pressure_hpa")) {
    const double diff = overallMax - overallMin;
    const double padding = qMax(1.0, (diff == 0.0 ? 2.0 : diff * 0.15));
    axisY->setRange(qMax(0.0, overallMin - padding), overallMax + padding);
    axisY->setLabelFormat("%.1f");
  } else if (activeKey == QStringLiteral("uv_voltage")) {
    axisY->setRange(0.0, qMax(3.3, overallMax + 0.2));
    axisY->setLabelFormat("%.2f");
  } else {
    const double diff = overallMax - overallMin;
    const double padding = qMax(
        0.5, (diff == 0.0
                  ? (qAbs(overallMax) > 0 ? qAbs(overallMax) * 0.15 + 0.5 : 1.0)
                  : diff * 0.15));
    axisY->setRange(qMax(0.0, overallMin - padding), overallMax + padding);
    axisY->setLabelFormat("%.1f");
  }
  m_chart->addAxis(axisY, Qt::AlignLeft);
  barSeries->attachAxis(axisY);
}

QString HistoryPage::currentDeviceType() const {
  return ui->deviceCombo->currentData(Qt::UserRole + 1).toString();
}

QString HistoryPage::metricTitle(const QString &key) {
  static const QHash<QString, QString> names{
      {"pressure_hpa", tr("Áp suất khí quyển (hPa)")},
      {"uv_index", tr("Cường độ tia UV (UVI)")},
      {"uv_voltage", tr("Điện áp cảm biến UV (V)")},
      {"temperature_c", tr("Nhiệt độ trạm đo (°C)")}};
  return names.value(key, key);
}

QString HistoryPage::compactMetricTitle(const QString &key) {
  static const QHash<QString, QString> names{{"pressure_hpa", tr("hPa")},
                                             {"uv_index", tr("UV Index")},
                                             {"uv_voltage", tr("V")},
                                             {"temperature_c", tr("°C")}};
  return names.value(key, key);
}

QString HistoryPage::metricUnit(const QString &key) {
  return compactMetricTitle(key);
}

void HistoryPage::openChartZoomDialog(const QString &initialMetricKey) {
  if (m_cachedKeys.isEmpty() || m_cachedRows.isEmpty()) {
    return;
  }

  QDialog dialog(this);
  dialog.setObjectName(QStringLiteral("chartZoomDialog"));
  dialog.setWindowTitle(tr("Phóng to biểu đồ cảm biến"));
  dialog.setModal(true);

  const int availableWidth =
      parentWidget() ? parentWidget()->width() - 16 : 760;
  const int availableHeight =
      parentWidget() ? parentWidget()->height() - 16 : 460;
  dialog.resize(qMax(360, availableWidth), qMax(280, availableHeight));

  auto *root = new QVBoxLayout(&dialog);
  root->setContentsMargins(14, 12, 14, 12);
  root->setSpacing(10);

  auto *headerLayout = new QHBoxLayout;
  headerLayout->setContentsMargins(0, 0, 0, 0);
  headerLayout->setSpacing(10);

  auto *titleBlock = new QVBoxLayout;
  titleBlock->setContentsMargins(0, 0, 0, 0);
  titleBlock->setSpacing(2);
  auto *dialogTitle = new QLabel(tr("Biểu đồ chi tiết"), &dialog);
  dialogTitle->setObjectName(QStringLiteral("chartZoomTitle"));
  auto *dialogSubtitle = new QLabel(ui->deviceCombo->currentText(), &dialog);
  dialogSubtitle->setObjectName(QStringLiteral("chartZoomSubtitle"));
  titleBlock->addWidget(dialogTitle);
  titleBlock->addWidget(dialogSubtitle);
  headerLayout->addLayout(titleBlock, 1);

  auto *metricCombo = new QComboBox(&dialog);
  metricCombo->setObjectName(QStringLiteral("historyMetricCombo"));
  metricCombo->setMinimumWidth(180);

  QStringList plotableKeys;
  for (const QJsonValue &k : m_cachedKeys) {
    const QString keyStr = k.toString();
    if (keyStr != QStringLiteral("ir_detected"))
      plotableKeys.append(keyStr);
  }

  for (const QString &key : plotableKeys) {
    metricCombo->addItem(tr("%1").arg(metricTitle(key)), key);
  }

  QString selectedKey =
      initialMetricKey.isEmpty() ? m_selectedMetricKey : initialMetricKey;
  if (selectedKey.isEmpty() || selectedKey == QStringLiteral("all") ||
      metricCombo->findData(selectedKey) < 0)
    selectedKey = plotableKeys.isEmpty() ? QString() : plotableKeys.first();

  int foundIdx = metricCombo->findData(selectedKey);
  if (foundIdx >= 0)
    metricCombo->setCurrentIndex(foundIdx);

  headerLayout->addWidget(metricCombo, 0, Qt::AlignVCenter);

  auto *closeBtn = new QPushButton(QStringLiteral("Đóng"), &dialog);
  closeBtn->setObjectName(QStringLiteral("chartZoomCloseBtn"));
  closeBtn->setFixedSize(36, 36);
  closeBtn->setCursor(Qt::PointingHandCursor);
  headerLayout->addWidget(closeBtn, 0, Qt::AlignVCenter);
  root->addLayout(headerLayout);

  auto *zoomChart = new QChart;
  zoomChart->setAnimationOptions(QChart::SeriesAnimations);
  zoomChart->legend()->setAlignment(Qt::AlignBottom);
  zoomChart->legend()->setVisible(true);

  auto *zoomChartView = new QChartView(zoomChart, &dialog);
  zoomChartView->setObjectName(QStringLiteral("chartZoomView"));
  zoomChartView->setRenderHint(QPainter::Antialiasing);
  zoomChartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  root->addWidget(zoomChartView, 1);

  auto *statsLayout = new QHBoxLayout;
  statsLayout->setContentsMargins(0, 0, 0, 0);
  statsLayout->setSpacing(10);

  auto *countBadge =
      new QLabel(tr("Tổng: %1 bản ghi").arg(m_cachedRows.size()), &dialog);
  auto *minBadge = new QLabel(&dialog);
  auto *maxBadge = new QLabel(&dialog);
  auto *avgBadge = new QLabel(&dialog);
  for (QLabel *badge : {countBadge, minBadge, maxBadge, avgBadge}) {
    badge->setObjectName(QStringLiteral("chartZoomStatBadge"));
  }
  statsLayout->addWidget(countBadge);
  statsLayout->addWidget(minBadge);
  statsLayout->addWidget(maxBadge);
  statsLayout->addWidget(avgBadge);
  statsLayout->addStretch();

  auto *bottomClose = new QPushButton(tr("Đóng"), &dialog);
  bottomClose->setObjectName(QStringLiteral("chartZoomBottomClose"));
  bottomClose->setMinimumWidth(90);
  statsLayout->addWidget(bottomClose);
  root->addLayout(statsLayout);

  connect(closeBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
  connect(bottomClose, &QPushButton::clicked, &dialog, &QDialog::accept);

  const auto renderZoomChart = [this, zoomChart, metricCombo, dialogTitle,
                                minBadge, maxBadge, avgBadge]() {
    zoomChart->removeAllSeries();
    for (QAbstractAxis *axis : zoomChart->axes()) {
      zoomChart->removeAxis(axis);
      axis->deleteLater();
    }

    const QString activeMetric = metricCombo->currentData().toString();
    const QJsonArray &keys = m_cachedKeys;
    const QJsonArray &rows = m_cachedRows;

    QString activeKey = activeMetric;
    if (activeKey.isEmpty() || activeKey == QStringLiteral("all")) {
      for (const QJsonValue &k : keys) {
        const QString keyStr = k.toString();
        if (keyStr != QStringLiteral("ir_detected")) {
          activeKey = keyStr;
          break;
        }
      }
    }

    dialogTitle->setText(
        tr("Biểu đồ chi tiết: %1").arg(metricTitle(activeKey)));

    const QString currentPeriod =
        m_cachedPeriod.isEmpty() ? ui->periodCombo->currentData().toString()
                                 : m_cachedPeriod;

    const QColor seriesColor = getColorForKey(activeKey, 0);

    double overallMin = std::numeric_limits<double>::max();
    double overallMax = std::numeric_limits<double>::lowest();
    double overallSum = 0;
    int overallCount = 0;

    auto *barSeries = new QBarSeries(zoomChart);

    QStringList categories;
    QList<QJsonObject> chronologicalRows;
    for (int r = rows.size() - 1; r >= 0; --r) {
      chronologicalRows.append(rows.at(r).toObject());
    }

    auto *barSet = new QBarSet(metricTitle(activeKey));
    barSet->setColor(seriesColor);
    barSet->setBorderColor(seriesColor.lighter(120));

    if (currentPeriod == QStringLiteral("month")) {
      for (const QJsonObject &entry : chronologicalRows) {
        const QString recStr =
            entry.value(QStringLiteral("recorded_at")).toString();
        const QDate d = QDate::fromString(recStr, QStringLiteral("yyyy-MM-dd"));
        categories << (d.isValid() ? QStringLiteral("Ngày %1").arg(
                                         d.day(), 2, 10, QChar('0'))
                                   : recStr);
        const QJsonObject m = entry.value(QStringLiteral("metrics")).toObject();
        const double val = m.value(activeKey).toDouble(0.0);
        *barSet << val;
        overallMin = qMin(overallMin, val);
        overallMax = qMax(overallMax, val);
        overallSum += val;
        ++overallCount;
      }
      dialogTitle->setText(tr("Biểu đồ cột chi tiết theo ngày · Tháng %1")
                               .arg(m_cachedSelectedDate.left(7)));
    } else if (currentPeriod == QStringLiteral("year")) {
      for (const QJsonObject &entry : chronologicalRows) {
        const QString recStr =
            entry.value(QStringLiteral("recorded_at")).toString();
        const int mNum = recStr.mid(5, 2).toInt();
        categories << QStringLiteral("Tháng %1").arg(mNum, 2, 10, QChar('0'));
        const QJsonObject m = entry.value(QStringLiteral("metrics")).toObject();
        const double val = m.value(activeKey).toDouble(0.0);
        *barSet << val;
        overallMin = qMin(overallMin, val);
        overallMax = qMax(overallMax, val);
        overallSum += val;
        ++overallCount;
      }
      dialogTitle->setText(tr("Biểu đồ cột chi tiết theo tháng · Năm %1")
                               .arg(m_cachedSelectedDate.left(4)));
    } else {
      // currentPeriod == "day"
      struct RawSample {
        QDateTime dt;
        double val;
      };
      QList<RawSample> rawSamples;
      for (const QJsonObject &entry : chronologicalRows) {
        const QJsonValue v =
            entry.value(QStringLiteral("metrics")).toObject().value(activeKey);
        if (!v.isDouble())
          continue;
        QString recStr = entry.value(QStringLiteral("recorded_at")).toString();
        QDateTime dt = QDateTime::fromString(recStr, Qt::ISODateWithMs);
        if (!dt.isValid())
          dt = QDateTime::fromString(recStr, Qt::ISODate);
        if (!dt.isValid())
          dt = QDateTime::fromString(recStr,
                                     QStringLiteral("yyyy-MM-dd HH:mm:ss"));
        dt = dt.toLocalTime();
        if (dt.isValid()) {
          rawSamples.append({dt, v.toDouble()});
        }
      }

      if (rawSamples.size() <= 20) {
        for (const auto &s : rawSamples) {
          categories.append(s.dt.toString(QStringLiteral("HH:mm:ss")));
          *barSet << s.val;
          overallMin = qMin(overallMin, s.val);
          overallMax = qMax(overallMax, s.val);
          overallSum += s.val;
          ++overallCount;
        }
      } else if (!rawSamples.isEmpty()) {
        int binMinutes = 2;
        QMap<QString, QPair<double, int>> bins;
        QStringList binOrder;

        for (int bm : {2, 5, 10, 15, 30, 60}) {
          binMinutes = bm;
          bins.clear();
          binOrder.clear();
          for (const auto &s : rawSamples) {
            int m = s.dt.time().minute();
            int b = (m / binMinutes) * binMinutes;
            QTime binTime(s.dt.time().hour(), b, 0);
            QString binKey = binTime.toString(QStringLiteral("HH:mm"));
            if (!bins.contains(binKey)) {
              binOrder.append(binKey);
            }
            bins[binKey].first += s.val;
            bins[binKey].second += 1;
          }
          if (binOrder.size() <= 25) {
            break;
          }
        }

        for (const QString &binKey : binOrder) {
          categories.append(binKey);
          double avg = bins[binKey].first / bins[binKey].second;
          *barSet << avg;
          overallMin = qMin(overallMin, avg);
          overallMax = qMax(overallMax, avg);
          overallSum += avg;
          ++overallCount;
        }
      }

      dialogTitle->setText(
          tr("Biểu đồ cột chi tiết %1 · Ngày %2")
              .arg(metricShortName(activeKey), m_cachedSelectedDate.left(10)));
    }

    const int numBars = categories.size();
    double barWidth = 0.60;
    if (numBars <= 1) {
      barWidth = 0.08;
    } else if (numBars <= 2) {
      barWidth = 0.14;
    } else if (numBars <= 3) {
      barWidth = 0.20;
    } else if (numBars <= 5) {
      barWidth = 0.32;
    } else if (numBars <= 8) {
      barWidth = 0.45;
    } else {
      barWidth = 0.65;
    }
    barSeries->setBarWidth(barWidth);

    barSeries->append(barSet);
    zoomChart->addSeries(barSeries);

    auto *axisX = new QBarCategoryAxis(zoomChart);
    QFont axisFont;
    axisFont.setPixelSize(10);
    axisX->setLabelsFont(axisFont);
    axisX->append(categories);
    zoomChart->addAxis(axisX, Qt::AlignBottom);
    barSeries->attachAxis(axisX);

    auto *axisY = new QValueAxis(zoomChart);
    axisY->setLabelsFont(axisFont);
    axisY->setLabelsColor(seriesColor);
    axisY->setTitleBrush(seriesColor);
    axisY->setTitleFont(axisFont);
    axisY->setTitleText(compactMetricTitle(activeKey));

    if (overallMin > overallMax) {
      overallMin = 0.0;
      overallMax = 10.0;
    }

    if (activeKey == QStringLiteral("uv_index")) {
      axisY->setRange(0.0, qMax(11.0, overallMax + 1.0));
      axisY->setLabelFormat("%.1f");
    } else if (activeKey == QStringLiteral("pressure_hpa")) {
      const double diff = overallMax - overallMin;
      const double padding = qMax(1.0, (diff == 0.0 ? 2.0 : diff * 0.15));
      axisY->setRange(qMax(0.0, overallMin - padding), overallMax + padding);
      axisY->setLabelFormat("%.1f");
    } else if (activeKey == QStringLiteral("uv_voltage")) {
      axisY->setRange(0.0, qMax(3.3, overallMax + 0.2));
      axisY->setLabelFormat("%.2f");
    } else {
      const double diff = overallMax - overallMin;
      const double padding = qMax(
          0.5,
          (diff == 0.0
               ? (qAbs(overallMax) > 0 ? qAbs(overallMax) * 0.15 + 0.5 : 1.0)
               : diff * 0.15));
      axisY->setRange(qMax(0.0, overallMin - padding), overallMax + padding);
      axisY->setLabelFormat("%.2f");
    }
    zoomChart->addAxis(axisY, Qt::AlignLeft);
    barSeries->attachAxis(axisY);

    if (overallCount > 0 && !activeKey.isEmpty()) {
      const QString unit = compactMetricTitle(activeKey);
      minBadge->setText(
          tr("Min: %1 %2").arg(QString::number(overallMin, 'f', 2), unit));
      maxBadge->setText(
          tr("Max: %1 %2").arg(QString::number(overallMax, 'f', 2), unit));
      avgBadge->setText(
          tr("TB: %1 %2")
              .arg(QString::number(overallSum / overallCount, 'f', 2), unit));
      minBadge->show();
      maxBadge->show();
      avgBadge->show();
    } else {
      minBadge->hide();
      maxBadge->hide();
      avgBadge->hide();
    }
  };

  connect(metricCombo, &QComboBox::currentIndexChanged, &dialog,
          [renderZoomChart](int) { renderZoomChart(); });

  renderZoomChart();
  dialog.exec();

  const QString finalKey = metricCombo->currentData().toString();
  int idx = m_metricCombo->findData(finalKey);
  if (idx >= 0 && idx != m_metricCombo->currentIndex()) {
    m_metricCombo->setCurrentIndex(idx);
  }
}
