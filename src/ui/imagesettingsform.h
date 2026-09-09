#ifndef IMAGESETTINGSFORM_H
#define IMAGESETTINGSFORM_H

#include <QWidget>
#include <QPoint>

namespace Ui {
class ImageSettingsForm;
}

// 圖片小工具完整設定資料結構
struct ImageWidgetData {
    QString id;
    QString name;
    QString relativeImagePath;
    QPoint pos;
    int scale = 100;
    int hoverOpacity = 20;
    int windowLevel = 0;
    bool draggable = true;
    bool hoverHide = false;
    bool clickThrough = false;
    bool snap = true;
    bool noFrame = false;
};

class ImageSettingsForm : public QWidget {
    Q_OBJECT

public:
    explicit ImageSettingsForm(QWidget *parent = nullptr);
    ~ImageSettingsForm() override;

    // 載入資料至表單 UI
    void loadData(const ImageWidgetData &data);

    // 取得當前 UI 的資料
    ImageWidgetData currentData() const;

signals:
    // 當使用者在表單進行任何修改時觸發
    void dataChanged(const ImageWidgetData &data);

private slots:
    void onBrowseImage();
    void onHoverHideToggled(bool checked);
    void onFieldChanged();

private:
    Ui::ImageSettingsForm *ui;
    QString m_currentId;
    QString m_relativeImagePath; // 儲存內部相對路徑
    bool m_isUpdatingUI = false;  // 防止程式更新 UI 時重複觸發 onFieldChanged
};

#endif // IMAGESETTINGSFORM_H
