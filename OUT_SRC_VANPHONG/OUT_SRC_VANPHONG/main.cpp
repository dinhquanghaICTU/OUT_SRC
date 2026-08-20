#include "EmbeddedServer.h"
#include "ui/MainWindow.h"

#include <QApplication>
#include <QFile>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    // Tự động nhận diện: Nếu có X11 Forwarding (DISPLAY) thì dùng xcb để hiện lên PC
    // Nếu chạy trực tiếp trên Pi không qua X11 thì dùng eglfs
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) {
        if (!qEnvironmentVariableIsEmpty("DISPLAY")) {
            qputenv("QT_QPA_PLATFORM", "xcb");
        } else {
            qputenv("QT_QPA_PLATFORM", "eglfs");
            qputenv("QT_QPA_EGLFS_INTEGRATION", "eglfs_kms");
        }
    }
    if (qEnvironmentVariableIsEmpty("QT_QPA_FONTDIR")) {
        qputenv("QT_QPA_FONTDIR", "/usr/share/fonts/truetype");
    }
    if (qEnvironmentVariableIsEmpty("QT_QPA_GENERIC_PLUGINS")) {
        qputenv("QT_QPA_GENERIC_PLUGINS", "libinput");
    }

    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("Van Phong Smart Agri Irrigation"));

    QFile styleFile(QStringLiteral(":/styles/app.qss"));
    if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        app.setStyleSheet(QString::fromUtf8(styleFile.readAll()));
    }

    EmbeddedServer embeddedServer;
    QString serverError;
    if (!embeddedServer.start(&serverError)) {
        QMessageBox::critical(nullptr, QStringLiteral("Không thể khởi động Server nội bộ"), serverError);
        return 5;
    }

    MainWindow window;
    window.setMinimumSize(800, 480);
    window.resize(800, 480);
    window.show();
    return app.exec();
}
