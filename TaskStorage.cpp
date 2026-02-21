#include "TaskStorage.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QStandardPaths>
#include <QDir>
#include <QCoreApplication>

TaskStorage::TaskStorage()
{
    QString appDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(appDataDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    m_filename = dir.filePath("tasks.json");
}

void saveInternal(const QString& filename, const std::vector<TaskItem>& tasks)
{
    QJsonArray array;
    for (const auto& t : tasks) {
        QJsonObject obj;
        obj["text"] = t.text;
        obj["isCompleted"] = t.isCompleted;
        array.append(obj);
    }

    QJsonDocument doc(array);
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        file.write(doc.toJson());
        file.close();
    }
}

std::vector<TaskItem> TaskStorage::load()
{
    std::vector<TaskItem> tasks;
    QFile file(m_filename);
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return tasks;

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isArray()) {
        QJsonArray array = doc.array();
        tasks.reserve(array.size());
        for (const QJsonValue& val : array) {
            if (val.isObject()) {
                QJsonObject obj = val.toObject();
                tasks.push_back({
                    obj["text"].toString(),
                    obj["isCompleted"].toBool()
                });
            }
        }
    }

    return tasks;
}

void TaskStorage::add(const QString& task)
{
    if (task.trimmed().isEmpty()) return;

    auto tasks = load();
    tasks.push_back({ task.trimmed(), false });
    saveInternal(m_filename, tasks);
}

void TaskStorage::update(int index, const QString& newText)
{
    if (newText.trimmed().isEmpty()) return;

    auto tasks = load();
    if (index < 0 || static_cast<size_t>(index) >= tasks.size())
        return;

    tasks[index].text = newText;
    saveInternal(m_filename, tasks);
}

void TaskStorage::setCompleted(int index, bool completed)
{
    auto tasks = load();
    if (index < 0 || static_cast<size_t>(index) >= tasks.size())
        return;

    tasks[index].isCompleted = completed;
    saveInternal(m_filename, tasks);
}

void TaskStorage::remove(int index)
{
    auto tasks = load();
    
    if (index < 0 || static_cast<size_t>(index) >= tasks.size())
        return;

    tasks.erase(tasks.begin() + index);
    saveInternal(m_filename, tasks);
}
