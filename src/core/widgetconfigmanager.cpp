#include "widgetconfigmanager.h"
#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

WidgetConfigManager::WidgetConfigManager(QObject *parent) : QObject(parent) {}

QString WidgetConfigManager::getConfigFilePath() {
    // 確保設定檔一律生成在 .exe 所在目錄
    return QCoreApplication::applicationDirPath() + "/widgets_config.json";
}

bool WidgetConfigManager::saveWidgets(const QList<ImageWidgetData> &widgetList) {
    QJsonArray array;
    for (const auto &data : widgetList) {
        QJsonObject obj;
        obj["id"] = data.id;
        obj["name"] = data.name;
        obj["relativeImagePath"] = data.relativeImagePath;
        obj["posX"] = data.pos.x();
        obj["posY"] = data.pos.y();
        obj["scale"] = data.scale;
        obj["hoverOpacity"] = data.hoverOpacity;
        obj["windowLevel"] = data.windowLevel;
        obj["draggable"] = data.draggable;
        obj["hoverHide"] = data.hoverHide;
        obj["clickThrough"] = data.clickThrough;
        obj["snap"] = data.snap;
        obj["noFrame"] = data.noFrame;
        array.append(obj);
    }

    QJsonObject root;
    root["widgets"] = array;

    QJsonDocument doc(root);
    QFile file(getConfigFilePath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "無法寫入 widgets_config.json:" << file.errorString();
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

QList<ImageWidgetData> WidgetConfigManager::loadWidgets() {
    QList<ImageWidgetData> list;
    QFile file(getConfigFilePath());
    if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return list;
    }

    QByteArray rawData = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(rawData);
    if (!doc.isObject()) return list;

    QJsonObject root = doc.object();
    QJsonArray array = root["widgets"].toArray();

    for (const auto &val : array) {
        QJsonObject obj = val.toObject();
        ImageWidgetData data;
        data.id = obj["id"].toString();
        data.name = obj["name"].toString();
        data.relativeImagePath = obj["relativeImagePath"].toString();
        data.pos = QPoint(obj["posX"].toInt(150), obj["posY"].toInt(150));
        data.scale = obj["scale"].toInt(100);
        data.hoverOpacity = obj["hoverOpacity"].toInt(20);
        data.windowLevel = obj["windowLevel"].toInt(0);
        data.draggable = obj["draggable"].toBool(true);
        data.hoverHide = obj["hoverHide"].toBool(false);
        data.clickThrough = obj["clickThrough"].toBool(false);
        data.snap = obj["snap"].toBool(true);
        data.noFrame = obj["noFrame"].toBool(false);
        list.append(data);
    }

    return list;
}
