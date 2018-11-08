#ifndef PADDINGGENERATOR_H
#define PADDINGGENERATOR_H

#include <QImage>

class PaddingGenerator
{
public:
    PaddingGenerator();
    ~PaddingGenerator();

    void setTileSize(int width, int height);
    void setPadding(int value);
    void setForcePot(bool value);
    void setTransparent(bool value);
    void setReorder(bool value);
    void setBackgroundColor(QColor value);
    QImage* create(QImage* source);

private:
    int tileWidth;
    int tileHeight;
    int padding;
    bool forcePot;
    bool transparent;
    bool reorder;
    int cols;
    int rows;
    int gridWidth;
    int gridHeight;
    int targetWidth;
    int targetHeight;
    QImage* target;
    QColor backgroundColor;

    void findSizes(QImage* source);
    void createTargetImage();
    void drawTiles(QImage* source);
    void drawEdges();
};

#endif // PADDINGGENERATOR_H
