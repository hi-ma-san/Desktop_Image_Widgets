#include "imagewidget.h"
#include "../core/settingsmanager.h"
#include <QVBoxLayout>
#include <QFileInfo>
#include <QMouseEvent>
#include <QCoreApplication>

ImageWidget::ImageWidget(const QString &id, QWidget *parent)
    : BaseComponent(parent), m_id(id) {
    initComponent();
    initStyle();
}

void ImageWidget::initComponent() {
    m_displayLabel = new QLabel(this);
    m_displayLabel->setAlignment(Qt::AlignCenter);
    m_displayLabel->setScaledContents(true);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(15, 15, 15, 15);
    layout->addWidget(m_displayLabel);

    setAttribute(Qt::WA_TranslucentBackground);
    updateWindowFlags();
    resize(200, 200);
}

void ImageWidget::initStyle() {
    BaseComponent::initStyle();
    if (m_displayLabel) {
        m_displayLabel->setStyleSheet("background: transparent; border: none;");
    }
}

void ImageWidget::updateWindowFlags() {
    Qt::WindowFlags flags = Qt::FramelessWindowHint | Qt::SubWindow;
    if (m_windowLevel == 0) flags |= Qt::WindowStaysOnTopHint;
    else if (m_windowLevel == 2) flags |= Qt::WindowStaysOnBottomHint;

    setAttribute(Qt::WA_TransparentForMouseEvents, m_clickThrough);
    setWindowFlags(flags);
    show();
}

void ImageWidget::setImagePath(const QString &relativePath) {
    m_relativePath = relativePath;
    if (relativePath.isEmpty()) return;

    QString fullPath;
    if (relativePath.startsWith(":/")) {
        // 資源檔路徑，直接載入
        fullPath = relativePath;
    } else {
        // 本機 images/ 目錄下的相對路徑，拼接執行檔目錄
        fullPath = QCoreApplication::applicationDirPath() + "/" + relativePath;
    }

    loadMedia(fullPath);
}
void ImageWidget::setScale(int scale) {
    m_scale = scale;
    updateComponentSize();
}

void ImageWidget::setWindowLevel(int level) {
    if (m_windowLevel != level) {
        m_windowLevel = level;
        updateWindowFlags();
    }
}

void ImageWidget::setDraggable(bool enable) {
    m_draggable = enable;
}

void ImageWidget::setClickThrough(bool enable) {
    if (m_clickThrough != enable) {
        m_clickThrough = enable;
        updateWindowFlags();
    }
}

void ImageWidget::loadMedia(const QString &path) {
    if (m_gifMovie) {
        m_gifMovie->stop();
        delete m_gifMovie;
        m_gifMovie = nullptr;
    }

    QFileInfo info(path);
    QString ext = info.suffix().toLower();
    QSize rawSize;

    if (ext == "gif") {
        m_gifMovie = new QMovie(path);
        m_gifMovie->start();
        rawSize = m_gifMovie->currentPixmap().size();
        m_displayLabel->setMovie(m_gifMovie);
    } else {
        m_displayLabel->setMovie(nullptr);
        QPixmap pix(path);
        if (!pix.isNull()) {
            rawSize = pix.size();
            m_displayLabel->setPixmap(pix);
        }
    }

    if (!rawSize.isEmpty()) {
        m_baseSize = rawSize.scaled(400, 400, Qt::KeepAspectRatio);
        updateComponentSize();
    }
}

void ImageWidget::updateComponentSize() {
    if (!m_baseSize.isValid()) return;

    QSize contentSize = m_baseSize * (m_scale / 100.0);
    m_displayLabel->setFixedSize(contentSize);

    QSize windowSize = contentSize + QSize(30, 30);
    setFixedSize(windowSize);

    if (m_gifMovie) {
        m_gifMovie->setScaledSize(contentSize);
    }
}

void ImageWidget::mousePressEvent(QMouseEvent *event) {
    if (SettingsManager::instance()->isGlobalDragLocked() || !m_draggable) {
        event->ignore();
        return;
    }
    if (event->button() == Qt::LeftButton) {
        m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}

void ImageWidget::mouseMoveEvent(QMouseEvent *event) {
    if (SettingsManager::instance()->isGlobalDragLocked() || !m_draggable) {
        event->ignore();
        return;
    }
    if (event->buttons() & Qt::LeftButton) {
        QPoint newPos = event->globalPosition().toPoint() - m_dragPosition;

        // 套用吸附演算法校正 newPos
        performSnap(newPos);

        move(newPos);
        emit positionChanged(m_id, newPos);
        event->accept();
    }
}

ImageWidget::~ImageWidget() {
    if (m_gifMovie) {
        m_gifMovie->stop();
        delete m_gifMovie;
    }
}
