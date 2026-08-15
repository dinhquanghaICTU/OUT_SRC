#include "EmbeddedServer.h"
#include "mainwindow.h"

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>
#include <QFont>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("HoangAnh IoT Command Center"));
    QApplication::setApplicationVersion(QStringLiteral("1.0.0"));

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("HoangAnh IoT UI + API + MQTT service in one app"));
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

    parser.addOption(portOption);
    parser.addOption(dbOption);
    parser.addOption(mqttHostOption);
    parser.addOption(mqttPortOption);
    parser.addOption(noHotspotOption);
    parser.addOption(hotspotIfaceOption);
    parser.addOption(hotspotSsidOption);
    parser.addOption(hotspotPassOption);
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
    w.setMinimumSize(800, 480);
    w.resize(800, 480);
    w.show();
    return app.exec();
}
