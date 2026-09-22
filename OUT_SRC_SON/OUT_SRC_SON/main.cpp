#include "EmbeddedServer.h"
#include "ui/MainWindow.h"
#include "ui/pages/DashboardPage.h"
#include "ui/pages/HistoryPage.h"
#include "ui/dialogs/SelectOnlineDeviceDialog.h"

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDate>
#include <QFile>
#include <QMessageBox>
#include <QPixmap>
#include <QScreen>
#include <QTimer>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("Environmental Monitor"));

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption autoLoginOption(QStringLiteral("auto-login"), QStringLiteral("Tự động đăng nhập"));
    QCommandLineOption pageOption(QStringLiteral("page"), QStringLiteral("Trang mở sẵn (0:Dash, 2:Devices, 3:History, 4:Users)"), QStringLiteral("index"), QStringLiteral("0"));
    QCommandLineOption periodOption(QStringLiteral("period"), QStringLiteral("Khoảng thời gian thống kê (day, month, year)"), QStringLiteral("period"), QStringLiteral(""));
    QCommandLineOption dateOption(QStringLiteral("date"), QStringLiteral("Ngày thống kê (YYYY-MM-DD)"), QStringLiteral("date"), QStringLiteral(""));
    QCommandLineOption tabOption(QStringLiteral("tab"), QStringLiteral("Tab biểu đồ (0) hoặc bảng (1)"), QStringLiteral("tab"), QStringLiteral("-1"));
    QCommandLineOption screenshotOption(QStringLiteral("screenshot"), QStringLiteral("Chụp màn hình rồi thoát"), QStringLiteral("path"), QStringLiteral(""));
    QCommandLineOption delayOption(QStringLiteral("delay"), QStringLiteral("Thời gian chờ chụp màn hình (ms)"), QStringLiteral("ms"), QStringLiteral("1200"));
    QCommandLineOption popupOption(QStringLiteral("popup"), QStringLiteral("Mở sẵn popup test (config, threshold)"), QStringLiteral("type"), QStringLiteral(""));

    parser.addOption(autoLoginOption);
    parser.addOption(pageOption);
    parser.addOption(periodOption);
    parser.addOption(dateOption);
    parser.addOption(tabOption);
    parser.addOption(screenshotOption);
    parser.addOption(delayOption);
    parser.addOption(popupOption);
    parser.process(app);

    QFile styleFile(QStringLiteral(":/styles/app.qss"));
    if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text))
        app.setStyleSheet(QString::fromUtf8(styleFile.readAll()));

    EmbeddedServer embeddedServer;
    QString serverError;
    if (!embeddedServer.start(&serverError)) {
        QMessageBox::critical(nullptr, QStringLiteral("Không khởi động được server nội bộ"), serverError);
        return 5;
    }

    MainWindow window;
    window.setMinimumSize(800, 480);
    window.resize(800, 480);
    window.show();

    const QString popupType = parser.value(popupOption);
    const QString screenshotPath = parser.value(screenshotOption);
    const int delayMs = parser.value(delayOption).toInt();

    if (parser.isSet(autoLoginOption)) {
        const QString pageStr = parser.value(pageOption);
        int pageIdx = 0;
        if (pageStr == QStringLiteral("history") || pageStr == QStringLiteral("3")) {
            pageIdx = 3;
        } else if (pageStr == QStringLiteral("devices") || pageStr == QStringLiteral("2")) {
            pageIdx = 2;
        } else if (pageStr == QStringLiteral("users") || pageStr == QStringLiteral("4")) {
            pageIdx = 4;
        } else {
            pageIdx = pageStr.toInt();
        }
        const QString period = parser.value(periodOption);
        const QString dateStr = parser.value(dateOption);
        const int tabIdx = parser.isSet(tabOption) ? parser.value(tabOption).toInt() : -1;

        QTimer::singleShot(300, &window, [&window, pageIdx, period, dateStr, tabIdx, screenshotPath, delayMs, popupType] {
            window.triggerLogin(QStringLiteral("admin"), QStringLiteral("1"), pageIdx);

            QTimer::singleShot(800, &window, [&window, period, dateStr, tabIdx, screenshotPath, delayMs, popupType] {
                if (window.historyPage()) {
                    if (!period.isEmpty())
                        window.historyPage()->setPeriod(period);
                    if (!dateStr.isEmpty()) {
                        const QDate d = QDate::fromString(dateStr, Qt::ISODate);
                        if (d.isValid())
                            window.historyPage()->setDate(d);
                    }
                    if (tabIdx >= 0)
                        window.historyPage()->setViewTab(tabIdx);
                }

                if (popupType == QStringLiteral("select-device") || popupType == QStringLiteral("add-device")) {
                    QJsonArray testDevs;
                    testDevs.append(QJsonObject{
                        {QStringLiteral("device_id"), QStringLiteral("son-190782")},
                        {QStringLiteral("name"), QStringLiteral("Trạm Bơm Tự Động & Mực Nước")},
                        {QStringLiteral("online"), true},
                        {QStringLiteral("firmware_version"), QStringLiteral("1.0.0")}
                    });
                    SelectOnlineDeviceDialog dlg(testDevs, &window);
                    if (!screenshotPath.isEmpty()) {
                        QTimer::singleShot(delayMs, [&dlg, screenshotPath] {
                            QScreen *screen = QGuiApplication::primaryScreen();
                            QPixmap pix = screen ? screen->grabWindow(0) : QPixmap();
                            pix.save(screenshotPath);
                            QApplication::quit();
                        });
                    }
                    dlg.exec();
                } else if (popupType == QStringLiteral("config") || popupType == QStringLiteral("threshold")) {
                    if (!screenshotPath.isEmpty()) {
                        QTimer::singleShot(delayMs, &window, [screenshotPath] {
                            QScreen *screen = QGuiApplication::primaryScreen();
                            QPixmap pix = screen ? screen->grabWindow(0) : QPixmap();
                            pix.save(screenshotPath);
                            QApplication::quit();
                        });
                    }
                    if (window.dashboardPage()) {
                        window.dashboardPage()->openPumpAutoConfig();
                    }
                } else if (!screenshotPath.isEmpty()) {
                    QTimer::singleShot(delayMs, &window, [&window, screenshotPath] {
                        QScreen *screen = QGuiApplication::primaryScreen();
                        QPixmap pix = screen ? screen->grabWindow(0) : window.grab();
                        pix.save(screenshotPath);
                        QApplication::quit();
                    });
                }
            });
        });
    } else if (!screenshotPath.isEmpty()) {
        // Screenshot without login (LoginPage)
        QTimer::singleShot(delayMs, &window, [&window, screenshotPath] {
            QScreen *screen = QGuiApplication::primaryScreen();
            QPixmap pix = screen ? screen->grabWindow(0) : window.grab();
            pix.save(screenshotPath);
            QApplication::quit();
        });
    }

    return app.exec();
}
