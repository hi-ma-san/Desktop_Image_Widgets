#include "imagecontrolpanel.h"
#include "ui_imagecontrolpanel.h"
#include "imagesettingsform.h"
#include "../core/settingsmanager.h"
#include "../core/widgetconfigmanager.h"
#include "../core/thememanager.h"
#include <QVBoxLayout>
#include <QAction>
#include <QApplication>
#include <QIcon>
#include <QUuid>
#include <QMessageBox>
#include <QDir>
#include <QFile>

ImageControlPanel::ImageControlPanel(QWidget *parent)
    : QWidget(parent), ui(new Ui::ImageControlPanel) {
    ui->setupUi(this);

    // 1. 嵌入右側設定表單
    auto *containerLayout = new QVBoxLayout(ui->settingsContainer_widget);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    m_settingsForm = new ImageSettingsForm(this);
    containerLayout->addWidget(m_settingsForm);
    ui->settingsContainer_widget->setEnabled(false);

    // 2. 綁定 SettingsManager 全域設定
    auto *sm = SettingsManager::instance();
    ui->autoStart_checkBox->setChecked(sm->isAutoStart());
    ui->trayIcon_checkBox->setChecked(sm->isShowTrayIcon());
    ui->globalLockDrag_checkBox->setChecked(sm->isGlobalDragLocked());

    connect(ui->autoStart_checkBox, &QCheckBox::toggled, sm, &SettingsManager::setAutoStart);
    connect(ui->trayIcon_checkBox, &QCheckBox::toggled, sm, &SettingsManager::setShowTrayIcon);
    connect(ui->globalLockDrag_checkBox, &QCheckBox::toggled, sm, &SettingsManager::setGlobalDragLocked);
    connect(ui->clearCache_button, &QPushButton::clicked, this, &ImageControlPanel::onClearCacheClicked);

    connect(sm, &SettingsManager::trayIconSettingChanged, this, [this](bool show) {
        if (m_trayIcon) m_trayIcon->setVisible(show);
    });

    // 3. 綁定掛件操作按鈕
    connect(ui->btnAddWidget, &QPushButton::clicked, this, &ImageControlPanel::onAddWidget);
    connect(ui->btnDeleteWidget, &QPushButton::clicked, this, &ImageControlPanel::onDeleteWidget);
    connect(ui->toolList_widget, &QListWidget::currentRowChanged, this, &ImageControlPanel::onWidgetSelectionChanged);
    connect(m_settingsForm, &ImageSettingsForm::dataChanged, this, &ImageControlPanel::onWidgetDataChanged);

    // 4. 綁定主題操作按鈕
    connect(ui->saveTheme_button, &QPushButton::clicked, this, &ImageControlPanel::onSaveThemeClicked);
    connect(ui->loadTheme_button, &QPushButton::clicked, this, &ImageControlPanel::onLoadThemeClicked);
    connect(ui->deleteTheme_button, &QPushButton::clicked, this, &ImageControlPanel::onDeleteThemeClicked);



    initTrayIcon();
    refreshThemeList();
    loadSavedWidgets();
}

ImageControlPanel::~ImageControlPanel() {
    saveWidgetsToDisk();
    clearAllWidgets();
    delete ui;
}

// 刷新「主題」分頁列表
void ImageControlPanel::refreshThemeList() {
    ui->themeList_widget->clear();
    QStringList themes = ThemeManager::getThemeList();
    ui->themeList_widget->addItems(themes);

    // 高亮顯示上次套用的主題
    QString lastTheme = SettingsManager::instance()->getLastTheme();
    if (!lastTheme.isEmpty()) {
        auto items = ui->themeList_widget->findItems(lastTheme, Qt::MatchExactly);
        if (!items.isEmpty()) {
            ui->themeList_widget->setCurrentItem(items.first());
        }
    }
}

