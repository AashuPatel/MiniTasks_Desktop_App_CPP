#include "SidePanel.h"
#include <QPainter>
#include <QStyleOption>

SidePanel::SidePanel(QWidget *parent)
    : QWidget(parent)
{
    // Tool ensures it floats over other windows. 
    // DoesNotAcceptFocus ensures clicking it or showing it doesn't steal focus from TaskPopup (which would auto-close TaskPopup).
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::WindowDoesNotAcceptFocus);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);

    // Height same as task modal (400). Width approx equal to the QLineEdit height in TaskPopup (~40px)
    setFixedSize(40, 400);

    setObjectName("SidePanelWindow");
    setStyleSheet(R"(
        #SidePanelWindow {
            background: rgba(0, 0, 0, 0.65);
            border-radius: 12px;
            border: 1px solid rgba(255, 255, 255, 0.75);
        }
    )");
}

void SidePanel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
