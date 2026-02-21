#include "SidePanel.h"
#include <QPainter>
#include <QStyleOption>
#include <QDateTime>
#include <algorithm>

SidePanel::SidePanel(QWidget *parent)
    : QWidget(parent)
{
    // Tool ensures it floats over other windows. 
    // DoesNotAcceptFocus ensures clicking it or showing it doesn't steal focus from TaskPopup (which would auto-close TaskPopup).
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::WindowDoesNotAcceptFocus);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);

    // Widened from 40 to 60 to fit hours/minutes cleanly
    setFixedSize(60, 400);

    setObjectName("SidePanelWindow");
    setStyleSheet(R"(
        #SidePanelWindow {
            background: rgba(0, 0, 0, 0.65);
            border-radius: 12px;
            border: 1px solid rgba(255, 255, 255, 0.75);
        }
        QListWidget {
            background: transparent;
            border: none;
            outline: none;
        }
        QListWidget::item {
            color: rgba(255, 255, 255, 0.7);
            font-size: 13px;
            font-weight: bold;
            text-align: center;
            padding: 8px 0px;
            border-bottom: 1px solid rgba(255, 255, 255, 0.1);
        }
        QListWidget::item:hover {
            color: white;
            background: rgba(255, 255, 255, 0.1);
            border-radius: 4px;
        }
    )");

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 12, 4, 12);
    layout->setSpacing(0);

    m_scheduleList = new QListWidget(this);
    m_scheduleList->setFocusPolicy(Qt::NoFocus);
    m_scheduleList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scheduleList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    layout->addWidget(m_scheduleList);

    connect(m_scheduleList, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
        bool ok;
        int targetIndex = item->data(Qt::UserRole).toInt(&ok);
        if (ok) {
            emit scrollTargetRequested(targetIndex);
        }
    });
}

void SidePanel::reloadSchedule(const std::vector<TaskItem>& tasks)
{
    m_scheduleList->clear();
    
    // Sort logic requires capturing original indices, so we map them out into pairs
    std::vector<std::pair<int, TaskItem>> upcoming;
    qint64 now = QDateTime::currentMSecsSinceEpoch();

    for (size_t i = 0; i < tasks.size(); ++i) {
        const auto& t = tasks[i];
        if (!t.isCompleted && t.alarmTime > 0 && t.alarmTime >= now) {
            upcoming.push_back({static_cast<int>(i), t});
        }
    }

    // Sort ascending by alarm time (nearest future first)
    std::sort(upcoming.begin(), upcoming.end(), [](const auto& a, const auto& b) {
        return a.second.alarmTime < b.second.alarmTime;
    });

    for (const auto& pair : upcoming) {
        QDateTime dt = QDateTime::fromMSecsSinceEpoch(pair.second.alarmTime);
        QString timeStr = dt.toString("HH:mm");
        
        auto* item = new QListWidgetItem(timeStr, m_scheduleList);
        item->setTextAlignment(Qt::AlignCenter);
        item->setData(Qt::UserRole, pair.first); // Store the original tasks.json index natively
        m_scheduleList->addItem(item);
    }
}

void SidePanel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
