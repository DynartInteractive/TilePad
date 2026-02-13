#ifndef PIXMAPWIDGET_H
#define PIXMAPWIDGET_H

#include <QWidget>
#include <QPixmap>
#include <QBrush>
#include <QPaintEvent>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>

class PixmapDropWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PixmapDropWidget(QWidget *parent = nullptr);
    ~PixmapDropWidget() override;
    bool load(QString path);
    QPixmap* getPixmap();
    void setPixmap(QPixmap* pixmap);
    void setDarkMode(bool dark);
    void setReferenceSize(QSize size);

protected:
    void paintEvent(QPaintEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

signals:
    void dropSignal(QString path);
    void filesDropped(QStringList paths);

private:
    void rebuildCheckerboard();

    QPixmap* pixmap;
    bool m_dragHover = false;
    bool m_darkMode = true;
    QPixmap m_checkerboard;
    QSize m_referenceSize;
};

#endif // PIXMAPWIDGET_H
