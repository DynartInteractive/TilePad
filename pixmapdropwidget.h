#ifndef PIXMAPWIDGET_H
#define PIXMAPWIDGET_H

#include <QWidget>
#include <QPixmap>
#include <QBrush>
#include <QPaintEvent>
#include <QDragEnterEvent>
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

protected:
    void paintEvent(QPaintEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

signals:
    void dropSignal(QString path);

public slots:

private:
    QPixmap* pixmap;
    QBrush bgBrush;
};

#endif // PIXMAPWIDGET_H
