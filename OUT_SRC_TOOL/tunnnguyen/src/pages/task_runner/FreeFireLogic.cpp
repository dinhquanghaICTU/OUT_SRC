#include "FreeFireLogic.h"
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <QRandomGenerator>
#include <QThread>

bool FreeFireLogic::executeAccount(const FreeFireTaskConfig &cfg, TaskLoggerFunc log) {
    if (!log) {
        log = [](const QString&, const QString&) {};
    }

    log(QString(">>> Bắt đầu xử lý tài khoản: %1 | Pass: %2").arg(cfg.uid, cfg.newPass), "TASK");

    QString profilePath = QString("C:/temp/tunn_chrome_%1").arg(cfg.uid);

    QStringList args;
    args << QString("--user-data-dir=%1").arg(profilePath);
    args << "--no-first-run";
    args << "--no-default-browser-check";
    args << "--new-window";

    QString targetUrl = "https://account.garena.com";
    args << targetUrl;

    log(QString("Đang mở Chrome Profile độc lập cho UID %1...").arg(cfg.uid), "SETUP");

    QString chromeExe = "chrome.exe";
    if (QFile::exists("C:/Program Files/Google/Chrome/Application/chrome.exe")) {
        chromeExe = "C:/Program Files/Google/Chrome/Application/chrome.exe";
    } else if (QFile::exists("C:/Program Files (x86)/Google/Chrome/Application/chrome.exe")) {
        chromeExe = "C:/Program Files (x86)/Google/Chrome/Application/chrome.exe";
    }

    bool started = QProcess::startDetached(chromeExe, args);

    if (!started) {
        log("Lỗi: Không thể khởi chạy Google Chrome!", "ERROR");
        return false;
    }

    log(QString("Đã mở Chrome thành công vào URL: %1").arg(targetUrl), "SUCCESS");
    log(QString("Đang gửi lệnh ADB tới thiết bị [%1] để đồng bộ...").arg(cfg.adbDevice), "ADB");

    // Giả lập thời gian thao tác
    QThread::sleep(2);

    return true;
}
