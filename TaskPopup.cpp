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

TaskPopup::TaskPopup(QWidget *parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
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

    for (const auto& task : tasks)
    {
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

        auto* widget = new TaskItemWidget(task.text, index, task.isCompleted, this);
        connect(widget, &TaskItemWidget::deleteRequested, this, &TaskPopup::taskDeleted);
        connect(widget, &TaskItemWidget::doneRequested, this, &TaskPopup::taskDone);
        connect(widget, &TaskItemWidget::editRequested, this, &TaskPopup::onTaskEditRequested);
        m_taskList->setItemWidget(item, widget);
        
        index++;
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
