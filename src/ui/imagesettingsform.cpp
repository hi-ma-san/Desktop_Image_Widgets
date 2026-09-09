#include "imagesettingsform.h"
#include "ui_imagesettingsform.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QDir>
#include <QCoreApplication>
#include <QUuid>

ImageSettingsForm::ImageSettingsForm(QWidget *parent)
    : QWidget(parent), ui(new Ui::ImageSettingsForm) {
    ui->setupUi(this);

    // 1. 綁定更換圖片按鈕
    connect(ui->browseImage_button, &QPushButton::clicked, this, &ImageSettingsForm::onBrowseImage);

    // 2. 懸停自動隱藏 CheckBox 與透明度 Slider 啟用狀態連動
    connect(ui->hoverHide_checkBox, &QCheckBox::toggled, this, &ImageSettingsForm::onHoverHideToggled);

    // 3. Slider 滑動時即時更新百分比文字標籤並觸發變更訊號
    connect(ui->scale_slider, &QSlider::valueChanged, this, [this](int val) {
        ui->label_scaleVal->setText(QString("%1%").arg(val));
        onFieldChanged();
    });
    connect(ui->transparency_slider, &QSlider::valueChanged, this, [this](int val) {
        ui->label_opacityVal->setText(QString("%1%").arg(val));
        onFieldChanged();
    });

    // 4. 其餘欄位修改訊號綁定
    connect(ui->widgetName_lineEdit, &QLineEdit::textChanged, this, &ImageSettingsForm::onFieldChanged);
    connect(ui->coordX_lineEdit, &QLineEdit::textChanged, this, &ImageSettingsForm::onFieldChanged);
    connect(ui->coordY_lineEdit, &QLineEdit::textChanged, this, &ImageSettingsForm::onFieldChanged);
    connect(ui->position_comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ImageSettingsForm::onFieldChanged);
    connect(ui->draggable_checkBox, &QCheckBox::toggled, this, &ImageSettingsForm::onFieldChanged);
    connect(ui->clickThrough_checkBox, &QCheckBox::toggled, this, &ImageSettingsForm::onFieldChanged);

    connect(ui->snap_checkBox, &QCheckBox::toggled, this, &ImageSettingsForm::onFieldChanged);
    connect(ui->noFrame_checkBox, &QCheckBox::toggled, this, &ImageSettingsForm::onFieldChanged);
}

ImageSettingsForm::~ImageSettingsForm() {
    delete ui;
}

// 懸停開關切換：控制 Slider 是否可調整
void ImageSettingsForm::onHoverHideToggled(bool checked) {
    ui->transparency_slider->setEnabled(checked);
    onFieldChanged();
}

// 選擇圖片：自動複製到 applicationDirPath()/images/
void ImageSettingsForm::onBrowseImage() {
    QString srcPath = QFileDialog::getOpenFileName(this, "選擇圖片", "", "Images (*.png *.jpg *.jpeg *.bmp *.gif)");
    if (srcPath.isEmpty()) return;

    // 確保本機 images 目錄存在
    QString imgDir = QCoreApplication::applicationDirPath() + "/images";
    QDir().mkpath(imgDir);

    // 以 UUID 建立新檔名複製到專案資料夾
    QFileInfo fileInfo(srcPath);
    QString newFileName = QUuid::createUuid().toString(QUuid::WithoutBraces) + "." + fileInfo.suffix().toLower();
    QString destPath = imgDir + "/" + newFileName;

    // 複製檔案並更新內部相對路徑
    if (QFile::copy(srcPath, destPath)) {
        m_relativeImagePath = "images/" + newFileName;
        onFieldChanged();
    }
}

// 載入掛件資料並刷新 UI
void ImageSettingsForm::loadData(const ImageWidgetData &data) {
    m_isUpdatingUI = true;

    m_currentId = data.id;
    m_relativeImagePath = data.relativeImagePath;

    ui->widgetName_lineEdit->setText(data.name);
    ui->coordX_lineEdit->setText(QString::number(data.pos.x()));
    ui->coordY_lineEdit->setText(QString::number(data.pos.y()));

    // 縮放比例
    ui->scale_slider->setValue(data.scale);
    ui->label_scaleVal->setText(QString("%1%").arg(data.scale));

    // 懸停與透明度
    ui->hoverHide_checkBox->setChecked(data.hoverHide);
    ui->transparency_slider->setEnabled(data.hoverHide);
    ui->transparency_slider->setValue(data.hoverOpacity);
    ui->label_opacityVal->setText(QString("%1%").arg(data.hoverOpacity));

    // 吸附
    ui->snap_checkBox->setChecked(data.snap);

    // 外框
    ui->noFrame_checkBox->setChecked(data.noFrame);

    // 視窗層級與行為
    ui->position_comboBox->setCurrentIndex(data.windowLevel);
    ui->draggable_checkBox->setChecked(data.draggable);
    ui->clickThrough_checkBox->setChecked(data.clickThrough);

    m_isUpdatingUI = false;
}

// 從 UI 收集目前設定
ImageWidgetData ImageSettingsForm::currentData() const {
    ImageWidgetData data;
    data.id = m_currentId;
    data.name = ui->widgetName_lineEdit->text();
    data.relativeImagePath = m_relativeImagePath;
    data.pos = QPoint(ui->coordX_lineEdit->text().toInt(), ui->coordY_lineEdit->text().toInt());
    data.scale = ui->scale_slider->value();
    data.hoverHide = ui->hoverHide_checkBox->isChecked();
    data.hoverOpacity = ui->transparency_slider->value();
    data.windowLevel = ui->position_comboBox->currentIndex();
    data.draggable = ui->draggable_checkBox->isChecked();
    data.clickThrough = ui->clickThrough_checkBox->isChecked();
    data.snap = ui->snap_checkBox->isChecked();
    data.noFrame = ui->noFrame_checkBox->isChecked();
    return data;
}

// 欄位變更通知
void ImageSettingsForm::onFieldChanged() {
    if (m_isUpdatingUI) return;
    emit dataChanged(currentData());
}
