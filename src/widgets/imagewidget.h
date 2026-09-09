#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include "../core/basecomponent.h"
#include <QLabel>
#include <QMovie>
#include <QPoint>
#include <QSize>

class ImageWidget : public BaseComponent {
    Q_OBJECT

public:
    explicit ImageWidget(const QString &id, QWidget *parent = nullptr);
    ~ImageWidget() override;

    void initComponent() override;
    void initStyle() override;

    QString getId() const { return m_id; }
    void setImagePath(const QString &relativePath);
    void setScale(int scale);
    void setWindowLevel(int level);
    void setDraggable(bool enable);
    void setClickThrough(bool enable);

signals:
    void positionChanged(const QString &id, const QPoint &pos);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    void loadMedia(const QString &fullPath);
    void updateComponentSize();
    void updateWindowFlags();

    QString m_id;
    QString m_relativePath;
    QLabel *m_displayLabel = nullptr;
    QMovie *m_gifMovie = nullptr;

    QSize m_baseSize;
    int m_scale = 100;
    int m_windowLevel = 0;
    bool m_draggable = true;
    bool m_clickThrough = false;

    QPoint m_dragPosition;
};

#endif // IMAGEWIDGET_H
