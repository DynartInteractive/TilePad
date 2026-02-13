#ifndef STARTUPDIALOG_H
#define STARTUPDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>

class StartupDialog : public QDialog
{
    Q_OBJECT

public:
    enum class Action { NewProject, OpenProject, OpenRecent, Cancelled };

    explicit StartupDialog(QWidget* parent = nullptr);

    Action selectedAction() const;
    QString selectedPath() const;

private slots:
    void onNewProject();
    void onOpenProject();
    void onRecentDoubleClicked(QListWidgetItem* item);

private:
    void createLayout();
    void loadRecentProjects();

    Action m_action = Action::Cancelled;
    QString m_selectedPath;

    QLabel* m_titleLabel;
    QPushButton* m_newButton;
    QPushButton* m_openButton;
    QLabel* m_recentLabel;
    QListWidget* m_recentList;
};

#endif // STARTUPDIALOG_H
