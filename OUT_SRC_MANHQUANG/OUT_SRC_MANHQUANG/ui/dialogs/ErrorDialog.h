#pragma once

#include <QDialog>
#include <QString>

class QLabel;
class QPushButton;

class ErrorDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ErrorDialog(QWidget *parent = nullptr);
    static void showLoginError(QWidget *parent, const QString &message);
    static void showCustomError(QWidget *parent, const QString &title, const QString &message);

private:
    QLabel *m_iconLabel;
    QLabel *m_titleLabel;
    QLabel *m_messageLabel;
    QPushButton *m_closeBtn;
};
