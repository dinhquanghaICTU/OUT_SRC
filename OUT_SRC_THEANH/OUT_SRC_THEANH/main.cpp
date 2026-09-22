#include "EmbeddedServer.h"
#include "ui/MainWindow.h"

#include <QApplication>
#include <QFile>
#include <QMessageBox>
#include <QTimer>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("Hệ Thống Giám Sát Điện Năng ACS712 & ZMPT101B"));

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
    window.setMinimumSize(320, 240);
    window.resize(800, 480);
    window.show();

    if (argc >= 3 && QString::fromLatin1(argv[1]) == QStringLiteral("--screenshot")) {
        const QString outPath = QString::fromLatin1(argv[2]);
        window.loginAdminDirectly();
        QTimer::singleShot(2500, [&window, outPath]() {
            window.grab().save(outPath);
            qApp->quit();
        });
    } else if (argc >= 3 && QString::fromLatin1(argv[1]) == QStringLiteral("--screenshot-history")) {
        const QString outPath = QString::fromLatin1(argv[2]);
        window.showHistoryTable();
        QTimer::singleShot(3000, [&window, outPath]() {
            window.grab().save(outPath);
            qApp->quit();
        });
    } else if (argc >= 3 && QString::fromLatin1(argv[1]) == QStringLiteral("--screenshot-chart")) {
        const QString outPath = QString::fromLatin1(argv[2]);
        window.showHistoryChart();
        QTimer::singleShot(3000, [&window, outPath]() {
            window.grab().save(outPath);
            qApp->quit();
        });
    } else if (argc >= 3 && QString::fromLatin1(argv[1]) == QStringLiteral("--screenshot-users")) {
        const QString outPath = QString::fromLatin1(argv[2]);
        window.showUserManagement();
        QTimer::singleShot(3000, [&window, outPath]() {
            window.grab().save(outPath);
            qApp->quit();
        });
    } else if (argc >= 3 && QString::fromLatin1(argv[1]) == QStringLiteral("--screenshot-drawer")) {
        const QString outPath = QString::fromLatin1(argv[2]);
        window.showDeviceDrawer();
        QTimer::singleShot(3000, [&window, outPath]() {
            window.grab().save(outPath);
            qApp->quit();
        });
    }

    return app.exec();
}