// 徹底清理並銷毀所有桌面懸浮視窗實例
void ImageControlPanel::clearAllWidgets() {
    for (auto *widget : m_widgets) {
        if (widget) {
            widget->hide();
            widget->close();
            delete widget; // 即時銷毀視窗，避免殘留在畫面上
        }
    }
    m_widgets.clear();
    m_widgetDataMap.clear();

    // 阻斷訊號避免清空時觸發額外的選擇變更事件
    ui->toolList_widget->blockSignals(true);
    ui->toolList_widget->clear();
    ui->toolList_widget->blockSignals(false);

    m_currentSelectedId.clear();
    ui->settingsContainer_widget->setEnabled(false);
}

// --- 【儲存新主題】 ---
void ImageControlPanel::onSaveThemeClicked() {
    QString themeName = ui->themeName_lineEdit->text().trimmed();
    if (themeName.isEmpty()) {
        QMessageBox::warning(this, "提示", "請輸入主題名稱！");
        return;
    }

    if (ThemeManager::saveTheme(themeName, m_widgetDataMap.values())) {
        SettingsManager::instance()->setLastTheme(themeName);
        ui->themeName_lineEdit->clear();
        refreshThemeList();
        QMessageBox::information(this, "成功", QString("主題「%1」已成功儲存！").arg(themeName));
    }
}

// --- 【載入並切換主題】 ---
void ImageControlPanel::onLoadThemeClicked() {
    QListWidgetItem *item = ui->themeList_widget->currentItem();
    if (!item) {
        QMessageBox::warning(this, "提示", "請先從清單選擇要載入的主題！");
        return;
    }

    QString themeName = item->text();
    QList<ImageWidgetData> themeWidgets = ThemeManager::loadTheme(themeName);

    // 1. 銷毀當前所有掛件視窗與清單
    clearAllWidgets();

    // 2. 重新建立目標主題中的掛件視窗
    for (const auto &data : themeWidgets) {
        m_widgetDataMap.insert(data.id, data);
        createWidgetInstance(data); // 內部會自動將指標加入 m_widgets

        auto *listItem = new QListWidgetItem(data.name, ui->toolList_widget);
        listItem->setData(Qt::UserRole, data.id);
    }

    // 3. 記錄最後使用主題並同步至磁碟
    SettingsManager::instance()->setLastTheme(themeName);
    saveWidgetsToDisk();

    if (ui->toolList_widget->count() > 0) {
        ui->toolList_widget->setCurrentRow(0);
    }

    QMessageBox::information(this, "成功", QString("已成功套用主題「%1」！").arg(themeName));
}

// --- 【刪除主題】 ---
void ImageControlPanel::onDeleteThemeClicked() {
    QListWidgetItem *item = ui->themeList_widget->currentItem();
    if (!item) return;

    QString themeName = item->text();
    if (QMessageBox::question(this, "確認刪除", QString("確定要刪除主題「%1」嗎？").arg(themeName)) == QMessageBox::Yes) {
        ThemeManager::deleteTheme(themeName);
        refreshThemeList();
    }
}

void ImageControlPanel::loadSavedWidgets() {
    clearAllWidgets();
    QList<ImageWidgetData> savedList = WidgetConfigManager::loadWidgets();

    // 初次安裝/部署執行：若設定檔為空，直接指定資源檔中的 miku.gif
    if (savedList.isEmpty()) {
        ImageWidgetData defaultData;
        defaultData.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        defaultData.name = "miku";
        defaultData.relativeImagePath = ":/config/miku.gif"; // 直接指派資源檔路徑
        defaultData.pos = QPoint(200, 200);
        defaultData.scale = 100;
        defaultData.hoverOpacity = 20;
        defaultData.windowLevel = 0;
        defaultData.draggable = true;
        defaultData.hoverHide = false;
        defaultData.clickThrough = false;
        defaultData.snap = true;
        defaultData.noFrame = false;

        savedList.append(defaultData);
        WidgetConfigManager::saveWidgets(savedList); // 寫入 widgets_config.json
    }

    for (const auto &data : savedList) {
        m_widgetDataMap.insert(data.id, data);
        createWidgetInstance(data);

        auto *item = new QListWidgetItem(data.name, ui->toolList_widget);
        item->setData(Qt::UserRole, data.id);
    }

    if (ui->toolList_widget->count() > 0) {
        ui->toolList_widget->setCurrentRow(0);
    }
}

