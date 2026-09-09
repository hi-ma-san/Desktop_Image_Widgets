#ifndef WIDGETCONFIGMANAGER_H
#define WIDGETCONFIGMANAGER_H

#include <QObject>
#include <QList>
#include "../ui/imagesettingsform.h"

class WidgetConfigManager : public QObject {
    Q_OBJECT
public:
    explicit WidgetConfigManager(QObject *parent = nullptr);

    // 儲存與讀取所有掛件設定
    static bool saveWidgets(const QList<ImageWidgetData> &widgetList);
    static QList<ImageWidgetData> loadWidgets();

private:
    static QString getConfigFilePath();
};

#endif // WIDGETCONFIGMANAGER_H
