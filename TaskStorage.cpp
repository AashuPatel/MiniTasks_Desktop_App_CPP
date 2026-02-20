#include "TaskStorage.h"
#include <fstream>
#include <string>
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
    m_filename = dir.filePath("tasks.txt");
}

std::vector<QString> TaskStorage::load()
{
    std::vector<QString> tasks;
    std::ifstream file(m_filename.toStdString());
    
    if (!file.is_open())
        return tasks;

    std::string line;
    // Pre-allocate to minimize dynamic memory reallocation as requested
    tasks.reserve(64); 

    while (std::getline(file, line))
    {
        if (!line.empty()) {
            tasks.push_back(QString::fromStdString(line));
        }
    }

    return tasks;
}

void TaskStorage::add(const QString& task)
{
    if (task.trimmed().isEmpty()) return;

    std::ofstream file(m_filename.toStdString(), std::ios_base::app);
    if (file.is_open())
    {
        file << task.toStdString() << "\n";
    }
}

void TaskStorage::update(int index, const QString& newText)
{
    if (newText.trimmed().isEmpty()) return;

    auto tasks = load();
    if (index < 0 || static_cast<size_t>(index) >= tasks.size())
        return;

    tasks[index] = newText;

    std::ofstream file(m_filename.toStdString(), std::ios_base::trunc);
    if (file.is_open())
    {
        for (const auto& t : tasks)
        {
            file << t.toStdString() << "\n";
        }
    }
}

void TaskStorage::remove(int index)
{
    auto tasks = load();
    
    if (index < 0 || static_cast<size_t>(index) >= tasks.size())
        return;

    // Remove the target item
    tasks.erase(tasks.begin() + index);

    // Rewrite the entire file deterministically
    std::ofstream file(m_filename.toStdString(), std::ios_base::trunc);
    if (file.is_open())
    {
        for (const auto& t : tasks)
        {
            file << t.toStdString() << "\n";
        }
    }
}
