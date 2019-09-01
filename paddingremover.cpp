#include "paddingremover.h"
#include <QPainter>

PaddingRemover::PaddingRemover() {

}

PaddingRemover::~PaddingRemover() {
    if (target != nullptr) {
        delete target;
    }
}

void PaddingRemover::setTileSize(int width, int height) {
    tileWidth = width;
    tileHeight = height;
}

void PaddingRemover::setPadding(int value) {
    padding = value;
}

QImage* PaddingRemover::create(QImage* source) {
    if (target != nullptr) {
        delete target;
    }
    int gridWidth = tileWidth + padding * 2;
    int gridHeight = tileHeight + padding * 2;
    int cols = source->width() / gridWidth;
    int rows = source->height() / gridHeight;
    int targetWidth = cols * tileWidth;
    int targetHeight = rows * tileHeight;
    int sx;
    int sy = padding;
    target = new QImage(targetWidth, targetHeight, QImage::Format_ARGB32);
    QPainter p(target);
    for (int j = 0; j < rows; j++) {
        sx = padding;
        for (int i = 0; i < cols; i++) {
            p.drawImage(i * tileWidth, j * tileHeight, *source, sx, sy, tileWidth, tileHeight);
            sx += gridWidth;
        }
        sy += gridHeight;
    }
    return target;
}
