#pragma once

#include <QDialog>
#include <QString>
#include <QWidget>

class CustomMessageBox : public QDialog {
    Q_OBJECT
public:
    enum IconType {
        Warning,
        Information,
        Critical
    };

    explicit CustomMessageBox(IconType type, const QString &title, const QString &message, QWidget *parent = nullptr);
    ~CustomMessageBox() override = default;

    static void warning(QWidget *parent, const QString &title, const QString &message);
    static void information(QWidget *parent, const QString &title, const QString &message);
    static void critical(QWidget *parent, const QString &title, const QString &message);

private:
    void setupUi(IconType type, const QString &title, const QString &message);
};
