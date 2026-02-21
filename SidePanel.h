#ifndef SIDEPANEL_H
#define SIDEPANEL_H

#include <QWidget>
#include <QPaintEvent>

class SidePanel : public QWidget
{
    Q_OBJECT

public:
    explicit SidePanel(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
};

#endif // SIDEPANEL_H
