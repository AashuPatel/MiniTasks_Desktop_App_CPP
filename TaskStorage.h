#ifndef TASKSTORAGE_H
#define TASKSTORAGE_H

#include <QString>
#include <vector>

class TaskStorage
{
public:
    TaskStorage();
    std::vector<QString> load();
    void add(const QString& task);
    void update(int index, const QString& newText);
    void remove(int index);

private:
    QString m_filename;
};

#endif // TASKSTORAGE_H
