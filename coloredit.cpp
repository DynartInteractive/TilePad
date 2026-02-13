#include "coloredit.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QColorDialog>

ColorEdit::ColorEdit(QWidget *parent) : QWidget(parent) {
    color = QColor::fromString("#FF00FF");

    edit = new QLineEdit();
    edit->setText("FF00FF");
    edit->setMaximumWidth(70);
    connect(edit, &QLineEdit::textChanged, this, &ColorEdit::textChanged);
    connect(edit, &QLineEdit::returnPressed, this, &ColorEdit::returnPressed);

    box = new QPushButton();
    box->setFlat(true);
    box->setFixedSize(18, 18);
    box->setStyleSheet("background-color: #FF00FF; border: 1px solid black;");
    box->setCursor(Qt::PointingHandCursor);
    connect(box, &QPushButton::clicked, this, &ColorEdit::boxClicked);

    auto h = new QHBoxLayout();
    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(0);
    h->addWidget(new QLabel("#"));
    h->addWidget(edit);
    h->addSpacing(5);
    h->addWidget(box);
    h->addStretch();

    setLayout(h);
}

void ColorEdit::boxClicked() {
    QColorDialog dialog;
    if (dialog.exec()) {
        setColorText(dialog.selectedColor().name());
    }
}

void ColorEdit::textChanged(QString text) {
    setColorText(text);
}

void ColorEdit::returnPressed() {
    setColorText(edit->text());
}

void ColorEdit::setColorText(QString value) {
    QString text = value.toUpper().replace('#', "");
    QColor newColor;
    newColor = QColor::fromString("#" + text);
    if (!newColor.isValid()) {
        return;
    }
    box->setStyleSheet("background-color: #" + text + "; border: 1px solid black;");
    edit->setText(text);
    color = newColor;
}

QColor ColorEdit::getColor() {
    return color;
}
