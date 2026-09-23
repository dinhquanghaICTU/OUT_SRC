#include "core/ChromeLauncher.h"
#include <QDebug>

ChromeLauncher::ChromeLauncher(QObject *parent)
    : QObject(parent), m_process(new QProcess(this))
{
    connect(m_process, &QProcess::started, this, &ChromeLauncher::processStarted);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [this](int exitCode, QProcess::ExitStatus) {
                emit processFinished(exitCode);
            });
}

ChromeLauncher::~ChromeLauncher()
{
    stop();
}

bool ChromeLauncher::launch(int port, const QString &profilePath, const QString &proxy)
{
    QString program = "google-chrome";
#ifdef Q_OS_WIN
    program = "C:/Program Files/Google/Chrome/Application/chrome.exe";
#endif

    QStringList arguments;
    arguments << QString("--remote-debugging-port=%1").arg(port);
    if (!profilePath.isEmpty()) {
        arguments << QString("--user-data-dir=%1").arg(profilePath);
    }
    if (!proxy.isEmpty()) {
        arguments << QString("--proxy-server=%1").arg(proxy);
    }
    arguments << "--disable-blink-features=AutomationControlled";
    arguments << "--no-first-run";

    m_process->start(program, arguments);
    return m_process->waitForStarted();
}

void ChromeLauncher::stop()
{
    if (m_process && m_process->state() != QProcess::NotRunning) {
        m_process->terminate();
        if (!m_process->waitForFinished(2000)) {
            m_process->kill();
        }
    }
}
