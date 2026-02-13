#include "startupdialog.h"
#include "project.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QFileInfo>

StartupDialog::StartupDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("TilePad");
    setMinimumSize(400, 300);
    setModal(true);
    createLayout();
    loadRecentProjects();
}

StartupDialog::Action StartupDialog::selectedAction() const {
    return m_action;
}

QString StartupDialog::selectedPath() const {
    return m_selectedPath;
}

void StartupDialog::createLayout() {
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(24, 24, 24, 24);

    // Title
    m_titleLabel = new QLabel("TilePad");
    m_titleLabel->setObjectName("startupTitle");
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_titleLabel);

    // Buttons
    auto buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(12);

    m_newButton = new QPushButton("New Project");
    m_newButton->setMinimumHeight(36);
    connect(m_newButton, &QPushButton::clicked, this, &StartupDialog::onNewProject);

    m_openButton = new QPushButton("Open Project");
    m_openButton->setObjectName("secondaryButton");
    m_openButton->setMinimumHeight(36);
    connect(m_openButton, &QPushButton::clicked, this, &StartupDialog::onOpenProject);

    buttonLayout->addWidget(m_newButton);
    buttonLayout->addWidget(m_openButton);
    mainLayout->addLayout(buttonLayout);

    // Recent projects
    m_recentLabel = new QLabel("Recent Projects");
    m_recentLabel->setObjectName("startupRecentLabel");
    QFont recentFont = m_recentLabel->font();
    recentFont.setPointSize(10);
    m_recentLabel->setFont(recentFont);
    mainLayout->addWidget(m_recentLabel);

    m_recentList = new QListWidget();
    m_recentList->setObjectName("startupRecentList");
    connect(m_recentList, &QListWidget::itemDoubleClicked, this, &StartupDialog::onRecentDoubleClicked);
    mainLayout->addWidget(m_recentList, 1);
}

void StartupDialog::loadRecentProjects() {
    QStringList recent = Project::recentProjects();
    if (recent.isEmpty()) {
        m_recentLabel->setVisible(false);
        m_recentList->setVisible(false);
        return;
    }
    for (const QString& path : recent) {
        if (QFileInfo::exists(path)) {
            QFileInfo info(path);
            auto item = new QListWidgetItem(info.fileName());
            item->setToolTip(path);
            item->setData(Qt::UserRole, path);
            m_recentList->addItem(item);
        }
    }
    if (m_recentList->count() == 0) {
        m_recentLabel->setVisible(false);
        m_recentList->setVisible(false);
    }
}

void StartupDialog::onNewProject() {
    m_action = Action::NewProject;
    accept();
}

void StartupDialog::onOpenProject() {
    QString path = QFileDialog::getOpenFileName(this, "Open Project", QString(),
                                                 "TilePad Projects (*.tilepad)");
    if (!path.isEmpty()) {
        m_action = Action::OpenProject;
        m_selectedPath = path;
        accept();
    }
}

void StartupDialog::onRecentDoubleClicked(QListWidgetItem* item) {
    m_action = Action::OpenRecent;
    m_selectedPath = item->data(Qt::UserRole).toString();
    accept();
}
