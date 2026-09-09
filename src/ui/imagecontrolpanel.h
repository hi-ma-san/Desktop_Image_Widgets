#ifndef IMAGECONTROLPANEL_H
#define IMAGECONTROLPANEL_H

#include <QWidget>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QCloseEvent>
#include <QMap>
#include "../widgets/imagewidget.h"
#include "imagesettingsform.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ImageControlPanel; }
QT_END_NAMESPACE

class ImageControlPanel : public QWidget {
    Q_OBJECT

public:
    explicit ImageControlPanel(QWidget *parent = nullptr);
    ~ImageControlPanel() override;

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void onAddWidget();
    void onDeleteWidget();
    void onWidgetSelectionChanged();
    void onWidgetDataChanged(const ImageWidgetData &data);
    void onWidgetDragged(const QString &id, const QPoint &pos);

    // --- 主題切換與管理槽函式 ---
    void onSaveThemeClicked();
    void onLoadThemeClicked();
    void onDeleteThemeClicked();
    void onClearCacheClicked();

private:
    void initTrayIcon();
    void loadSavedWidgets();
    void saveWidgetsToDisk();
    void refreshThemeList();
    void clearAllWidgets();
    ImageWidget* createWidgetInstance(const ImageWidgetData &data);

    Ui::ImageControlPanel *ui;
    ImageSettingsForm *m_settingsForm = nullptr;
    QSystemTrayIcon *m_trayIcon = nullptr;
    QMenu *m_trayMenu = nullptr;

    QMap<QString, ImageWidget*> m_widgets;
    QMap<QString, ImageWidgetData> m_widgetDataMap;
    QString m_currentSelectedId;
};

#endif // IMAGECONTROLPANEL_H
