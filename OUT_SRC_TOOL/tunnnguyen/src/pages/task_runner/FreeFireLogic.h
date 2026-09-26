#ifndef FREEFIRELOGIC_H
#define FREEFIRELOGIC_H

#include <QString>
#include <functional>

// Kiểu hàm callback dùng để in log ra màn hình Console UI
using TaskLoggerFunc = std::function<void(const QString &msg, const QString &type)>;

struct FreeFireTaskConfig {
    QString uid;
    QString oldPass;
    QString newPass;
    QString newEmail;
    QString adbDevice;
    QString chromeProfile;
    bool changePassword = true;
    bool changeEmail = true;
    bool removePhone = true;
    bool exportResult = true;
    int delaySeconds = 2;
    int threadCount = 2;
};

class FreeFireLogic {
public:
    // ========================================================================
    // >>> HÀM ĐỂ BẠN TỰ DO VIẾT LOGIC CHO KỊCH BẢN AUTO CHANGE INFO FREE FIRE <<<
    // Trả về true nếu thành công, false nếu thất bại
    // ========================================================================
    static bool executeAccount(const FreeFireTaskConfig &config, TaskLoggerFunc log);
};

#endif // FREEFIRELOGIC_H
