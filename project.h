#ifndef PROJECT_H
#define PROJECT_H

#include <QString>
#include <QList>
#include <QPixmap>
#include <QColor>

struct ProjectSettings {
    int tileWidth = 16;
    int tileHeight = 16;
    int padding = 1;
    bool forcePot = true;
    bool reorder = false;
    bool removePadding = false;
    bool transparent = true;
    QString backgroundColor = "#FF00FF";
    bool watchFile = false;
    QString exportDirectory;
};

struct FileEntry {
    QString sourcePath;
    QString exportPath;
    QPixmap sourcePixmap;
    QPixmap resultPixmap;
    bool dirty = false;
    bool processed = false;
};

class Project
{
public:
    Project();

    bool save(const QString& path);
    bool load(const QString& path);

    int addFile(const QString& sourcePath);
    void removeFile(int index);
    int fileCount() const;
    FileEntry& fileAt(int index);
    const FileEntry& fileAt(int index) const;

    QString projectPath() const;
    void setProjectPath(const QString& path);

    ProjectSettings& settings();
    const ProjectSettings& settings() const;

    bool isModified() const;
    void setModified(bool modified);

    void markAllDirty();
    void clear();

    static QString defaultExportPath(const QString& sourcePath);

    // Recent projects (stored in QSettings)
    static QStringList recentProjects();
    static void addRecentProject(const QString& path);

private:
    QString m_projectPath;
    ProjectSettings m_settings;
    QList<FileEntry> m_files;
    bool m_modified = false;
};

#endif // PROJECT_H
