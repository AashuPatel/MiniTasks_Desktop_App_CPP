#ifndef FLOATINGBUTTON_H
#define FLOATINGBUTTON_H

#include <QWidget>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QSvgWidget>
#include "TaskPopup.h"
#include "TaskStorage.h"
#include "SidePanel.h"

class FloatingButton : public QWidget
{
    Q_OBJECT

public:
    explicit FloatingButton(QWidget *parent = nullptr);
    ~FloatingButton();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void handleTaskAdded(const QString& task);
    void handleTaskDeleted(int index);
    void handleTaskDone(int index, bool completed);
    void handleTaskEdited(int index, const QString& newText);

private:
    void repositionPopup();
    void togglePopup();

    TaskPopup* m_popup;
    SidePanel* m_sidePanel;
    TaskStorage m_storage;
    QSvgWidget* m_svgWidget;
    qint64 m_lastPopupHideTime = 0;
    
    // Dragging state
    bool m_isDragging = false;
    bool m_movedDuringPress = false;
    QPoint m_dragOffset;
    QPoint m_dragStartPosition;
    bool m_suppressToggle = false;
};

#endif // FLOATINGBUTTON_H
