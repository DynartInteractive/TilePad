#include "paddinggenerator.h"

#include <QPainter>

PaddingGenerator::PaddingGenerator() {
    target = nullptr;
    tileWidth = 16;
    tileHeight = 16;
    padding = 1;
    forcePot = true;
    transparent = true;
    reorder = false;
    cols = 0;
    rows = 0;
    gridWidth = 0;
    gridHeight = 0;
    targetWidth = 0;
    targetHeight = 0;
    backgroundColor = QColor::fromString("#FF00FF");
}

PaddingGenerator::~PaddingGenerator() {
    if (target != nullptr) {
        delete target;
    }
}

void PaddingGenerator::setTileSize(int width, int height) {
    tileWidth = width;
    tileHeight = height;
}

void PaddingGenerator::setPadding(int value) {
    padding = value;
}

void PaddingGenerator::setForcePot(bool value) {
    forcePot = value;
}

void PaddingGenerator::setTransparent(bool value) {
    transparent = value;
}

void PaddingGenerator::setReorder(bool value) {
    reorder = value;
}

void PaddingGenerator::setBackgroundColor(QColor value) {
    backgroundColor = value;
}

QImage* PaddingGenerator::create(QImage* source) {
    findSizes(source);
    createTargetImage();
    drawTiles(source);
    drawEdges();
    return target;
}

void PaddingGenerator::findSizes(QImage* source) {
    cols = source->width() / tileWidth;
    rows = source->height() / tileHeight;
    gridWidth = tileWidth + padding * 2;
    gridHeight = tileHeight + padding * 2;
    targetWidth = cols * gridWidth;
    targetHeight = rows * gridHeight;
    if (!forcePot) {
        return;
    }
    int size = 1;
    bool widthOk = false;
    bool heightOk = false;
    for (int i = 1; i < 31; i++) {
        if (!widthOk && targetWidth < size) {
            targetWidth = size;
            widthOk = true;
        }
        if (!heightOk && targetHeight < size) {
            targetHeight = size;
            heightOk = true;
        }
        if (widthOk && heightOk) {
            break;
        }
        size *= 2;
    }
}

void PaddingGenerator::createTargetImage() {
    if (target != nullptr) {
        delete target;
    }
    target = new QImage(targetWidth, targetHeight, QImage::Format_ARGB32);
    if (transparent) {
        target->fill(Qt::transparent);
    } else {
        target->fill(backgroundColor);
    }
}

void PaddingGenerator::drawTiles(QImage* source) {
    int x = padding;
    int y = padding;
    int sx = 0;
    int sy = 0;
    bool doReorder = forcePot && reorder;
    QPainter p(target);
    for (int j = 0; j < rows; j++) {
        for (int i = 0; i < cols; i++) {
            p.drawImage(x, y, *source, sx, sy, tileWidth, tileHeight);
            x += gridWidth;
            if (doReorder && x >= targetWidth - gridWidth) {
                x = padding;
                y += gridHeight;
            }
            sx += tileWidth;
        }
        if (!doReorder) {
            x = padding;
            y += gridHeight;
        }
        sx = 0;
        sy += tileHeight;
    }
}

void PaddingGenerator::drawEdges() {
    cols = targetWidth / gridWidth;
    rows = targetHeight / gridHeight;
    int y;
    int x;

    for (int offset = 1; offset < padding + 1; offset++) {
        for (int j = 0; j < rows; j++) {
            y = j * gridHeight + padding;
            for (x = 0; x < targetWidth; x++) {
                target->setPixel(x, y - offset, target->pixel(x, y));
            }
            y = j * gridHeight + tileHeight + padding - 1;
            for (x = 0; x < targetWidth; x++) {
                target->setPixel(x, y + offset, target->pixel(x, y));
            }
        }
    }

    for (int offset = 1; offset < padding + 1; offset++) {
        for (int j = 0; j < cols; j++) {
            x = j * gridWidth + padding;
            for (y = 0; y < targetHeight; y++) {
                target->setPixel(x - offset, y, target->pixel(x, y));
            }
            x = j * gridWidth + tileWidth + padding - 1;
            for (y = 0; y < targetHeight; y++) {
                target->setPixel(x + offset, y, target->pixel(x, y));
            }
        }
    }

}
