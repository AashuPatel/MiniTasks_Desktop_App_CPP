#ifndef TASKSTORAGE_H
#define TASKSTORAGE_H

#include <QString>
#include <vector>

struct TaskItem {
    QString text;
    bool isCompleted;
};

class TaskStorage
{
public:
    TaskStorage();
    std::vector<TaskItem> load();
    void add(const QString& task);
    void update(int index, const QString& newText);
    void setCompleted(int index, bool completed);
    void remove(int index);

private:
    QString m_filename;
};

#endif // TASKSTORAGE_H
