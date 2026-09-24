#include "EmbeddedServer.h"
#include "ui/MainWindow.h"
#include "ui/pages/DashboardPage.h"
#include "ui/pages/DeviceManagementPage.h"
#include "ui/pages/HistoryPage.h"
#include "ui/pages/UserManagementPage.h"

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QFile>
#include <QMessageBox>
#include <QPixmap>
#include <QScreen>
#include <QTimer>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("Hoàng Minh Smart Cooling Monitor"));

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption autoLoginOption(QStringLiteral("auto-login"), QStringLiteral("Tự động đăng nhập"));
    QCommandLineOption pageOption(QStringLiteral("page"), QStringLiteral("Trang mở sẵn (0:Dash, 1:Devices, 3:History, 4:Users)"), QStringLiteral("index"), QStringLiteral("0"));
    QCommandLineOption screenshotOption(QStringLiteral("screenshot"), QStringLiteral("Chụp màn hình rồi thoát"), QStringLiteral("path"), QStringLiteral(""));
    QCommandLineOption delayOption(QStringLiteral("delay"), QStringLiteral("Thời gian chờ chụp màn hình (ms)"), QStringLiteral("ms"), QStringLiteral("1200"));
    QCommandLineOption popupOption(QStringLiteral("popup"), QStringLiteral("Mở sẵn popup test (config)"), QStringLiteral("type"), QStringLiteral(""));
    QCommandLineOption periodOption(QStringLiteral("period"), QStringLiteral("Đặt khoảng thời gian lịch sử (day, month, year)"), QStringLiteral("period"), QStringLiteral(""));

    parser.addOption(autoLoginOption);
    parser.addOption(pageOption);
    parser.addOption(screenshotOption);
    parser.addOption(delayOption);
    parser.addOption(popupOption);
    parser.addOption(periodOption);
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
    const QString periodVal = parser.value(periodOption);
    const int delayMs = parser.value(delayOption).toInt();

    if (parser.isSet(autoLoginOption)) {
        const int pageIdx = parser.value(pageOption).toInt();

        QTimer::singleShot(300, &window, [&window, pageIdx, screenshotPath, delayMs, popupType, periodVal] {
            window.triggerLogin(QStringLiteral("admin"), QStringLiteral("1"), pageIdx);

            if (!periodVal.isEmpty() && window.historyPage()) {
                window.historyPage()->setPeriod(periodVal);
            }

            QTimer::singleShot(800, &window, [&window, screenshotPath, delayMs, popupType] {
                if (popupType == QStringLiteral("config")) {
                    if (!screenshotPath.isEmpty()) {
                        QTimer::singleShot(delayMs, [&window, screenshotPath] {
                            QPixmap pix = window.grab();
                            pix.save(screenshotPath);
                            QApplication::quit();
                        });
                    }
                    if (window.dashboardPage()) {
                        window.dashboardPage()->openCoolingConfig();
                    }
                } else if (popupType == QStringLiteral("user-add")) {
                    if (!screenshotPath.isEmpty()) {
                        QTimer::singleShot(delayMs, [&window, screenshotPath] {
                            QScreen *screen = QGuiApplication::primaryScreen();
                            QPixmap pix = screen ? screen->grabWindow(0) : window.grab();
                            pix.save(screenshotPath);
                            QApplication::quit();
                        });
                    }
                    if (window.userManagementPage()) {
                        window.userManagementPage()->openEditDialog({});
                    }
                } else if (popupType == QStringLiteral("device-config")) {
                    if (!screenshotPath.isEmpty()) {
                        QTimer::singleShot(delayMs, [&window, screenshotPath] {
                            QScreen *screen = QGuiApplication::primaryScreen();
                            QPixmap pix = screen ? screen->grabWindow(0) : window.grab();
                            pix.save(screenshotPath);
                            QApplication::quit();
                        });
                    }
                    if (window.deviceManagementPage()) {
                        window.deviceManagementPage()->openFirstDeviceConfig();
                    }
                } else if (!screenshotPath.isEmpty()) {
                    QTimer::singleShot(delayMs, [&window, screenshotPath] {
                        QPixmap pix = window.grab();
                        pix.save(screenshotPath);
                        QApplication::quit();
                    });
                }
            });
        });
    } else if (!screenshotPath.isEmpty()) {
        QTimer::singleShot(delayMs, [&window, screenshotPath] {
            QPixmap pix = window.grab();
            pix.save(screenshotPath);
            QApplication::quit();
        });
    }

    return app.exec();
}
