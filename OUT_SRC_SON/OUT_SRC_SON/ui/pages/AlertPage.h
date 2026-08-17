#pragma once

#include <QWidget>

namespace Ui { class AlertPage; }

class AlertPage : public QWidget
{
    Q_OBJECT

public:
    explicit AlertPage(QWidget *parent = nullptr);
    ~AlertPage() override;

public slots:
    void addAlert(const QString &message, double value);

private:
    Ui::AlertPage *ui;
};
