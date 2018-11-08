#ifndef COLOREDIT_H
#define COLOREDIT_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>

class ColorEdit : public QWidget
{
    Q_OBJECT
public:
    explicit ColorEdit(QWidget *parent = nullptr);
    QColor getColor();
    void setColorText(QString text);

signals:

public slots:
    void boxClicked();
    void textChanged(QString text);
    void returnPressed();

private:
    QColor color;
    QLineEdit* edit;
    QPushButton* box;
};

#endif // COLOREDIT_H
