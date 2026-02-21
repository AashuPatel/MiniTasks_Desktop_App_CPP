#ifndef SIDEPANEL_H
#define SIDEPANEL_H

#include <QWidget>
#include <QPaintEvent>
#include <QVBoxLayout>
#include <QListWidget>
#include <QLabel>
#include "TaskStorage.h"

class SidePanel : public QWidget
{
    Q_OBJECT

public:
    explicit SidePanel(QWidget *parent = nullptr);
    void reloadSchedule(const std::vector<TaskItem>& tasks);

signals:
    void scrollTargetRequested(int taskIndex);

protected:
    void paintEvent(QPaintEvent *event) override;
    
private:
    QListWidget* m_scheduleList;
};

#endif // SIDEPANEL_H
