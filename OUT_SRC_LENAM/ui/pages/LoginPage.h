#pragma once

#include <QWidget>

namespace Ui { class LoginPage; }

class LoginPage : public QWidget
{
    Q_OBJECT

public:
    explicit LoginPage(QWidget *parent = nullptr);
    ~LoginPage() override;

signals:
    void loginRequested(const QString &username, const QString &password);

private:
    Ui::LoginPage *ui;
};
