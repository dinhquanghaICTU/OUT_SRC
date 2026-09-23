#include "EmbeddedServer.h"
#include "ui/MainWindow.h"
#include "ui/pages/HistoryPage.h"

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDate>
#include <QDialog>
#include <QFile>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QScreen>
#include <QScrollArea>
#include <QTimer>
#include <QVBoxLayout>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("TrungKien Smart Monitor"));

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption autoLoginOption(QStringLiteral("auto-login"), QStringLiteral("Tự động đăng nhập"));
    QCommandLineOption pageOption(QStringLiteral("page"), QStringLiteral("Trang mở sẵn (0:Dash, 2:Devices, 3:History, 4:Users)"), QStringLiteral("index"), QStringLiteral("0"));
    QCommandLineOption periodOption(QStringLiteral("period"), QStringLiteral("Khoảng thời gian thống kê (day, month, year)"), QStringLiteral("period"), QStringLiteral(""));
    QCommandLineOption dateOption(QStringLiteral("date"), QStringLiteral("Ngày thống kê (YYYY-MM-DD)"), QStringLiteral("date"), QStringLiteral(""));
    QCommandLineOption tabOption(QStringLiteral("tab"), QStringLiteral("Tab biểu đồ (0) hoặc bảng (1)"), QStringLiteral("tab"), QStringLiteral("-1"));
    QCommandLineOption screenshotOption(QStringLiteral("screenshot"), QStringLiteral("Chụp màn hình rồi thoát"), QStringLiteral("path"), QStringLiteral(""));
    QCommandLineOption delayOption(QStringLiteral("delay"), QStringLiteral("Thời gian chờ chụp màn hình (ms)"), QStringLiteral("ms"), QStringLiteral("1200"));
    QCommandLineOption popupOption(QStringLiteral("popup"), QStringLiteral("Mở sẵn popup test (add, delete)"), QStringLiteral("type"), QStringLiteral(""));

    parser.addOption(autoLoginOption);
    parser.addOption(pageOption);
    parser.addOption(periodOption);
    parser.addOption(dateOption);
    parser.addOption(tabOption);
    parser.addOption(screenshotOption);
    parser.addOption(delayOption);
    parser.addOption(popupOption);
    parser.process(app);

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

    const QString popupType = parser.value(popupOption);
    const QString screenshotPath = parser.value(screenshotOption);
    const int delayMs = parser.value(delayOption).toInt();

    if (popupType == QStringLiteral("error") || popupType == QStringLiteral("login_error")) {
        QTimer::singleShot(400, &window, [&window, screenshotPath, delayMs] {
            window.triggerLogin(QStringLiteral("admin"), QStringLiteral("sai_mat_khau_123"), 0);

            if (!screenshotPath.isEmpty()) {
                QTimer::singleShot(delayMs, &window, [screenshotPath] {
                    QScreen *screen = QGuiApplication::primaryScreen();
                    QPixmap pix = screen ? screen->grabWindow(0) : QPixmap();
                    pix.save(screenshotPath);
                    QApplication::quit();
                });
            }
        });
    } else if (parser.isSet(autoLoginOption)) {
        const QString pageStr = parser.value(pageOption);
        int pageIdx = 0;
        if (pageStr == QStringLiteral("history") || pageStr == QStringLiteral("3")) {
            pageIdx = 3;
        } else if (pageStr == QStringLiteral("devices") || pageStr == QStringLiteral("2")) {
            pageIdx = 2;
        } else if (pageStr == QStringLiteral("users") || pageStr == QStringLiteral("4")) {
            pageIdx = 4;
        } else {
            pageIdx = pageStr.toInt();
        }
        const QString period = parser.value(periodOption);
        const QString dateStr = parser.value(dateOption);
        const int tabIdx = parser.isSet(tabOption) ? parser.value(tabOption).toInt() : -1;

        QTimer::singleShot(300, &window, [&window, pageIdx, period, dateStr, tabIdx, screenshotPath, delayMs, popupType] {
            window.triggerLogin(QStringLiteral("admin"), QStringLiteral("1"), pageIdx);

            QTimer::singleShot(700, &window, [&window, period, dateStr, tabIdx, screenshotPath, delayMs, popupType] {
                if (window.historyPage()) {
                    if (!period.isEmpty())
                        window.historyPage()->setPeriod(period);
                    if (!dateStr.isEmpty()) {
                        const QDate d = QDate::fromString(dateStr, Qt::ISODate);
                        if (d.isValid())
                            window.historyPage()->setDate(d);
                    }
                    if (tabIdx >= 0)
                        window.historyPage()->setViewTab(tabIdx);
                }

                if (popupType == QStringLiteral("add")) {
                    auto *dlg = new QDialog(&window);
                    dlg->setObjectName(QStringLiteral("claimDeviceDialog"));
                    dlg->setWindowTitle(QObject::tr("Thêm & Đặt tên trạm đo"));
                    dlg->setFixedWidth(440);
                    dlg->setMaximumHeight(280);

                    dlg->setStyleSheet(QStringLiteral(
                        "QDialog#claimDeviceDialog { background-color: #0b152d; border: 1.5px solid #1c2b54; border-radius: 10px; } "
                        "QWidget#claimBody { background-color: #0b152d; } "
                        "QScrollArea { background: transparent; border: none; } "
                        "QLabel#claimDeviceDialogTitle { color: #38bdf8; font-size: 15px; font-weight: 900; } "
                        "QLabel#claimDeviceDialogHint { color: #94a3b8; font-size: 11px; font-weight: 600; } "
                        "QLabel#claimDeviceDialogIcon { font-size: 26px; color: #38bdf8; background: #111d3d; border: 1px solid #233870; border-radius: 8px; padding: 6px 12px; } "
                        "QLabel#claimDeviceInfo { color: #f59e0b; font-size: 11px; font-weight: 700; background: rgba(245, 158, 11, 0.12); border: 1px solid #b45309; border-radius: 6px; padding: 6px 10px; } "
                        "QLineEdit#claimDeviceNameInput { background-color: #070d1e; color: #ffffff; border: 1.5px solid #233870; border-radius: 6px; padding: 6px 10px; font-size: 12px; font-weight: 700; min-height: 28px; } "
                        "QPushButton#claimDeviceCancelButton { background-color: #1e293b; color: #cbd5e1; border: 1px solid #334155; border-radius: 6px; padding: 8px 16px; font-size: 11px; font-weight: 800; min-height: 28px; } "
                        "QPushButton#claimDeviceSaveButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #059669, stop:1 #047857); color: #ffffff; border: 1px solid #10b981; border-radius: 6px; padding: 8px 18px; font-size: 12px; font-weight: 900; min-height: 28px; } "
                    ));

                    auto *root = new QVBoxLayout(dlg);
                    root->setContentsMargins(14, 12, 14, 12);
                    root->setSpacing(10);
                    auto *scroll = new QScrollArea(dlg);
                    scroll->setWidgetResizable(true);
                    scroll->setFrameShape(QFrame::NoFrame);
                    auto *body = new QWidget(scroll);
                    body->setObjectName(QStringLiteral("claimBody"));
                    auto *bodyLayout = new QVBoxLayout(body);
                    bodyLayout->setContentsMargins(10, 10, 10, 10);
                    bodyLayout->setSpacing(12);
                    scroll->setWidget(body);
                    root->addWidget(scroll, 1);

                    auto *head = new QHBoxLayout;
                    auto *dialogIcon = new QLabel(QStringLiteral("️"), body);
                    dialogIcon->setObjectName(QStringLiteral("claimDeviceDialogIcon"));
                    dialogIcon->setAlignment(Qt::AlignCenter);
                    auto *titleBlock = new QVBoxLayout;
                    auto *title = new QLabel(QObject::tr("Thêm trạm đo mới"), body);
                    title->setObjectName(QStringLiteral("claimDeviceDialogTitle"));
                    auto *subtitle = new QLabel(QObject::tr("Đặt tên gợi nhớ để quản lý trạm đo tia UV & áp suất."), body);
                    subtitle->setObjectName(QStringLiteral("claimDeviceDialogHint"));
                    subtitle->setWordWrap(true);
                    titleBlock->addWidget(title);
                    titleBlock->addWidget(subtitle);
                    head->addWidget(dialogIcon);
                    head->addLayout(titleBlock, 1);
                    bodyLayout->addLayout(head);

                    auto *info = new QLabel(QObject::tr("ID: Trungkien-150304  •  Trạm đo tia UV & Áp suất"), body);
                    info->setObjectName(QStringLiteral("claimDeviceInfo"));
                    bodyLayout->addWidget(info);

                    auto *name = new QLineEdit(QObject::tr("Trạm Đo UV & Áp Suất Ban Công"), body);
                    name->setObjectName(QStringLiteral("claimDeviceNameInput"));
                    bodyLayout->addWidget(name);

                    auto *actions = new QHBoxLayout;
                    actions->setSpacing(10);
                    auto *cancel = new QPushButton(QObject::tr("Hủy"), dlg);
                    auto *save = new QPushButton(QObject::tr("Thêm trạm đo"), dlg);
                    cancel->setObjectName(QStringLiteral("claimDeviceCancelButton"));
                    save->setObjectName(QStringLiteral("claimDeviceSaveButton"));
                    actions->addWidget(cancel);
                    actions->addWidget(save);
                    root->addLayout(actions);

                    dlg->show();
                    dlg->raise();

                    if (!screenshotPath.isEmpty()) {
                        QTimer::singleShot(delayMs, dlg, [dlg, screenshotPath] {
                            QScreen *screen = QGuiApplication::primaryScreen();
                            QPixmap pix = screen ? screen->grabWindow(0) : dlg->grab();
                            pix.save(screenshotPath);
                            QApplication::quit();
                        });
                    }
                } else if (popupType == QStringLiteral("delete")) {
                    auto *msgBox = new QMessageBox(&window);
                    msgBox->setWindowTitle(QObject::tr("Xác nhận gỡ/xóa trạm đo"));
                    msgBox->setText(QObject::tr("Bạn có chắc chắn muốn gỡ trạm đo 'Trạm Đo UV & Áp Suất (Trung Kiên)' (ID: Trungkien-150304) khỏi tài khoản?\nTrạm đo sẽ trở lại danh sách có thể thêm."));
                    msgBox->setIcon(QMessageBox::Warning);
                    auto *yesBtn = msgBox->addButton(QObject::tr("Xác nhận gỡ/xóa"), QMessageBox::YesRole);
                    auto *noBtn = msgBox->addButton(QObject::tr("Hủy bỏ"), QMessageBox::NoRole);
                    msgBox->setDefaultButton(noBtn);
                    msgBox->setStyleSheet(QStringLiteral(
                        "QMessageBox { background-color: #0b152d; border: 1.5px solid #1c2b54; border-radius: 8px; } "
                        "QLabel { color: #f1f5f9; font-size: 12px; font-weight: 700; background: transparent; } "
                        "QPushButton { min-width: 100px; min-height: 30px; border-radius: 6px; font-size: 11px; font-weight: 800; padding: 6px 14px; } "
                    ));
                    yesBtn->setStyleSheet(QStringLiteral(
                        "QPushButton { background-color: #dc2626; color: #ffffff; border: 1px solid #ef4444; border-radius: 6px; padding: 6px 14px; font-weight: 800; } "
                    ));
                    noBtn->setStyleSheet(QStringLiteral(
                        "QPushButton { background-color: #1e293b; color: #cbd5e1; border: 1px solid #334155; border-radius: 6px; padding: 6px 14px; font-weight: 800; } "
                    ));
                    msgBox->show();
                    msgBox->raise();

                    if (!screenshotPath.isEmpty()) {
                        QTimer::singleShot(delayMs, msgBox, [msgBox, screenshotPath] {
                            QScreen *screen = QGuiApplication::primaryScreen();
                            QPixmap pix = screen ? screen->grabWindow(0) : msgBox->grab();
                            pix.save(screenshotPath);
                            QApplication::quit();
                        });
                    }
                } else if (!screenshotPath.isEmpty()) {
                    QTimer::singleShot(delayMs, &window, [&window, screenshotPath] {
                        QPixmap pix = window.grab();
                        pix.save(screenshotPath);
                        QApplication::quit();
                    });
                }
            });
        });
    } else if (!screenshotPath.isEmpty()) {
        QTimer::singleShot(delayMs, &window, [&window, screenshotPath] {
            QScreen *screen = QGuiApplication::primaryScreen();
            QPixmap pix = screen ? screen->grabWindow(0) : window.grab();
            pix.save(screenshotPath);
            QApplication::quit();
        });
    }

    return app.exec();
}