void ImageControlPanel::saveWidgetsToDisk() {
    WidgetConfigManager::saveWidgets(m_widgetDataMap.values());
}

// 統一在此處建立掛件，並確保將實例登錄進 m_widgets 清單
ImageWidget* ImageControlPanel::createWidgetInstance(const ImageWidgetData &data) {
    // 防呆：若原先已存在同 ID 元件，先銷毀舊的實例
    if (m_widgets.contains(data.id)) {
        ImageWidget *oldWidget = m_widgets.take(data.id);
        oldWidget->close();
        delete oldWidget;
    }

    auto *widget = new ImageWidget(data.id);
    widget->move(data.pos);
    widget->setWindowLevel(data.windowLevel);
    widget->setDraggable(data.draggable);
    widget->setClickThrough(data.clickThrough);
    widget->setSnapEnabled(data.snap);
    widget->setFrameless(data.noFrame);
    widget->setHoverHide(data.hoverHide, data.hoverOpacity / 100.0);
    widget->setScale(data.scale);
    if (!data.relativeImagePath.isEmpty()) {
        widget->setImagePath(data.relativeImagePath);
    }
    widget->show();

    // 關鍵修復：將生成的 ImageWidget* 指標登錄進管理字典
    m_widgets.insert(data.id, widget);

    connect(widget, &ImageWidget::positionChanged, this, &ImageControlPanel::onWidgetDragged);
    return widget;
}

void ImageControlPanel::onAddWidget() {
    QString id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    int widgetIndex = m_widgets.size() + 1;

    ImageWidgetData data;
    data.id = id;
    data.name = QString("圖片掛件 %1").arg(widgetIndex);
    data.pos = QPoint(150 + widgetIndex * 30, 150 + widgetIndex * 30);
    data.scale = 100;
    data.hoverOpacity = 20;
    data.hoverHide = false;
    data.windowLevel = 0;
    data.draggable = true;
    data.clickThrough = false;
    data.snap = true;
    data.noFrame = false;

    m_widgetDataMap.insert(id, data);
    createWidgetInstance(data);

    auto *item = new QListWidgetItem(data.name, ui->toolList_widget);
    item->setData(Qt::UserRole, id);
    ui->toolList_widget->setCurrentItem(item);

    saveWidgetsToDisk();
}

void ImageControlPanel::onDeleteWidget() {
    int currentRow = ui->toolList_widget->currentRow();
    if (currentRow < 0) return;

    QListWidgetItem *item = ui->toolList_widget->item(currentRow);
    QString id = item->data(Qt::UserRole).toString();

    if (m_widgets.contains(id)) {
        ImageWidget *widget = m_widgets.take(id);
        widget->close();
        delete widget;
    }
    m_widgetDataMap.remove(id);

    delete ui->toolList_widget->takeItem(currentRow);

    if (ui->toolList_widget->count() == 0) {
        m_currentSelectedId.clear();
        ui->settingsContainer_widget->setEnabled(false);
    }

    saveWidgetsToDisk();
}

void ImageControlPanel::onWidgetSelectionChanged() {
    QListWidgetItem *item = ui->toolList_widget->currentItem();
    if (!item) {
        ui->settingsContainer_widget->setEnabled(false);
        m_currentSelectedId.clear();
        return;
    }

    m_currentSelectedId = item->data(Qt::UserRole).toString();
    if (m_widgetDataMap.contains(m_currentSelectedId)) {
        ui->settingsContainer_widget->setEnabled(true);
        m_settingsForm->loadData(m_widgetDataMap[m_currentSelectedId]);
    }
}

