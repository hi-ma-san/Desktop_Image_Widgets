#include "settingsmanager.h"

#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QCoreApplication>
#include <QMutexLocker>

SettingsManager* SettingsManager::m_instance = nullptr;
QMutex SettingsManager::m_mutex;

SettingsManager::SettingsManager(QObject *parent) : QObject(parent) {
    // 確保設定檔生成在 .exe 同層目錄
    QString configPath = QCoreApplication::applicationDirPath() + "/config.ini";
    m_settings = new QSettings(configPath, QSettings::IniFormat, this);

    // 啟動時自動載入之前存過的數值
    m_globalDragLocked = m_settings->value("Global/DragLocked", false).toBool();
    m_showTrayIcon = m_settings->value("Global/ShowTrayIcon", true).toBool();
    m_lastTheme = m_settings->value("Global/LastTheme", "").toString();
}

SettingsManager* SettingsManager::instance() {
    if (!m_instance) {
        QMutexLocker locker(&m_mutex);
        if (!m_instance) {
            m_instance = new SettingsManager();
        }
    }
    return m_instance;
}

/** --- launches automatically --- **/

bool SettingsManager::isAutoStart() const {
    QString appName = QFileInfo(QCoreApplication::applicationFilePath()).completeBaseName();
    QString startupPath = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation)
                          + "/Startup/" + appName + ".lnk";
    return QFile::exists(startupPath);
}

void SettingsManager::setAutoStart(bool enable) {
    QString startupDir = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation) + "/Startup/";
    QString appName = QFileInfo(QCoreApplication::applicationFilePath()).completeBaseName();
    QString shortcutPath = startupDir + appName + ".lnk";
    QString appPath = QCoreApplication::applicationFilePath();

    if (enable) {
        if (!QFile::exists(shortcutPath)) {
            QFile::link(appPath, shortcutPath);
            qDebug() << "SettingsManager: 已建立啟動捷徑" << shortcutPath;
        }
    } else {
        if (QFile::exists(shortcutPath)) {
            QFile::remove(shortcutPath);
            qDebug() << "SettingsManager: 已移除啟動捷徑";
        }
    }
}

/** --- Global Setting --- **/

void SettingsManager::setGlobalDragLocked(bool locked) {
    if (m_globalDragLocked != locked) {
        m_globalDragLocked = locked;
        m_settings->setValue("Global/DragLocked", locked);
        emit globalDragLockedChanged(locked);
    }
}

void SettingsManager::setShowTrayIcon(bool show) {
    if (m_showTrayIcon != show) {
        m_showTrayIcon = show;
        m_settings->setValue("Global/ShowTrayIcon", show);
        emit trayIconSettingChanged(show);
        qDebug() << "SettingsManager: 托盤圖示狀態更新為 ->" << show;
    }
}

void SettingsManager::setLastTheme(const QString &themeName) {
    if (m_lastTheme != themeName) {
        m_lastTheme = themeName;
        m_settings->setValue("Global/LastTheme", themeName);
        m_settings->sync();
    }
}
