#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QObject>
#include <QStringList>
#include <QList>
#include "../ui/imagesettingsform.h"

class ThemeManager : public QObject {
    Q_OBJECT
public:
    explicit ThemeManager(QObject *parent = nullptr);

    static QString getThemeDirectory();
    static QStringList getThemeList();
    static bool saveTheme(const QString &themeName, const QList<ImageWidgetData> &widgetList);
    static QList<ImageWidgetData> loadTheme(const QString &themeName);
    static bool deleteTheme(const QString &themeName);
};

#endif // THEMEMANAGER_H
