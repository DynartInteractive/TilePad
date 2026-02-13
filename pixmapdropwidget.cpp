#include "pixmapdropwidget.h"

#include <QColor>
#include <QPainter>
#include <QMimeData>
#include <QFont>

PixmapDropWidget::PixmapDropWidget(QWidget *parent) : QWidget(parent) {
    setAcceptDrops(true);
    pixmap = new QPixmap();
    rebuildCheckerboard();
}

PixmapDropWidget::~PixmapDropWidget() {
    delete pixmap;
}

QPixmap* PixmapDropWidget::getPixmap() {
    return pixmap;
}

void PixmapDropWidget::setPixmap(QPixmap* value) {
    pixmap = value;
}

void PixmapDropWidget::setDarkMode(bool dark) {
    m_darkMode = dark;
    rebuildCheckerboard();
    update();
}

void PixmapDropWidget::setReferenceSize(QSize size) {
    m_referenceSize = size;
    update();
}

void PixmapDropWidget::rebuildCheckerboard() {
    const int cellSize = 8;
    m_checkerboard = QPixmap(cellSize * 2, cellSize * 2);
    QPainter p(&m_checkerboard);
    if (m_darkMode) {
        p.fillRect(0, 0, cellSize, cellSize, QColor("#3a3a3a"));
        p.fillRect(cellSize, 0, cellSize, cellSize, QColor("#2a2a2a"));
        p.fillRect(0, cellSize, cellSize, cellSize, QColor("#2a2a2a"));
        p.fillRect(cellSize, cellSize, cellSize, cellSize, QColor("#3a3a3a"));
    } else {
        p.fillRect(0, 0, cellSize, cellSize, QColor("#e0e0e0"));
        p.fillRect(cellSize, 0, cellSize, cellSize, QColor("#cccccc"));
        p.fillRect(0, cellSize, cellSize, cellSize, QColor("#cccccc"));
        p.fillRect(cellSize, cellSize, cellSize, cellSize, QColor("#e0e0e0"));
    }
    p.end();
}

void PixmapDropWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    // Draw checkerboard background
    if (!m_checkerboard.isNull()) {
        p.drawTiledPixmap(rect(), m_checkerboard);
    } else {
        p.fillRect(rect(), QColor("#333333"));
    }

    if (pixmap && !pixmap->isNull()) {
        if (m_referenceSize.isValid() && !m_referenceSize.isEmpty()) {
            // Same scale for source and result, image center = widget center
            QSize refFit = m_referenceSize.scaled(size(), Qt::KeepAspectRatio);
            double scale = (double)refFit.width() / (double)m_referenceSize.width();
            int drawW = (int)(pixmap->width() * scale);
            int drawH = (int)(pixmap->height() * scale);
            int x = (width() - drawW) / 2;
            int y = (height() - drawH) / 2;
            p.drawPixmap(x, y, drawW, drawH, *pixmap);
        } else {
            QSize scaledSize = pixmap->size().scaled(size(), Qt::KeepAspectRatio);
            int x = (width() - scaledSize.width()) / 2;
            int y = (height() - scaledSize.height()) / 2;
            p.drawPixmap(x, y, scaledSize.width(), scaledSize.height(), *pixmap);
        }
    } else {
        // Draw drop zone indicator
        QColor borderColor = m_darkMode ? QColor("#666666") : QColor("#aaaaaa");
        QPen dashPen(borderColor, 2, Qt::DashLine);
        p.setPen(dashPen);
        p.setBrush(Qt::NoBrush);
        QRect inner = rect().adjusted(20, 20, -20, -20);
        p.drawRoundedRect(inner, 8, 8);

        // Draw "Drop an image here" text
        QColor textColor = m_darkMode ? QColor("#888888") : QColor("#999999");
        p.setPen(textColor);
        QFont font = p.font();
        font.setPointSize(14);
        p.setFont(font);
        p.drawText(inner, Qt::AlignCenter, "Drop an image here");
    }

    // Drag hover overlay
    if (m_dragHover) {
        p.fillRect(rect(), QColor(74, 144, 217, 40));
        QPen hoverPen(QColor("#4a90d9"), 3, Qt::SolidLine);
        p.setPen(hoverPen);
        p.setBrush(Qt::NoBrush);
        p.drawRect(rect().adjusted(1, 1, -1, -1));
    }

    p.end();
}

void PixmapDropWidget::dragEnterEvent(QDragEnterEvent* event) {
    auto urls = event->mimeData()->urls();
    if (urls.size()) {
        event->setDropAction(Qt::CopyAction);
        event->accept();
        m_dragHover = true;
        update();
    } else {
        event->ignore();
    }
}

void PixmapDropWidget::dragLeaveEvent(QDragLeaveEvent* event) {
    m_dragHover = false;
    update();
    QWidget::dragLeaveEvent(event);
}

void PixmapDropWidget::dropEvent(QDropEvent* event) {
    m_dragHover = false;
    update();
    auto urls = event->mimeData()->urls();
    QStringList paths;
    for (const QUrl& url : urls) {
        if (url.isLocalFile()) {
            paths.append(url.toLocalFile());
        }
    }
    if (!paths.isEmpty()) {
        event->acceptProposedAction();
        emit filesDropped(paths);
    }
}

bool PixmapDropWidget::load(QString path) {
    pixmap->load(path);
    update();
    return !pixmap->isNull();
}
