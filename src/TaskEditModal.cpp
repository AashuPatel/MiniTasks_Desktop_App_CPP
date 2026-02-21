#include "TaskEditModal.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QStyleOption>
#include <QKeyEvent>

TaskEditModal::TaskEditModal(const QString& text, QWidget *parent)
    : QWidget(parent, Qt::Popup | Qt::FramelessWindowHint)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(280, 200);

    setObjectName("TaskEditModal");
    setStyleSheet(R"(
        #TaskEditModal {
            background: rgba(20, 20, 20, 0.95);
            border-radius: 12px;
            border: 1px solid rgba(255, 255, 255, 0.15);
        }
        QTextEdit {
            background: rgba(255, 255, 255, 0.05);
            border: 1px solid rgba(255, 255, 255, 0.1);
            border-radius: 6px;
            padding: 8px;
            color: white;
            font-size: 14px;
        }
        QPushButton {
            background: rgba(255, 255, 255, 0.08);
            border: 1px solid rgba(255, 255, 255, 0.15);
            border-radius: 4px;
            padding: 6px 16px;
            color: white;
            font-weight: bold;
        }
        QPushButton:hover {
            background: rgba(255, 255, 255, 0.15);
        }
        #SaveBtn {
            background: rgba(46, 204, 113, 0.8);
            border: none;
            color: white;
        }
        #SaveBtn:hover {
            background: rgba(39, 174, 96, 1);
        }
    )");

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(12);

    m_textEdit = new QTextEdit(this);
    m_textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_textEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_textEdit->setPlainText(text);
    mainLayout->addWidget(m_textEdit);

    auto* btnLayout = new QHBoxLayout();
    m_cancelBtn = new QPushButton("Cancel", this);
    m_cancelBtn->setCursor(Qt::PointingHandCursor);

    m_saveBtn = new QPushButton("Save", this);
    m_saveBtn->setObjectName("SaveBtn");
    m_saveBtn->setCursor(Qt::PointingHandCursor);

    btnLayout->addStretch();
    btnLayout->addWidget(m_cancelBtn);
    btnLayout->addWidget(m_saveBtn);
    mainLayout->addLayout(btnLayout);

    connect(m_cancelBtn, &QPushButton::clicked, this, &TaskEditModal::cancelRequested);
    connect(m_saveBtn, &QPushButton::clicked, this, [this]() {
        emit saveRequested(m_textEdit->toPlainText().trimmed());
    });
}

void TaskEditModal::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void TaskEditModal::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        emit cancelRequested();
    } else {
        QWidget::keyPressEvent(event);
    }
}
