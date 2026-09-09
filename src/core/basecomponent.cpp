#include "basecomponent.h"
#include <QStyle>
#include <QApplication>
#include <QScreen>

BaseComponent::BaseComponent(QWidget *parent) : QWidget(parent) {
    this->setAttribute(Qt::WA_TranslucentBackground);

    m_hoverCheckTimer = new QTimer(this);
    m_hoverCheckTimer->setInterval(50);
    connect(m_hoverCheckTimer, &QTimer::timeout, this, &BaseComponent::handleHoverCheck);
}

void BaseComponent::initStyle() {
    refreshStyle();
}

void BaseComponent::setFrameless(bool enable) {
    if (m_frameless != enable) {
        m_frameless = enable;
        refreshStyle();
    }
}

void BaseComponent::refreshStyle() {
    if (m_frameless) {
        // 去除外框與底色：完全透明
        this->setStyleSheet(
            "background-color: transparent;"
            "border: none;"
            );
    } else {
        // 預設樣式：半透明深色底 + 圓角 + 外框
        this->setStyleSheet(
            "background-color: rgba(30, 30, 30, 180);"
            "border-radius: 20px;"
            "border: 1px solid rgba(255, 255, 255, 40);"
            );
    }

    this->style()->unpolish(this);
    this->style()->polish(this);
    this->update();
}

void BaseComponent::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void BaseComponent::setHoverHide(bool enable, double hoverOpacity) {
    m_hoverHide = enable;
    m_hoverOpacity = hoverOpacity;

    if (m_hoverHide) {
        m_hoverCheckTimer->start();
    } else {
        m_hoverCheckTimer->stop();
        this->setWindowOpacity(1.0);
    }
}

void BaseComponent::handleHoverCheck() {
    if (!m_hoverHide) return;

    bool isHovering = this->geometry().contains(QCursor::pos());

    if (isHovering) {
        double targetOpacity = qMax(0.01, m_hoverOpacity);
        if (this->windowOpacity() != targetOpacity) {
            this->setWindowOpacity(targetOpacity);
        }
    } else {
        if (this->windowOpacity() != 1.0) {
            this->setWindowOpacity(1.0);
        }
    }
}

void BaseComponent::performSnap(QPoint &newPos) {
    if (!m_snapEnabled) return;

    int snapMargin = 20;
    QScreen *currentScreen = this->screen();
    if (!currentScreen) currentScreen = QApplication::primaryScreen();
    QRect screenRect = currentScreen->availableGeometry();

    if (qAbs(newPos.x() - screenRect.left()) < snapMargin)
        newPos.setX(screenRect.left());
    if (qAbs(newPos.x() + this->width() - screenRect.right()) < snapMargin)
        newPos.setX(screenRect.right() - this->width());
    if (qAbs(newPos.y() - screenRect.top()) < snapMargin)
        newPos.setY(screenRect.top());
    if (qAbs(newPos.y() + this->height() - screenRect.bottom()) < snapMargin)
        newPos.setY(screenRect.bottom() - this->height());

    const auto widgets = QApplication::topLevelWidgets();
    for (QWidget *w : widgets) {
        if (w == this || !w->isVisible() || !w->inherits("BaseComponent")) continue;

        QRect otherRect = w->frameGeometry();

        if (qAbs(newPos.x() - otherRect.left()) < snapMargin)
            newPos.setX(otherRect.left());
        if (qAbs(newPos.x() + this->width() - otherRect.right()) < snapMargin)
            newPos.setX(otherRect.right() - this->width());
        if (qAbs(newPos.x() - otherRect.right()) < snapMargin)
            newPos.setX(otherRect.right());
        if (qAbs(newPos.x() + this->width() - otherRect.left()) < snapMargin)
            newPos.setX(otherRect.left() - this->width());

        if (qAbs(newPos.y() - otherRect.top()) < snapMargin)
            newPos.setY(otherRect.top());
        if (qAbs(newPos.y() + this->height() - otherRect.bottom()) < snapMargin)
            newPos.setY(otherRect.bottom() - this->height());
        if (qAbs(newPos.y() - otherRect.bottom()) < snapMargin)
            newPos.setY(otherRect.bottom());
        if (qAbs(newPos.y() + this->height() - otherRect.top()) < snapMargin)
            newPos.setY(otherRect.top() - this->height());
    }
}
