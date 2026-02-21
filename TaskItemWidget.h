#ifndef TASKITEMWIDGET_H
#define TASKITEMWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>

class TaskItemWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TaskItemWidget(const QString& text, int index, bool isCompleted, QWidget *parent = nullptr);
    int getIndex() const { return m_index; }

signals:
    void deleteRequested(int index);
    void doneRequested(int index, bool isCompleted);
    void editRequested(int index, const QString& text);

protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    int m_index;
    bool m_isCompleted;
    QString m_fullText;
    QLabel *m_label;
    QPushButton *m_doneBtn;
    QPushButton *m_deleteBtn;
};

#endif // TASKITEMWIDGET_H