void ImageControlPanel::onWidgetDataChanged(const ImageWidgetData &data) {
    if (data.id.isEmpty() || !m_widgets.contains(data.id)) return;

    m_widgetDataMap[data.id] = data;

    ImageWidget *widget = m_widgets[data.id];
    widget->move(data.pos);
    widget->setWindowLevel(data.windowLevel);
    widget->setDraggable(data.draggable);
    widget->setClickThrough(data.clickThrough);
    widget->setSnapEnabled(data.snap);
    widget->setFrameless(data.noFrame);
    widget->setHoverHide(data.hoverHide, data.hoverOpacity / 100.0);
    widget->setScale(data.scale);
    widget->setImagePath(data.relativeImagePath);

    QListWidgetItem *item = ui->toolList_widget->currentItem();
    if (item && item->data(Qt::UserRole).toString() == data.id) {
        if (item->text() != data.name) {
            item->setText(data.name);
        }
    }

    saveWidgetsToDisk();
}

void ImageControlPanel::onWidgetDragged(const QString &id, const QPoint &pos) {
    if (!m_widgetDataMap.contains(id)) return;

    m_widgetDataMap[id].pos = pos;

    if (m_currentSelectedId == id) {
        m_settingsForm->loadData(m_widgetDataMap[id]);
    }

    saveWidgetsToDisk();
}

void ImageControlPanel::initTrayIcon() {
    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setIcon(qApp->windowIcon());
    m_trayIcon->setToolTip("桌面圖片掛件管理員");

    m_trayMenu = new QMenu(this);
    auto *showAction = new QAction("顯示主控制台", this);
    connect(showAction, &QAction::triggered, this, [this]() {
        this->showNormal();
        this->activateWindow();
    });

    auto *exitAction = new QAction("完全退出", this);
    connect(exitAction, &QAction::triggered, this, []() {
        qApp->quit();
    });

    m_trayMenu->addAction(showAction);
    m_trayMenu->addSeparator();
    m_trayMenu->addAction(exitAction);
    m_trayIcon->setContextMenu(m_trayMenu);

    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &ImageControlPanel::onTrayIconActivated);

    if (SettingsManager::instance()->isShowTrayIcon()) {
        m_trayIcon->show();
    }
}

void ImageControlPanel::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick) {
        if (isVisible()) hide();
        else {
            showNormal();
            activateWindow();
        }
    }
}

void ImageControlPanel::closeEvent(QCloseEvent *event) {
    saveWidgetsToDisk();
    if (SettingsManager::instance()->isShowTrayIcon() && m_trayIcon && m_trayIcon->isVisible()) {
        hide();
        event->ignore();
    } else {
        event->accept();
    }
}

// 清除快取
void ImageControlPanel::onClearCacheClicked() {
    auto reply = QMessageBox::warning(
        this,
        "清除快取確認",
        "此操作將會徹底刪除：\n"
        "1. 所有複製進本地的自訂圖片\n"
        "2. 所有已儲存的主題檔案\n"
        "3. 目前所有的掛件配置\n\n"
        "確定要重設並回到初始預設初音掛件嗎？",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
        );

    if (reply != QMessageBox::Yes) {
        return;
    }

    // 1. 銷毀當前所有運行中的視窗與清單
    clearAllWidgets();

    // 2. 刪除 widgets_config.json
    QString configPath = QCoreApplication::applicationDirPath() + "/widgets_config.json";
    if (QFile::exists(configPath)) {
        QFile::remove(configPath);
    }

    // 3. 刪除 images/ 資料夾內的所有實體圖片
    QString imgDirPath = QCoreApplication::applicationDirPath() + "/images";
    QDir imgDir(imgDirPath);
    if (imgDir.exists()) {
        imgDir.removeRecursively(); // 遞迴刪除內部所有檔案
    }

    // 4. 刪除 themes/ 資料夾內的所有主題 JSON 檔
    QString themeDirPath = ThemeManager::getThemeDirectory();
    QDir themeDir(themeDirPath);
    if (themeDir.exists()) {
        themeDir.removeRecursively();
    }

    // 5. 清空全域設定紀錄的主題名稱
    SettingsManager::instance()->setLastTheme("");

    // 6. 刷新主題清單 UI (讓它變空)
    refreshThemeList();

    // 7. 重新呼叫 loadSavedWidgets() 重新初始化 miku.gif
    loadSavedWidgets();

    QMessageBox::information(this, "完成", "快取已完全清除，已恢復預設掛件！");
}
