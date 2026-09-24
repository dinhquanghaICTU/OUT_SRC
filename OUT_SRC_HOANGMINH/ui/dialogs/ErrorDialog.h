#pragma once

#include <QDialog>

class ErrorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ErrorDialog(const QString &title,
                         const QString &message,
                         const QString &details = {},
                         QWidget *parent = nullptr);

    static void showLoginError(QWidget *parent, const QString &technicalMessage);
};
