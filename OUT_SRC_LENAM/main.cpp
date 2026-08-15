#include "EmbeddedServer.h"
#include "ui/MainWindow.h"

#include <QApplication>
#include <QFile>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("LeNam Smart Monitor"));

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
    return app.exec();
}
