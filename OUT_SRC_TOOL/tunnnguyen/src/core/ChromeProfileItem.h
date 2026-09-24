#pragma once

#include <QString>

class ChromeLauncher;

struct ChromeProfileItem {
    QString id;
    QString name;
    QString proxy;
    int port = 9222;
    int windowWidth = 0;
    int windowHeight = 0;
    QString deviceType = "windows";
    QString customUserAgent;
    bool isRunning = false;
    bool isSelected = false;
    ChromeLauncher *launcher = nullptr;
};
