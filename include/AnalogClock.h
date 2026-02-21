#ifndef ANALOGCLOCK_H
#define ANALOGCLOCK_H

#include <QWidget>
#include <QTimer>
#include <QPaintEvent>

class AnalogClock : public QWidget
{
    Q_OBJECT

public:
    explicit AnalogClock(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QTimer *timer;
};

#endif // ANALOGCLOCK_H
