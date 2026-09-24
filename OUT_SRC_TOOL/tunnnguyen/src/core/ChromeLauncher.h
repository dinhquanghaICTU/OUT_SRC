#pragma once

#include <QObject>
#include <QProcess>
#include <QString>

class ChromeLauncher : public QObject {
    Q_OBJECT
public:
    explicit ChromeLauncher(QObject *parent = nullptr);
    ~ChromeLauncher();

    bool launch(int port, const QString &profilePath, const QString &proxy = "",
                int width = 0, int height = 0, int posX = -1, int posY = -1, double scale = 1.0,
                const QString &userAgent = "");
    void stop();

signals:
    void processStarted();
    void processFinished(int exitCode);

private:
    QProcess *m_process = nullptr;
};
