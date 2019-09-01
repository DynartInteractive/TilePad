#ifndef PADDINGREMOVER_H
#define PADDINGREMOVER_H

#include <QImage>

class PaddingRemover
{
public:
    PaddingRemover();
    ~PaddingRemover();

    void setTileSize(int width, int height);
    void setPadding(int value);
    QImage* create(QImage* source);

private:
    int tileWidth;
    int tileHeight;
    int padding;

    QImage* target = nullptr;
};

#endif // PADDINGREMOVER_H
