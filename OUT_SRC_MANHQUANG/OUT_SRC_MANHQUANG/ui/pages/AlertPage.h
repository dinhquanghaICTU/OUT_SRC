#pragma once

#include <QWidget>

namespace Ui { class AlertPage; }

class AlertPage : public QWidget
{
    Q_OBJECT

public:
    explicit AlertPage(QWidget *parent = nullptr);
    ~AlertPage() override;

    void addAlert(const QString &source, const QString &severity, const QString &message);

private:
    Ui::AlertPage *ui;
};
