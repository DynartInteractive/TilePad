#include "project.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QSettings>

Project::Project() {
}

bool Project::save(const QString& path) {
    QJsonObject root;
    root["version"] = 1;

    QJsonObject settingsObj;
    settingsObj["tileWidth"] = m_settings.tileWidth;
    settingsObj["tileHeight"] = m_settings.tileHeight;
    settingsObj["padding"] = m_settings.padding;
    settingsObj["forcePot"] = m_settings.forcePot;
    settingsObj["reorder"] = m_settings.reorder;
    settingsObj["removePadding"] = m_settings.removePadding;
    settingsObj["transparent"] = m_settings.transparent;
    settingsObj["backgroundColor"] = m_settings.backgroundColor;
    settingsObj["watchFile"] = m_settings.watchFile;
    settingsObj["exportDirectory"] = m_settings.exportDirectory;
    root["settings"] = settingsObj;

    QJsonArray filesArray;
    for (const auto& file : m_files) {
        QJsonObject fileObj;
        fileObj["sourcePath"] = file.sourcePath;
        fileObj["exportPath"] = file.exportPath;
        filesArray.append(fileObj);
    }
    root["files"] = filesArray;

    QJsonDocument doc(root);
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly)) {
        return false;
    }
    f.write(doc.toJson(QJsonDocument::Indented));
    f.close();

    m_projectPath = path;
    m_modified = false;
    addRecentProject(path);
    return true;
}

bool Project::load(const QString& path) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        return false;
    }
    QByteArray data = f.readAll();
    f.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        return false;
    }

    QJsonObject root = doc.object();

    QJsonObject settingsObj = root["settings"].toObject();
    m_settings.tileWidth = settingsObj["tileWidth"].toInt(16);
    m_settings.tileHeight = settingsObj["tileHeight"].toInt(16);
    m_settings.padding = settingsObj["padding"].toInt(1);
    m_settings.forcePot = settingsObj["forcePot"].toBool(true);
    m_settings.reorder = settingsObj["reorder"].toBool(false);
    m_settings.removePadding = settingsObj["removePadding"].toBool(false);
    m_settings.transparent = settingsObj["transparent"].toBool(true);
    m_settings.backgroundColor = settingsObj["backgroundColor"].toString("#FF00FF");
    m_settings.watchFile = settingsObj["watchFile"].toBool(false);
    m_settings.exportDirectory = settingsObj["exportDirectory"].toString();

    m_files.clear();
    QJsonArray filesArray = root["files"].toArray();
    for (const auto& val : filesArray) {
        QJsonObject fileObj = val.toObject();
        FileEntry entry;
        entry.sourcePath = fileObj["sourcePath"].toString();
        entry.exportPath = fileObj["exportPath"].toString();
        m_files.append(entry);
    }

    m_projectPath = path;
    m_modified = false;
    addRecentProject(path);
    return true;
}

int Project::addFile(const QString& sourcePath) {
    FileEntry entry;
    entry.sourcePath = sourcePath;
    if (!m_settings.exportDirectory.isEmpty()) {
        QFileInfo info(sourcePath);
        entry.exportPath = m_settings.exportDirectory + "/" + info.completeBaseName() + ".export." + info.suffix();
    } else {
        entry.exportPath = defaultExportPath(sourcePath);
    }
    m_files.append(entry);
    m_modified = true;
    return m_files.size() - 1;
}

void Project::removeFile(int index) {
    if (index >= 0 && index < m_files.size()) {
        m_files.removeAt(index);
        m_modified = true;
    }
}

int Project::fileCount() const {
    return m_files.size();
}

FileEntry& Project::fileAt(int index) {
    return m_files[index];
}

const FileEntry& Project::fileAt(int index) const {
    return m_files[index];
}

QString Project::projectPath() const {
    return m_projectPath;
}

void Project::setProjectPath(const QString& path) {
    m_projectPath = path;
}

ProjectSettings& Project::settings() {
    return m_settings;
}

const ProjectSettings& Project::settings() const {
    return m_settings;
}

bool Project::isModified() const {
    return m_modified;
}

void Project::setModified(bool modified) {
    m_modified = modified;
}

void Project::markAllDirty() {
    for (auto& file : m_files) {
        if (file.processed) {
            file.dirty = true;
        }
    }
}

void Project::clear() {
    m_files.clear();
    m_settings = ProjectSettings();
    m_projectPath.clear();
    m_modified = false;
}

QString Project::defaultExportPath(const QString& sourcePath) {
    QFileInfo info(sourcePath);
    return info.absoluteDir().path() + "/" + info.completeBaseName() + ".export." + info.suffix();
}

QStringList Project::recentProjects() {
    QSettings settings;
    return settings.value("recentProjects").toStringList();
}

void Project::addRecentProject(const QString& path) {
    QSettings settings;
    QStringList recent = settings.value("recentProjects").toStringList();
    recent.removeAll(path);
    recent.prepend(path);
    while (recent.size() > 10) {
        recent.removeLast();
    }
    settings.setValue("recentProjects", recent);
}
