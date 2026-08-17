#pragma once

#include <QWidget>

namespace Ui { class LoginPage; }

class VirtualKeyboard;

class LoginPage : public QWidget
{
    Q_OBJECT

public:
    explicit LoginPage(QWidget *parent = nullptr);
    ~LoginPage() override;

signals:
    void loginRequested(const QString &username, const QString &password);

protected:
    void hideEvent(QHideEvent *event) override;

private:
    Ui::LoginPage *ui;
    VirtualKeyboard *m_keyboard = nullptr;
};
