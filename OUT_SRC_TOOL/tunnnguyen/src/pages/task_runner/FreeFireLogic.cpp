#include "FreeFireLogic.h"
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <QRandomGenerator>
#include <QThread>

// ============================================================================
// >>> NƠI BẠN TỰ DO VIẾT & SỬA CODE LOGIC KỊCH BẢN TẠI ĐÂY <<<
// ============================================================================
bool FreeFireLogic::executeAccount(const FreeFireTaskConfig &cfg, TaskLoggerFunc log) {
    if (!log) {
        log = [](const QString&, const QString&) {};
    }

    log(QString(">>> Bắt đầu xử lý tài khoản: %1 | Pass: %2").arg(cfg.uid, cfg.newPass), "TASK");


    QString profilePath = QString("/tmp/tunn_chrome_%1").arg(cfg.uid);

    QStringList args;
    args << QString("--user-data-dir=%1").arg(profilePath);
    args << "--no-first-run";
    args << "--no-default-browser-check";

    args << "--new-window";

    // Trang cần vào:
    QString targetUrl = "https://account.garena.com";
    args << targetUrl;

    log(QString("Đang mở Chrome Profile độc lập cho UID %1...").arg(cfg.uid), "SETUP");

    QProcess process;
    // Bật Chrome và đợi hoặc chạy độc lập:
    bool started = QProcess::startDetached("google-chrome", args);

    if (!started) {
        log("Lỗi: Không thể khởi chạy Google Chrome!", "ERROR");
        return false; // Thất bại
    }

    log(QString("Đã mở Chrome thành công vào URL: %1").arg(targetUrl), "SUCCESS");

    // Giả lập thời gian thao tác (chờ 3 giây trước khi sang nick kế tiếp)
    QThread::sleep(3);

    // QUAN TRỌNG: Phải return true để thanh báo cáo tính là THÀNH CÔNG!
    return true;
}

