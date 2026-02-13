#include "pixmapdropwidget.h"

#include <QColor>
#include <QPainter>
#include <QMimeData>

PixmapDropWidget::PixmapDropWidget(QWidget *parent) : QWidget(parent) {
    setAcceptDrops(true);
    pixmap = new QPixmap();
    bgBrush = QBrush(QColor("#444"));
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

void PixmapDropWidget::paintEvent(QPaintEvent* event) {
    QWidget::paintEvent(event);
    QPainter p(this);
    p.setPen(Qt::NoPen);
    p.setBrush(bgBrush);
    p.drawRect(rect());
    if (!pixmap->isNull()) {
        p.drawPixmap(QPoint(0, 0), *pixmap, pixmap->rect());
    }
    p.end();
}

void PixmapDropWidget::dragEnterEvent(QDragEnterEvent* event) {
    auto urls = event->mimeData()->urls();
    if (urls.size()) {
        event->setDropAction(Qt::CopyAction);
        event->accept();
    } else {
        event->ignore();
    }
}

void PixmapDropWidget::dropEvent(QDropEvent* event) {
    auto urls = event->mimeData()->urls();
    for (const QUrl& url : urls) {
        if (url.isLocalFile()) {
            event->acceptProposedAction();
            emit dropSignal(url.toLocalFile());
        }
    }
}

bool PixmapDropWidget::load(QString path) {
    pixmap->load(path);
    update();
    return !pixmap->isNull();
}
