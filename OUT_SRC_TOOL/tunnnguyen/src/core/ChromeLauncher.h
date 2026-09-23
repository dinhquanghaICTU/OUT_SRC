#pragma once

#include <QObject>
#include <QProcess>
#include <QString>

class ChromeLauncher : public QObject {
    Q_OBJECT
public:
    explicit ChromeLauncher(QObject *parent = nullptr);
    ~ChromeLauncher();

    bool launch(int port, const QString &profilePath, const QString &proxy = "");
    void stop();

signals:
    void processStarted();
    void processFinished(int exitCode);

private:
    QProcess *m_process = nullptr;
};
