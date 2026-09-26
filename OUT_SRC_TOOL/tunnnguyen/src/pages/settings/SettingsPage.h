#ifndef SETTINGSPAGE_H
#define SETTINGSPAGE_H

#include <QWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>

class SettingsPage : public QWidget {
    Q_OBJECT

public:
    explicit SettingsPage(QWidget *parent = nullptr);
    ~SettingsPage() override = default;

public slots:
    void onBrowseChromePath();
    void onBrowseAdbPath();
    void onSaveSettings();

private:
    void setupUi();

    QLineEdit *m_txtChromePath = nullptr;
    QLineEdit *m_txtAdbPath = nullptr;
    QSpinBox *m_spinTimeout = nullptr;
    QSpinBox *m_spinMaxThreads = nullptr;
    QComboBox *m_themeCombo = nullptr;
    QPushButton *m_btnSave = nullptr;
};

#endif // SETTINGSPAGE_H
