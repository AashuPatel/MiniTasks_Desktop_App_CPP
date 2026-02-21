#ifndef TASKPOPUP_H
#define TASKPOPUP_H

#include <QWidget>
#include <QLineEdit>
#include <QListWidget>
#include <QVBoxLayout>
#include <QPaintEvent>

#include "TaskStorage.h"

class TaskPopup : public QWidget
{
    Q_OBJECT

public:
    explicit TaskPopup(QWidget *parent = nullptr);
    void reloadTasks(const std::vector<TaskItem>& tasks);

signals:
    void taskAdded(const QString& task);
    void taskDeleted(int index);
    void taskDone(int index, bool completed);
    void taskEdited(int index, const QString& newText);
    void popupHidden();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private slots:
    void onReturnPressed();
    void onTaskEditRequested(int index, const QString& text);

private:
    QLineEdit* m_inputField;
    QListWidget* m_taskList;
};

#endif // TASKPOPUP_H
