#include "AnalogClock.h"
#include <QPainter>
#include <QTime>
#include <cmath>

AnalogClock::AnalogClock(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(50, 50); // Small, ultra-minimal size to fit inside the 60px wide SidePanel
    
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        update(); // Trigger a repaint every second
    });
    timer->start(1000);
    
    setAttribute(Qt::WA_TranslucentBackground);
}

void AnalogClock::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    int side = qMin(width(), height());
    painter.translate(width() / 2, height() / 2);
    painter.scale(side / 100.0, side / 100.0);
    
    QTime time = QTime::currentTime();
    
    // Draw minimalist outer ring
    painter.setPen(QPen(QColor(255, 255, 255, 40), 2));
    painter.drawEllipse(-45, -45, 90, 90);
    
    // Draw hour hand
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255, 200));
    painter.save();
    painter.rotate(30.0 * ((time.hour() + time.minute() / 60.0)));
    painter.drawRoundedRect(-2, -25, 4, 25, 2, 2);
    painter.restore();
    
    // Draw minute hand
    painter.setBrush(QColor(165, 180, 252, 200)); // Indigo-300 tint for minutes
    painter.save();
    painter.rotate(6.0 * (time.minute() + time.second() / 60.0));
    painter.drawRoundedRect(-1.5, -35, 3, 35, 1.5, 1.5);
    painter.restore();
    
    // Draw second hand
    painter.setBrush(QColor(135, 206, 235, 255)); // SkyBlue for seconds to match the selection flash
    painter.save();
    painter.rotate(6.0 * time.second());
    painter.drawRect(-0.5, -40, 1, 40);
    painter.restore();
    
    // Center dot
    painter.setBrush(QColor(255, 255, 255, 255));
    painter.drawEllipse(-2, -2, 4, 4);
}
