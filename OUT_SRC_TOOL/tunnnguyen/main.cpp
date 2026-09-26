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
        if (int idx = a.arguments().indexOf("--nav-page"); idx != -1 && idx + 1 < a.arguments().size()) {
            w.onNavButtonClicked(a.arguments()[idx + 1].toInt());
        } else if (a.arguments().contains("--page-task-runner")) {
            w.openTaskRunnerPage(0);
        } else if (a.arguments().contains("--page-ff-config")) {
            w.openTaskRunnerPage(1);
        } else if (a.arguments().contains("--run-ff-demo")) {
            w.openTaskRunnerPage(1, true);
        }
        w.show();
        return a.exec();
//    }

    return 0;
}
