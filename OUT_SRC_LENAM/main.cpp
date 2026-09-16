#include "EmbeddedServer.h"
#include "mainwindow.h"

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>
#include <QFont>
#include <QMessageBox>
#include <QScreen>
#include <QTimer>

int main(int argc, char *argv[])
{
    qputenv("QT_IM_MODULE", "qtvirtualkeyboard");
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("Lê Nam - Hệ Thống Giám Sát Khí Quyển & Chuyển Động"));
    QApplication::setApplicationVersion(QStringLiteral("1.0.0"));

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Hệ thống giám sát khí quyển và chuyển động theo thời gian thực - Dự án của Lê Nam"));
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption portOption({QStringLiteral("p"), QStringLiteral("port")},
                                  QStringLiteral("HTTP API listen port"), QStringLiteral("port"),
                                  QStringLiteral("8080"));
    QCommandLineOption dbOption({QStringLiteral("d"), QStringLiteral("database")},
                                QStringLiteral("SQLite database path"), QStringLiteral("path"));
    QCommandLineOption mqttHostOption(QStringLiteral("mqtt-host"),
                                      QStringLiteral("MQTT broker host"), QStringLiteral("host"),
                                      QStringLiteral("127.0.0.1"));
    QCommandLineOption mqttPortOption(QStringLiteral("mqtt-port"),
                                      QStringLiteral("MQTT broker port"), QStringLiteral("port"),
                                      QStringLiteral("1883"));
    QCommandLineOption noHotspotOption(QStringLiteral("no-hotspot"),
                                       QStringLiteral("Không bật WiFi AP bằng nmcli"));
    QCommandLineOption hotspotIfaceOption(QStringLiteral("hotspot-iface"),
                                          QStringLiteral("WiFi interface phát AP"), QStringLiteral("iface"),
                                          QStringLiteral("wlan1"));
    QCommandLineOption hotspotSsidOption(QStringLiteral("hotspot-ssid"),
                                         QStringLiteral("SSID phát cho ESP"), QStringLiteral("ssid"),
                                         QStringLiteral("ICTU_IOT_AP"));
    QCommandLineOption hotspotPassOption(QStringLiteral("hotspot-pass"),
                                         QStringLiteral("Mật khẩu WiFi AP"), QStringLiteral("password"),
                                         QStringLiteral("12345678"));
    QCommandLineOption fullscreenOption({QStringLiteral("f"), QStringLiteral("fullscreen")},
                                        QStringLiteral("Chạy chế độ toàn màn hình cho màn hình 7 inch"));
    QCommandLineOption autoLoginOption(QStringLiteral("auto-login"),
                                       QStringLiteral("Tự động đăng nhập vào trạm"));
    QCommandLineOption initialPageOption(QStringLiteral("page"),
                                         QStringLiteral("Trang khởi động (0: Quan trắc, 1: Trạm đo, 2: Biểu đồ, 3: Quản trị)"),
                                         QStringLiteral("index"), QStringLiteral("0"));
    QCommandLineOption dateOption(QStringLiteral("date"),
                                  QStringLiteral("Ngày hiển thị trên đồ thị (YYYY-MM-DD)"),
                                  QStringLiteral("date_str"), QStringLiteral(""));
    QCommandLineOption tabOption(QStringLiteral("tab"),
                                 QStringLiteral("Tab đồ thị con (0: Áp suất, 1: Chuyển động, 2: Nhiệt độ, 3: Đa kênh, 4: Bảng biểu)"),
                                 QStringLiteral("tab_idx"), QStringLiteral("-1"));
    QCommandLineOption screenshotOption(QStringLiteral("screenshot"),
                                        QStringLiteral("Chụp ảnh cửa sổ vào file và thoát"),
                                        QStringLiteral("path"), QStringLiteral(""));

    parser.addOption(portOption);
    parser.addOption(dbOption);
    parser.addOption(mqttHostOption);
    parser.addOption(mqttPortOption);
    parser.addOption(noHotspotOption);
    parser.addOption(hotspotIfaceOption);
    parser.addOption(hotspotSsidOption);
    parser.addOption(hotspotPassOption);
    parser.addOption(fullscreenOption);
    parser.addOption(autoLoginOption);
    parser.addOption(initialPageOption);
    parser.addOption(dateOption);
    parser.addOption(tabOption);
    parser.addOption(screenshotOption);
    parser.process(app);

    bool portOk = false;
    const int port = parser.value(portOption).toInt(&portOk);
    bool mqttPortOk = false;
    const int mqttPort = parser.value(mqttPortOption).toInt(&mqttPortOk);
    if (!portOk || port < 1 || port > 65535 || !mqttPortOk || mqttPort < 1 || mqttPort > 65535) {
        QMessageBox::critical(nullptr, QStringLiteral("Lỗi cấu hình"),
                              QStringLiteral("Port API hoặc MQTT không hợp lệ."));
        return 2;
    }

    EmbeddedServer::Config serverConfig;
    serverConfig.httpPort = quint16(port);
    serverConfig.databasePath = parser.value(dbOption);
    serverConfig.mqttHost = parser.value(mqttHostOption);
    serverConfig.mqttPort = quint16(mqttPort);
    serverConfig.enableHotspot = !parser.isSet(noHotspotOption);
    serverConfig.hotspotInterface = parser.value(hotspotIfaceOption);
    serverConfig.hotspotSsid = parser.value(hotspotSsidOption);
    serverConfig.hotspotName = serverConfig.hotspotSsid;
    serverConfig.hotspotPassword = parser.value(hotspotPassOption);

    EmbeddedServer embeddedServer;
    QString serverError;
    if (!embeddedServer.start(serverConfig, &serverError)) {
        QMessageBox::critical(nullptr, QStringLiteral("Không khởi động được server nội bộ"), serverError);
        return 5;
    }

    QFont font(QStringLiteral("Inter"));
    font.setPointSize(10);
    app.setFont(font);

    MainWindow w;
    w.setBaseUrl(QStringLiteral("http://127.0.0.1:%1").arg(port));
    w.setMinimumSize(780, 440);
    w.resize(800, 480);

    const auto screens = QGuiApplication::screens();
    const QRect primaryGeo = (!screens.isEmpty() && screens.first()) ? screens.first()->geometry() : QRect(0, 0, 800, 480);
    const bool isSmallTouchDisplay = (primaryGeo.width() <= 800 && primaryGeo.height() <= 480);
    if (parser.isSet(fullscreenOption) || isSmallTouchDisplay) {
        w.showFullScreen();
    } else {
        w.show();
    }

    if (parser.isSet(autoLoginOption)) {
        const int targetPage = parser.value(initialPageOption).toInt();
        const QString customDateStr = parser.value(dateOption);
        const int subTab = parser.isSet(tabOption) ? parser.value(tabOption).toInt() : -1;

        QTimer::singleShot(250, &w, [&w, targetPage, customDateStr, subTab]{
            w.triggerLogin(QStringLiteral("admin"), QStringLiteral("admin"), targetPage);
            if (!customDateStr.isEmpty()) {
                const QDate d = QDate::fromString(customDateStr, Qt::ISODate);
                if (d.isValid()) {
                    w.setInitialHistoryDate(d);
                }
            }
            if (subTab >= 0) {
                w.setHistorySubTab(subTab);
            }
        });
    }

    if (parser.isSet(screenshotOption)) {
        const QString shotPath = parser.value(screenshotOption);
        QTimer::singleShot(2500, [&w, shotPath, &app]{
            QPixmap pix = w.grab();
            pix.save(shotPath);
            qDebug() << "Captured screenshot saved to:" << shotPath;
            app.quit();
        });
    }

    return app.exec();
}
