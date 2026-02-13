#include "mainwindow.h"
#include "paddinggenerator.h"
#include "paddingremover.h"

#include <QApplication>
#include <QGuiApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QImage>
#include <QFileInfo>

int runCli(QGuiApplication& app) {
    QCommandLineParser parser;
    parser.setApplicationDescription("TilePad - Tile padding generator/remover");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption inputOption(QStringList() << "i" << "input", "Input image file path.", "file");
    QCommandLineOption outputOption(QStringList() << "o" << "output", "Output image file path.", "file");
    QCommandLineOption tileWidthOption("tile-width", "Tile width in pixels (default: 16).", "pixels", "16");
    QCommandLineOption tileHeightOption("tile-height", "Tile height in pixels (default: 16).", "pixels", "16");
    QCommandLineOption paddingOption(QStringList() << "p" << "padding", "Padding in pixels (default: 1).", "pixels", "1");
    QCommandLineOption forcePotOption("force-pot", "Force power of two output dimensions.");
    QCommandLineOption reorderOption("reorder", "Reorder tiles (used with --force-pot).");
    QCommandLineOption transparentOption("transparent", "Use transparent padding (default).");
    QCommandLineOption bgColorOption("bg-color", "Background color hex (e.g. FF00FF).", "color", "FF00FF");
    QCommandLineOption removeOption("remove", "Remove padding instead of adding it.");

    parser.addOption(inputOption);
    parser.addOption(outputOption);
    parser.addOption(tileWidthOption);
    parser.addOption(tileHeightOption);
    parser.addOption(paddingOption);
    parser.addOption(forcePotOption);
    parser.addOption(reorderOption);
    parser.addOption(transparentOption);
    parser.addOption(bgColorOption);
    parser.addOption(removeOption);

    parser.process(app);

    if (!parser.isSet(inputOption)) {
        fputs("Error: --input is required.\n", stderr);
        parser.showHelp(1);
    }
    if (!parser.isSet(outputOption)) {
        fputs("Error: --output is required.\n", stderr);
        parser.showHelp(1);
    }

    QString inputPath = parser.value(inputOption);
    QString outputPath = parser.value(outputOption);
    int tileWidth = parser.value(tileWidthOption).toInt();
    int tileHeight = parser.value(tileHeightOption).toInt();
    int padding = parser.value(paddingOption).toInt();
    bool forcePot = parser.isSet(forcePotOption);
    bool reorder = parser.isSet(reorderOption);
    bool transparent = parser.isSet(transparentOption) || !parser.isSet(bgColorOption);
    bool remove = parser.isSet(removeOption);

    QImage sourceImage(inputPath);
    if (sourceImage.isNull()) {
        fputs(QString("Error: Could not load image: %1\n").arg(inputPath).toStdString().c_str(), stderr);
        return 1;
    }

    QImage* resultImage;
    if (remove) {
        PaddingRemover remover;
        remover.setTileSize(tileWidth, tileHeight);
        remover.setPadding(padding);
        resultImage = remover.create(&sourceImage);
    } else {
        PaddingGenerator generator;
        generator.setTileSize(tileWidth, tileHeight);
        generator.setPadding(padding);
        generator.setForcePot(forcePot);
        generator.setReorder(reorder);
        generator.setTransparent(transparent);
        QColor bgColor;
        bgColor.setNamedColor("#" + parser.value(bgColorOption));
        generator.setBackgroundColor(bgColor);
        resultImage = generator.create(&sourceImage);
    }

    QFileInfo fileInfo(outputPath);
    QString format = fileInfo.suffix().toUpper();
    if (format == "JPEG") {
        format = "JPG";
    }
    if (format != "PNG" && format != "JPG") {
        format = "PNG";
    }

    if (!resultImage->save(outputPath, format.toStdString().c_str())) {
        fputs(QString("Error: Could not save image: %1\n").arg(outputPath).toStdString().c_str(), stderr);
        return 1;
    }

    fputs(QString("Saved: %1\n").arg(outputPath).toStdString().c_str(), stdout);
    return 0;
}

int main(int argc, char *argv[]) {
    // Check if any CLI arguments (besides the program name) are provided
    bool hasCliArgs = false;
    for (int i = 1; i < argc; i++) {
        QString arg(argv[i]);
        if (arg.startsWith("-")) {
            hasCliArgs = true;
            break;
        }
    }

    if (hasCliArgs) {
        QGuiApplication app(argc, argv);
        app.setApplicationName("TilePad");
        app.setApplicationVersion("0.5.0");
        return runCli(app);
    }

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
