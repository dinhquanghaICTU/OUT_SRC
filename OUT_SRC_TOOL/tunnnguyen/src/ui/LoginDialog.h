#pragma once

#include <QWidget>
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QAction>
#include <QString>

class LoginDialog : public QDialog {
    Q_OBJECT
public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog() override = default;

    QString getEmail() const;
    QString getPassword() const;

signals:
    void loginSubmitted(const QString &email, const QString &password);

private slots:
    void onTogglePasswordVisibility();
    void onLoginClicked();
    void onCopyHwidClicked();

private:
    void setupUi();
    QWidget* createLeftForm();
    QWidget* createRightBanner();

    QLineEdit *m_emailEdit = nullptr;
    QLineEdit *m_passwordEdit = nullptr;
    QPushButton *m_loginBtn = nullptr;
    QAction *m_togglePasswordAction = nullptr;
    bool m_passwordVisible = false;
    QString m_hwid;
};
