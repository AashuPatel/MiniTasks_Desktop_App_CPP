#include "TaskItemWidget.h"
#include <QHBoxLayout>
#include <QMouseEvent>

TaskItemWidget::TaskItemWidget(const QString& text, int index, bool isCompleted, bool isUrgent, QWidget *parent)
    : QWidget(parent), m_index(index), m_isCompleted(isCompleted), m_isUrgent(isUrgent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setObjectName("TaskItem");
    
    // Default glassy styling
    QString bgAlpha = isCompleted ? "0.02" : (isUrgent ? "0.85" : "0.05");
    QString bgHoverAlpha = isCompleted ? "0.05" : (isUrgent ? "0.95" : "0.15");
    QString borderColor = isCompleted ? "rgba(255, 255, 255, 0.3)" : (isUrgent ? "rgba(129, 140, 248, 0.9)" : "rgba(255, 255, 255, 0.72)");
    QString hoverBorderColor = isCompleted ? "rgba(255, 255, 255, 0.4)" : (isUrgent ? "rgba(165, 180, 252, 1.0)" : "rgba(255, 255, 255, 0.82)");
    QString backgroundColor = isUrgent ? QString("rgba(79, 70, 229, %1)").arg(bgAlpha) : QString("rgba(255, 255, 255, %1)").arg(bgAlpha);
    QString backgroundHoverColor = isUrgent ? QString("rgba(79, 70, 229, %1)").arg(bgHoverAlpha) : QString("rgba(255, 255, 255, %1)").arg(bgHoverAlpha);
    
    QString textStyle = isCompleted ? "color: rgba(255, 255, 255, 0.4); text-decoration: line-through;" : "color: white;";
    if (isUrgent && !isCompleted) textStyle += " font-weight: bold;";

    setStyleSheet(QString(R"(
        #TaskItem {
            background: %1;
            border: 1px solid %2;
            border-radius: 8px;
            margin: 2px 4px;
        }
        #TaskItem:hover {
            background: %3;
            border: 1px solid %4;
            margin: 0px 2px;
        }
    )").arg(backgroundColor, borderColor, backgroundHoverColor, hoverBorderColor));

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 4, 8, 4);
    layout->setSpacing(4);

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

    m_snoozeBtn = new QPushButton("zZ", this);
    m_snoozeBtn->setFixedSize(24, 24);
    m_snoozeBtn->setStyleSheet(
        "QPushButton { color: rgba(255, 255, 255, 0.6); background: transparent; border: none; font-size: 14px; font-weight: bold; margin-bottom: 2px; }"
        "QPushButton:hover { color: rgba(165, 180, 252, 1); }"
    );
    m_snoozeBtn->setCursor(Qt::PointingHandCursor);
    m_snoozeBtn->hide(); // Hidden by default

    m_deleteBtn = new QPushButton("✕", this);
    m_deleteBtn->setFixedSize(24, 24);
    // Minimalistic styling for the delete button
    m_deleteBtn->setStyleSheet(
        "QPushButton { color: rgba(255, 255, 255, 0.6); background: transparent; border: none; font-size: 16px; font-weight: bold; }"
        "QPushButton:hover { color: rgba(255, 100, 100, 1); }"
    );
    m_deleteBtn->setCursor(Qt::PointingHandCursor);
    m_deleteBtn->hide(); // Hidden by default

    auto* btnLayout = new QHBoxLayout();
    btnLayout->setContentsMargins(0, 4, 0, 0);
    btnLayout->setSpacing(4);
    if (isUrgent) {
        btnLayout->addWidget(m_snoozeBtn);
    }
    btnLayout->addWidget(m_doneBtn);
    btnLayout->addWidget(m_deleteBtn);

    layout->addWidget(m_label, 1);
    layout->addLayout(btnLayout);
    btnLayout->setAlignment(Qt::AlignRight | Qt::AlignTop);

    connect(m_snoozeBtn, &QPushButton::clicked, this, [this]() {
        emit snoozeRequested(m_index);
    });

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
    if (m_isUrgent) m_snoozeBtn->show();
    m_doneBtn->show();
    m_deleteBtn->show();
}

void TaskItemWidget::leaveEvent(QEvent *event)
{
    QWidget::leaveEvent(event);
    if (m_isUrgent) m_snoozeBtn->hide();
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
