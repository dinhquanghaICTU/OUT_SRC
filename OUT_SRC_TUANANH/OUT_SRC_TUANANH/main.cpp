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
#include <QPainter>
#include <QPixmap>
#include <QScreen>
#include <QTimer>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("Smart Ambient Room Lighting Monitor"));

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption autoLoginOption(QStringLiteral("auto-login"), QStringLiteral("Tự động đăng nhập admin"));
    QCommandLineOption pageOption(QStringLiteral("page"), QStringLiteral("Trang mở sẵn (0:Dash, 1:Devices, 2:History, 3:Users)"), QStringLiteral("index"), QStringLiteral("0"));
    QCommandLineOption screenshotOption(QStringLiteral("screenshot"), QStringLiteral("Chụp màn hình rồi thoát"), QStringLiteral("path"), QStringLiteral(""));
    QCommandLineOption delayOption(QStringLiteral("delay"), QStringLiteral("Thời gian chờ chụp màn hình (ms)"), QStringLiteral("ms"), QStringLiteral("1200"));
    QCommandLineOption userOption(QStringLiteral("username"), QStringLiteral("Tên tài khoản"), QStringLiteral("user"), QStringLiteral("admin"));
    QCommandLineOption passOption(QStringLiteral("password"), QStringLiteral("Mật khẩu"), QStringLiteral("pass"), QStringLiteral("admin123"));
    QCommandLineOption relayOption(QStringLiteral("relay"), QStringLiteral("Trạng thái đèn (0: Tắt, 1: Bật)"), QStringLiteral("state"), QStringLiteral("-1"));
    QCommandLineOption motionOption(QStringLiteral("motion"), QStringLiteral("Trạng thái PIR (0: Không có người, 1: Có người)"), QStringLiteral("val"), QStringLiteral("-1"));
    QCommandLineOption configOption(QStringLiteral("open-config"), QStringLiteral("Mở dialog cài đặt ngưỡng"));
    QCommandLineOption periodOption(QStringLiteral("history-period"), QStringLiteral("Khoảng thời gian lịch sử (day, month, year)"), QStringLiteral("period"), QStringLiteral(""));
    QCommandLineOption historyViewOption(QStringLiteral("history-view"), QStringLiteral("Xem bảng hay biểu đồ (chart, table)"), QStringLiteral("view"), QStringLiteral(""));

    parser.addOption(autoLoginOption);
    parser.addOption(pageOption);
    parser.addOption(screenshotOption);
    parser.addOption(delayOption);
    parser.addOption(userOption);
    parser.addOption(passOption);
    parser.addOption(relayOption);
    parser.addOption(motionOption);
    parser.addOption(configOption);
    parser.addOption(periodOption);
    parser.addOption(historyViewOption);
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
    window.setFixedSize(800, 480);
    window.show();

    const bool autoLogin = parser.isSet(autoLoginOption);
    const int pageIndex = parser.value(pageOption).toInt();
    const QString screenshotPath = parser.value(screenshotOption);
    const int delayMs = parser.value(delayOption).toInt();
    const QString username = parser.value(userOption);
    const QString password = parser.value(passOption);
    const int relayVal = parser.value(relayOption).toInt();
    const int motionVal = parser.value(motionOption).toInt();
    const bool openConfig = parser.isSet(configOption);
    const QString historyPeriod = parser.value(periodOption);
    const QString historyView = parser.value(historyViewOption);

    if (autoLogin) {
        window.autoLogin(username, password);
        QTimer::singleShot(600, [&window, pageIndex, screenshotPath, delayMs, relayVal, motionVal, openConfig, historyPeriod, historyView] {
            window.navigateToPage(pageIndex);
            if (!historyPeriod.isEmpty() && window.historyPage()) {
                window.historyPage()->setPeriod(historyPeriod);
            }
            if (historyView == QStringLiteral("table") && window.historyPage()) {
                window.historyPage()->showTableView(true);
            }
            if ((relayVal >= 0 || motionVal >= 0) && window.dashboardPage()) {
                window.dashboardPage()->updateDeviceMetrics(QJsonObject{
                    {QStringLiteral("relay"), relayVal == 1},
                    {QStringLiteral("relay_on"), relayVal == 1},
                    {QStringLiteral("light_lux"), 320.0},
                    {QStringLiteral("motion_detected"), motionVal == 1},
                    {QStringLiteral("motion"), motionVal == 1}
                });
            }
            if (openConfig && window.dashboardPage()) {
                QTimer::singleShot(300, [&window] {
                    window.dashboardPage()->openConfigDialog();
                });
            }
            if (!screenshotPath.isEmpty()) {
                QTimer::singleShot(delayMs + (openConfig ? 600 : 0), [&window, screenshotPath] {
                    QPixmap pix = window.grab();
                    // Also grab top level active modal if present
                    if (QWidget *activeModal = QApplication::activeModalWidget()) {
                        // paint window and modal together
                        QPixmap combined(window.size());
                        window.render(&combined);
                        QPainter p(&combined);
                        const QPoint modalPos = activeModal->mapToGlobal(QPoint(0, 0)) - window.mapToGlobal(QPoint(0, 0));
                        activeModal->render(&p, modalPos);
                        p.end();
                        combined.save(screenshotPath);
                    } else {
                        pix.save(screenshotPath);
                    }
                    QApplication::quit();
                });
            }
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
