#include "thememanager.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

ThemeManager::ThemeManager(QObject *parent) : QObject(parent) {}

QString ThemeManager::getThemeDirectory() {
    QString path = QCoreApplication::applicationDirPath() + "/themes";
    QDir().mkpath(path);
    return path;
}

QStringList ThemeManager::getThemeList() {
    QDir dir(getThemeDirectory());
    QStringList filters;
    filters << "*.json";
    QStringList files = dir.entryList(filters, QDir::Files, QDir::Name);

    QStringList themes;
    for (const QString &file : files) {
        themes << QFileInfo(file).completeBaseName();
    }
    return themes;
}

bool ThemeManager::saveTheme(const QString &themeName, const QList<ImageWidgetData> &widgetList) {
    if (themeName.trimmed().isEmpty()) return false;

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
    root["themeName"] = themeName;
    root["widgets"] = array;

    QString filePath = getThemeDirectory() + "/" + themeName + ".json";
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "儲存主題失敗:" << file.errorString();
        return false;
    }

    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

QList<ImageWidgetData> ThemeManager::loadTheme(const QString &themeName) {
    QList<ImageWidgetData> list;
    QString filePath = getThemeDirectory() + "/" + themeName + ".json";
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "載入主題失敗:" << file.errorString();
        return list;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isObject()) return list;

    QJsonArray array = doc.object()["widgets"].toArray();
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

bool ThemeManager::deleteTheme(const QString &themeName) {
    QString filePath = getThemeDirectory() + "/" + themeName + ".json";
    return QFile::remove(filePath);
}
