#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include <QSettings>
#include <QMutex>

class SettingsManager : public QObject {
    Q_OBJECT
public:
    static SettingsManager* instance();

    // launches automatically at system startup
    bool isAutoStart() const;
    void setAutoStart(bool enable);

    bool isGlobalDragLocked() const { return m_globalDragLocked; }
    void setGlobalDragLocked(bool locked);

    bool isShowTrayIcon() const { return m_showTrayIcon; }
    void setShowTrayIcon(bool show);

    QString getLastTheme() const { return m_lastTheme; }
    void setLastTheme(const QString &themeName);

signals:
    void globalDragLockedChanged(bool locked);
    void trayIconSettingChanged(bool show);

private:
    explicit SettingsManager(QObject *parent = nullptr);
    ~SettingsManager() override = default;

    static SettingsManager *m_instance;
    static QMutex m_mutex;

    QSettings *m_settings = nullptr;
    bool m_globalDragLocked = false;
    bool m_showTrayIcon = true;
    QString m_lastTheme;
};

#endif // SETTINGSMANAGER_H
