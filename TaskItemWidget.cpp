#include "TaskItemWidget.h"
#include <QHBoxLayout>
#include <QMouseEvent>

TaskItemWidget::TaskItemWidget(const QString& text, int index, bool isCompleted, QWidget *parent)
    : QWidget(parent), m_index(index), m_isCompleted(isCompleted)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setObjectName("TaskItem");
    
    // Dim the entire item if it is completed
    QString baseAlpha = isCompleted ? "0.02" : "0.05";
    QString hoverAlpha = isCompleted ? "0.05" : "0.15";
    QString borderAlpha = isCompleted ? "0.3" : "0.72";
    QString hoverBorderAlpha = isCompleted ? "0.4" : "0.82";
    QString textStyle = isCompleted ? "color: rgba(255, 255, 255, 0.4); text-decoration: line-through;" : "color: white;";

    setStyleSheet(QString(R"(
        #TaskItem {
            background: rgba(255, 255, 255, %1);
            border: 1px solid rgba(255, 255, 255, %2);
            border-radius: 8px;
            margin: 2px 4px;
        }
        #TaskItem:hover {
            background: rgba(255, 255, 255, %3);
            border: 1px solid rgba(255, 255, 255, %4);
            margin: 0px 2px;
        }
    )").arg(baseAlpha, borderAlpha, hoverAlpha, hoverBorderAlpha));

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 4, 8, 4);
    layout->setSpacing(8);

    m_fullText = text;
    QString displayText = text;
    // Allow up to roughly 3 lines of text (~90 characters) before truncating
    if (displayText.length() > 90) {
        displayText = displayText.left(86) + "...";
    }

    m_label = new QLabel(displayText, this);
    m_label->setStyleSheet(QString("background: transparent; padding: 4px; font-size: 14px; %1").arg(textStyle));
    m_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_label->setWordWrap(true);
    m_label->setAttribute(Qt::WA_TransparentForMouseEvents);

    m_doneBtn = new QPushButton("✓", this);
    m_doneBtn->setFixedSize(24, 24);
    m_doneBtn->setStyleSheet(
        "QPushButton { color: rgba(255, 255, 255, 0.6); background: transparent; border: none; font-size: 16px; font-weight: bold; }"
        "QPushButton:hover { color: rgba(100, 255, 100, 1); }"
    );
    m_doneBtn->setCursor(Qt::PointingHandCursor);
    m_doneBtn->hide(); // Hidden by default

    m_deleteBtn = new QPushButton("✕", this);
    m_deleteBtn->setFixedSize(24, 24);
    // Minimalistic styling for the delete button
    m_deleteBtn->setStyleSheet(
        "QPushButton { color: rgba(255, 255, 255, 0.6); background: transparent; border: none; font-size: 16px; font-weight: bold; }"
        "QPushButton:hover { color: rgba(255, 100, 100, 1); }"
    );
    m_deleteBtn->setCursor(Qt::PointingHandCursor);
    m_deleteBtn->hide(); // Hidden by default

    layout->addWidget(m_label);
    layout->addWidget(m_doneBtn);
    layout->addWidget(m_deleteBtn);

    connect(m_doneBtn, &QPushButton::clicked, this, [this]() {
        emit doneRequested(m_index, !m_isCompleted); // Toggle
    });

    connect(m_deleteBtn, &QPushButton::clicked, this, [this]() {
        emit deleteRequested(m_index);
    });
}

void TaskItemWidget::enterEvent(QEnterEvent *event)
{
    QWidget::enterEvent(event);
    m_doneBtn->show();
    m_deleteBtn->show();
}

void TaskItemWidget::leaveEvent(QEvent *event)
{
    QWidget::leaveEvent(event);
    m_doneBtn->hide();
    m_deleteBtn->hide();
}

void TaskItemWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit editRequested(m_index, m_fullText);
    }
    QWidget::mousePressEvent(event);
}
