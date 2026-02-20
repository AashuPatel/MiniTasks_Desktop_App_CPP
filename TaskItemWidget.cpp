#include "TaskItemWidget.h"
#include <QHBoxLayout>
#include <QMouseEvent>

TaskItemWidget::TaskItemWidget(const QString& text, int index, QWidget *parent)
    : QWidget(parent), m_index(index)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setObjectName("TaskItem");
    setStyleSheet(R"(
        #TaskItem {
            background: rgba(255, 255, 255, 0.05);
            border: 1px solid rgba(255, 255, 255, 0.72);
            border-radius: 8px;
            margin: 2px 4px; /* Regular margin */
        }
        #TaskItem:hover {
            background: rgba(255, 255, 255, 0.15);
            border: 1px solid rgba(255, 255, 255, 0.82);
            margin: 0px 2px; /* Grow by shrinking margin */
        }
    )");

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 4, 8, 4);
    layout->setSpacing(8);

    m_fullText = text;
    QString displayText = text;
    if (displayText.length() > 35) {
        displayText = displayText.left(32) + "...";
    }

    m_label = new QLabel(displayText, this);
    m_label->setStyleSheet("color: white; background: transparent; padding: 4px; font-size: 14px;");
    m_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_label->setWordWrap(true);
    m_label->setAttribute(Qt::WA_TransparentForMouseEvents);

    m_deleteBtn = new QPushButton("✕", this);
    m_deleteBtn->setFixedSize(24, 24);
    // Minimalistic styling for the delete button
    m_deleteBtn->setStyleSheet(
        "QPushButton { color: rgba(255, 255, 255, 0.6); background: transparent; border: none; font-size: 16px; font-weight: bold; }"
        "QPushButton:hover { color: rgba(255, 255, 255, 1); }"
    );
    m_deleteBtn->setCursor(Qt::PointingHandCursor);
    m_deleteBtn->hide(); // Hidden by default

    layout->addWidget(m_label);
    layout->addWidget(m_deleteBtn);

    connect(m_deleteBtn, &QPushButton::clicked, this, [this]() {
        emit deleteRequested(m_index);
    });
}

void TaskItemWidget::enterEvent(QEnterEvent *event)
{
    QWidget::enterEvent(event);
    m_deleteBtn->show();
}

void TaskItemWidget::leaveEvent(QEvent *event)
{
    QWidget::leaveEvent(event);
    m_deleteBtn->hide();
}

void TaskItemWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit editRequested(m_index, m_fullText);
    }
    QWidget::mousePressEvent(event);
}
