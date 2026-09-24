#include "mainwindow.h"
#include "ui/LoginDialog.h"

#include <QApplication>
#include <QFont>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    QFont appFont("Google Sans");
    appFont.setStyleHint(QFont::SansSerif);
    appFont.setPointSize(10);
    a.setFont(appFont);
    
    LoginDialog loginDialog;
//    if (loginDialog.exec() == QDialog::Accepted) {
        MainWindow w;
        w.show();
        return a.exec();
//    }

    return 0;
}
