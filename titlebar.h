#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QWidget>
#include <QLabel>
#include <QMenuBar>
#include <QPushButton>
#include <QPaintEvent>

class WindowButton : public QPushButton {
    Q_OBJECT
public:
    enum class Type { Minimize, Maximize, Restore, Close };

    WindowButton(Type type, QWidget* parent = nullptr);
    void setType(Type type);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    Type m_type;
};

class TitleBar : public QWidget {
    Q_OBJECT
public:
    explicit TitleBar(QWidget* parent = nullptr);

    QMenuBar* menuBar() const;
    void updateMaximizeButton(bool maximized);

private:
    QLabel* m_titleLabel;
    QMenuBar* m_menuBar;
    WindowButton* m_minimizeBtn;
    WindowButton* m_maximizeBtn;
    WindowButton* m_closeBtn;
};

#endif // TITLEBAR_H
