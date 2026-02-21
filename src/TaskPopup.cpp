#include "TaskPopup.h"
#include <QKeyEvent>
#include <QApplication>
#include <QPainter>
#include <QStyleOption>
#include "TaskItemWidget.h"
#include <QListWidgetItem>
#include <QEnterEvent>
#include "TaskEditModal.h"
#include <QFont>
#include <QFontMetrics>
#include <QDateTime>
#include <QTimer>

TaskPopup::TaskPopup(QWidget *parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    setFixedSize(300, 400);

    setObjectName("TaskPopupWindow");
    setStyleSheet(R"(
        #TaskPopupWindow {
            background: rgba(0, 0, 0, 0.65);
            border-radius: 12px;
            border: 1px solid rgba(255, 255, 255, 0.75);
        }
        QLineEdit {
            background: rgba(255, 255, 255, 0.08);
            border: 1px solid rgba(255, 255, 255, 0.77);
            border-radius: 6px;
            padding: 8px;
            color: white;
            font-size: 14px;
        }
        QListWidget {
            background: transparent;
            border: none;
            outline: none;
        }
        QListWidget::item {
            background: transparent;
            border: none;
        }
        QListWidget::item:hover {
            background: transparent;
            border: none;
        }
        QListWidget::item:selected {
            background: transparent;
            border: none;
        }
    )");

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(8);

    m_inputField = new QLineEdit(this);
    m_inputField->setPlaceholderText("Enter a new task...");
    
    m_taskList = new QListWidget(this);
    m_taskList->setFocusPolicy(Qt::NoFocus);
    m_taskList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_taskList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    layout->addWidget(m_inputField);
    layout->addWidget(m_taskList);

    connect(m_inputField, &QLineEdit::returnPressed, this, &TaskPopup::onReturnPressed);
}

void TaskPopup::reloadTasks(const std::vector<TaskItem>& tasks)
{
    m_taskList->clear();
    int index = 0;
    
    QFont font;
    font.setPixelSize(14);
    QFontMetrics fm(font);
    
    qint64 now = QDateTime::currentMSecsSinceEpoch();

    // Create a mutable copy to sort
    std::vector<TaskItem> sortedTasks = tasks;
    
    // Stable sort: Urgent (uncompleted + expired alarm) > Uncompleted > Completed
    std::stable_sort(sortedTasks.begin(), sortedTasks.end(), [now](const TaskItem& a, const TaskItem& b) {
        bool aUrgent = !a.isCompleted && a.alarmTime > 0 && now >= a.alarmTime;
        bool bUrgent = !b.isCompleted && b.alarmTime > 0 && now >= b.alarmTime;
        
        if (aUrgent && !bUrgent) return true;
        if (!aUrgent && bUrgent) return false;
        if (!a.isCompleted && b.isCompleted) return true;
        if (a.isCompleted && !b.isCompleted) return false;
        return false; // Preserve original order otherwise
    });

    for (const auto& task : sortedTasks)
    {
        bool isUrgent = !task.isCompleted && task.alarmTime > 0 && now >= task.alarmTime;
        auto* item = new QListWidgetItem(m_taskList);
        
        QString displayText = task.text;
        if (displayText.length() > 90) {
            displayText = displayText.left(86) + "...";
        }
        
        // Physically simulate word-wrapping across the 188px label bounds
        int textH = fm.boundingRect(0, 0, 188, 0, Qt::TextWordWrap, displayText).height();
        
        int computedHeight = 38; // 1 line default
        if (textH > 40) {
            computedHeight = 84; // 3+ lines mapped
        } else if (textH > 20) {
            computedHeight = 60; // 2 lines mapped
        }
        item->setSizeHint(QSize(0, computedHeight));
        
        m_taskList->addItem(item);

        // Map visual index back to the real storage index by finding the text match
        // Since text is the only unique identifier we have in tasks.json right now
        int realIndex = -1;
        for (size_t i = 0; i < tasks.size(); ++i) {
            if (tasks[i].text == task.text && tasks[i].isCompleted == task.isCompleted && tasks[i].alarmTime == task.alarmTime) {
                realIndex = static_cast<int>(i);
                break;
            }
        }

        auto* widget = new TaskItemWidget(task.text, realIndex, task.isCompleted, isUrgent, this);
        connect(widget, &TaskItemWidget::deleteRequested, this, &TaskPopup::taskDeleted);
        connect(widget, &TaskItemWidget::doneRequested, this, &TaskPopup::taskDone);
        connect(widget, &TaskItemWidget::snoozeRequested, this, &TaskPopup::taskSnoozed);
        connect(widget, &TaskItemWidget::editRequested, this, &TaskPopup::onTaskEditRequested);
        m_taskList->setItemWidget(item, widget);
        
        index++;
    }
}

void TaskPopup::scrollToTask(int targetIndex)
{
    // We must find the visual item representing the real tasks.json index
    for (int i = 0; i < m_taskList->count(); ++i) {
        QListWidgetItem* item = m_taskList->item(i);
        auto* widget = qobject_cast<TaskItemWidget*>(m_taskList->itemWidget(item));
        
        if (widget && widget->getIndex() == targetIndex) {
            m_taskList->scrollToItem(item, QAbstractItemView::PositionAtCenter);
            
            // Trigger 1-second SkyBlue flash
            QString originalStyle = widget->styleSheet();
            widget->setStyleSheet(originalStyle + R"(
                #TaskItem { border: 2px solid rgba(135, 206, 235, 1); }
            )");
            
            QTimer::singleShot(1000, widget, [widget, originalStyle]() {
                if (widget) {
                    widget->setStyleSheet(originalStyle);
                }
            });
            break;
        }
    }
}

void TaskPopup::onReturnPressed()
{
    QString text = m_inputField->text().trimmed();
    if (!text.isEmpty()) {
        emit taskAdded(text);
        m_inputField->clear();
    }
}

void TaskPopup::onTaskEditRequested(int index, const QString& text)
{
    auto* modal = new TaskEditModal(text, this);
    
    // Center relative to TaskPopup
    int mx = rect().center().x() - modal->width() / 2;
    int my = rect().center().y() - modal->height() / 2;
    modal->move(mapToGlobal(QPoint(mx, my)));
    
    connect(modal, &TaskEditModal::saveRequested, this, [this, index, modal](const QString& newText) {
        emit taskEdited(index, newText);
        modal->deleteLater();
    });
    connect(modal, &TaskEditModal::cancelRequested, modal, &QObject::deleteLater);
    
    modal->show();
}

void TaskPopup::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        hide();
    } else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        // Explicitly forward Enter to the input field if it has focus to avoid key swallow
        if (m_inputField->hasFocus()) {
            m_inputField->event(event);
        }
    } else {
        QWidget::keyPressEvent(event);
    }
}

void TaskPopup::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void TaskPopup::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
    emit popupHidden();
}

bool TaskPopup::event(QEvent *e)
{
    if (e->type() == QEvent::WindowDeactivate) {
        QWidget* active = QApplication::activeWindow();
        if (!active || (active != this && !this->isAncestorOf(active) && active->objectName() != "SidePanelWindow")) {
            hide();
        }
    }
    return QWidget::event(e);
}
