#ifndef BASECOMPONENT_H
#define BASECOMPONENT_H

#include <QWidget>
#include <QStyleOption>
#include <QPainter>
#include <QTimer>
#include <QCursor>
#include <QPoint>

class BaseComponent : public QWidget {
    Q_OBJECT

public:
    explicit BaseComponent(QWidget *parent = nullptr);
    virtual ~BaseComponent() override = default;

    virtual void initComponent() = 0;
    virtual void initStyle();
    void refreshStyle();

    // 懸停隱藏設定
    void setHoverHide(bool enable, double hoverOpacity = 0.2);

    // 邊緣/元件吸附設定
    void setSnapEnabled(bool enable) { m_snapEnabled = enable; }
    bool isSnapEnabled() const { return m_snapEnabled; }

    // 去除外框設定 (預設 false: 顯示圓角外框)
    void setFrameless(bool enable);
    bool isFrameless() const { return m_frameless; }

protected:
    void paintEvent(QPaintEvent *event) override;
    void performSnap(QPoint &newPos);

private slots:
    void handleHoverCheck();

private:
    bool m_hoverHide = false;
    double m_hoverOpacity = 0.2;
    QTimer *m_hoverCheckTimer = nullptr;

    bool m_snapEnabled = true;
    bool m_frameless = false; // 是否去除外框與底色
};

#endif // BASECOMPONENT_H
