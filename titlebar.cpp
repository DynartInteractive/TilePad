#include "titlebar.h"

#include <QHBoxLayout>
#include <QPainter>
#include <QStyleOption>

// --- WindowButton ---

WindowButton::WindowButton(Type type, QWidget* parent)
    : QPushButton(parent), m_type(type)
{
    setFixedSize(46, 32);
    setFlat(true);
    setCursor(Qt::ArrowCursor);

    switch (type) {
    case Type::Minimize: setObjectName("minimizeButton"); break;
    case Type::Maximize:
    case Type::Restore:  setObjectName("maximizeButton"); break;
    case Type::Close:    setObjectName("closeButton");    break;
    }
}

void WindowButton::setType(Type type) {
    m_type = type;
    update();
}

void WindowButton::paintEvent(QPaintEvent* event) {
    // Draw background via QSS
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    // Draw icon
    p.setRenderHint(QPainter::Antialiasing);

    QColor iconColor = palette().color(QPalette::WindowText);
    // Use secondary text color (lighter) for a subtler look
    if (!underMouse()) {
        iconColor = QColor(iconColor.red(), iconColor.green(), iconColor.blue(), 180);
    }

    QPen pen(iconColor, 1.0);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);

    int cx = width() / 2;
    int cy = height() / 2;
    int s = 5;

    switch (m_type) {
    case Type::Minimize:
        p.drawLine(cx - s, cy, cx + s, cy);
        break;

    case Type::Maximize:
        p.drawRect(cx - s, cy - s, s * 2, s * 2);
        break;

    case Type::Restore: {
        // Back rectangle (upper-right)
        int offset = 2;
        p.drawRect(cx - s + offset, cy - s, s * 2 - offset, s * 2 - offset);
        // Front rectangle (lower-left)
        p.fillRect(cx - s, cy - s + offset, s * 2 - offset, s * 2 - offset,
                    p.background().color().isValid() ? p.background().color() : QColor(0, 0, 0, 0));
        // Need to fill background behind front rect to hide back rect lines
        QColor bg = palette().color(QPalette::Window);
        // Check if we're hovered for the right bg color
        if (underMouse()) {
            // Will be handled by QSS background, just draw over
        }
        p.drawRect(cx - s, cy - s + offset, s * 2 - offset, s * 2 - offset);
        break;
    }

    case Type::Close:
        p.drawLine(cx - s, cy - s, cx + s, cy + s);
        p.drawLine(cx + s, cy - s, cx - s, cy + s);
        break;
    }

    p.end();
}

// --- TitleBar ---

TitleBar::TitleBar(QWidget* parent) : QWidget(parent) {
    setFixedHeight(32);
    setObjectName("titleBar");

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(12, 0, 0, 0);
    layout->setSpacing(0);

    m_titleLabel = new QLabel("TilePad");
    m_titleLabel->setObjectName("titleBarLabel");
    layout->addWidget(m_titleLabel);

    layout->addSpacing(12);

    m_menuBar = new QMenuBar();
    m_menuBar->setObjectName("titleBarMenu");
    m_menuBar->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    layout->addWidget(m_menuBar);

    layout->addStretch();

    m_minimizeBtn = new WindowButton(WindowButton::Type::Minimize);
    m_maximizeBtn = new WindowButton(WindowButton::Type::Maximize);
    m_closeBtn = new WindowButton(WindowButton::Type::Close);

    layout->addWidget(m_minimizeBtn);
    layout->addWidget(m_maximizeBtn);
    layout->addWidget(m_closeBtn);

    connect(m_minimizeBtn, &QPushButton::clicked, this, [this]() {
        window()->showMinimized();
    });
    connect(m_maximizeBtn, &QPushButton::clicked, this, [this]() {
        if (window()->isMaximized()) {
            window()->showNormal();
        } else {
            window()->showMaximized();
        }
    });
    connect(m_closeBtn, &QPushButton::clicked, this, [this]() {
        window()->close();
    });
}

QMenuBar* TitleBar::menuBar() const {
    return m_menuBar;
}

void TitleBar::updateMaximizeButton(bool maximized) {
    m_maximizeBtn->setType(maximized
        ? WindowButton::Type::Restore
        : WindowButton::Type::Maximize);
}
