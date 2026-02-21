#ifndef TASKSTORAGE_H
#define TASKSTORAGE_H

#include <QString>
#include <vector>

struct TaskItem {
    QString text;
    bool isCompleted;
    qint64 alarmTime = 0; // Epoch milliseconds, 0 if not an alarm
};

class TaskStorage
{
public:
    TaskStorage();
    std::vector<TaskItem> load();
    void add(const QString& task);
    void update(int index, const QString& newText);
    void setCompleted(int index, bool completed);
    void snooze(int index);
    void remove(int index);

private:
    QString m_filename;
};

#endif // TASKSTORAGE_H
