#include "FloatingButton.h"
#include <QApplication>
#include <QScreen>
#include <QDebug>
#include <QPainter>
#include <QStyleOption>
#include <QDateTime>
#include <QSettings>

// Windows API for true DWM blur
#include <windows.h>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

FloatingButton::FloatingButton(QWidget *parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(60, 60); // Updated to 60x60 to match the SVG dimensions better

    m_svgWidget = new QSvgWidget(this);
    m_svgWidget->load(QByteArray(R"V0G0N(
<svg width="120" height="120" viewBox="0 0 120 120" xmlns="http://www.w3.org/2000/svg">
  <defs>
    <style>
      /* Smooth floating animation scaled down for the smaller bubble */
      @keyframes float {
        0%, 100% { transform: translateY(0px); }
        50% { transform: translateY(-5px); }
      }
      
      /* Self-drawing animation adjusted for the shorter path length */
      @keyframes drawTick {
        0% { stroke-dashoffset: 40; opacity: 0; }
        10% { opacity: 1; }
        100% { stroke-dashoffset: 0; opacity: 1; }
      }

      /* Subtle pulsing glow for the tick */
      @keyframes pulse {
        0%, 100% { filter: drop-shadow(0 0 1px #00f7ff) drop-shadow(0 0 2px #00f7ff); }
        50% { filter: drop-shadow(0 0 2px #00f7ff) drop-shadow(0 0 4px #00f7ff); }
      }

      .bubble-group {
        animation: float 4s ease-in-out infinite;
        transform-origin: center;
      }

      .tick-path {
        stroke-dasharray: 40;
        stroke-dashoffset: 40;
        /* Draws in 1s, then pulses indefinitely */
        animation: drawTick 1.2s cubic-bezier(0.25, 1, 0.5, 1) forwards 0.3s,
                   pulse 3s ease-in-out infinite 1.5s;
      }
    </style>

    <radialGradient id="bubble-body" cx="50%" cy="50%" r="50%" fx="40%" fy="40%">
      <stop offset="0%" stop-color="#fff" stop-opacity="0.05"/>
      <stop offset="75%" stop-color="#a3cfff" stop-opacity="0.25"/>
      <stop offset="95%" stop-color="#c98fff" stop-opacity="0.45"/>
      <stop offset="100%" stop-color="#90e0ff" stop-opacity="0.6"/>
    </radialGradient>
    
    <linearGradient id="iridescence" x1="0%" y1="0%" x2="100%" y2="100%">
      <stop offset="20%" stop-color="#00ffff" stop-opacity="0.5">
        <animate attributeName="stop-color" values="#00ffff;#ff00ff;#ffff00;#00ffff" dur="5s" repeatCount="indefinite" />
      </stop>
      <stop offset="50%" stop-color="#ff00ff" stop-opacity="0.4">
        <animate attributeName="stop-color" values="#ff00ff;#ffff00;#00ffff;#ff00ff" dur="5s" repeatCount="indefinite" />
      </stop>
      <stop offset="80%" stop-color="#ffff00" stop-opacity="0.5">
        <animate attributeName="stop-color" values="#ffff00;#00ffff;#ff00ff;#ffff00" dur="5s" repeatCount="indefinite" />
      </stop>
    </linearGradient>

    <linearGradient id="sharp-highlight" x1="0%" y1="0%" x2="0%" y2="100%">
      <stop offset="0%" stop-color="white" stop-opacity="0.95"/>
      <stop offset="100%" stop-color="white" stop-opacity="0"/>
    </linearGradient>
  </defs>

  <g class="bubble-group">
    <circle cx="60" cy="60" r="30" fill="url(#bubble-body)" stroke="url(#iridescence)" stroke-width="1"/>

    <path d="M 45 45 Q 60 33 75 45" stroke="url(#sharp-highlight)" stroke-width="2" fill="none" stroke-linecap="round" opacity="0.9" transform="rotate(-15 60 60)"/>
    
    <path d="M 69 75 Q 75 72 78 66" stroke="white" stroke-width="1.2" fill="none" stroke-linecap="round" opacity="0.6"/>

    <path class="tick-path" d="M49.8 60 L57 67.2 L70.2 52.8" stroke="#00f7ff" stroke-width="2" fill="none" stroke-linecap="round" stroke-linejoin="round" />
  </g>
</svg>
)V0G0N"));
    m_svgWidget->setFixedSize(60, 60);
    m_svgWidget->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_svgWidget->setStyleSheet("background: transparent; border: none;");

    m_popup = new TaskPopup();
    
    // Connect popup signals to logic
    connect(m_popup, &TaskPopup::taskAdded, this, &FloatingButton::handleTaskAdded);
    connect(m_popup, &TaskPopup::taskDeleted, this, &FloatingButton::handleTaskDeleted);
    connect(m_popup, &TaskPopup::taskEdited, this, &FloatingButton::handleTaskEdited);
    
    // Qt::Popup hides automatically on focus loss (like clicking the button).
    // Track when it hides so toggle doesn't immediately reopen it.
    connect(m_popup, &TaskPopup::popupHidden, this, [this]() {
        m_lastPopupHideTime = QDateTime::currentMSecsSinceEpoch();
    });

    m_popup->winId(); // Ensure window handler is created for blur logic
    
    // Apply blur to popup as well
    HWND hwndFallback = (HWND)m_popup->winId();
    if (hwndFallback) {
        DWM_BLURBEHIND bb = {0};
        bb.dwFlags = DWM_BB_ENABLE;
        bb.fEnable = true;
        bb.hRgnBlur = NULL;
        DwmEnableBlurBehindWindow(hwndFallback, &bb);
    }

    // Load saved position or use default
    QSettings settings("Developer", "MiniTasks");
    QPoint savedPos = settings.value("buttonPosition", QPoint(-1, -1)).toPoint();
    
    if (savedPos != QPoint(-1, -1)) {
        move(savedPos);
    } else {
        if (QScreen *screen = QGuiApplication::primaryScreen()) {
            QRect availableGeometry = screen->availableGeometry();
            int defaultX = availableGeometry.right() - width() - 16;
            int defaultY = availableGeometry.top() + 16;
            move(defaultX, defaultY);
        }
    }
}

FloatingButton::~FloatingButton()
{
    m_popup->deleteLater();
}

void FloatingButton::repositionPopup()
{
    if (QScreen *screen = QGuiApplication::primaryScreen()) {
        QRect availableGeometry = screen->availableGeometry();
        int popupX = x() + width() - m_popup->width();
        int popupY = y() + height() + 16;
        
        // Prevent popup from going off-screen (e.g. if button dragged too far bottom or right)
        if (popupX < availableGeometry.left()) popupX = availableGeometry.left() + 16;
        if (popupX + m_popup->width() > availableGeometry.right()) popupX = availableGeometry.right() - m_popup->width() - 16;
        if (popupY + m_popup->height() > availableGeometry.bottom()) popupY = y() - m_popup->height() - 16; // Spawn above button if at bottom

        m_popup->move(popupX, popupY);
    }
}

void FloatingButton::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    repositionPopup();
}

void FloatingButton::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_isDragging = true;
        m_movedDuringPress = false;
        m_dragStartPosition = event->globalPosition().toPoint();
        m_dragOffset = m_dragStartPosition - frameGeometry().topLeft();
        
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        if (now - m_lastPopupHideTime < 150) {
            // The popup JUST closed because of focus loss down-click.
            // Do not reopen it on release.
            m_suppressToggle = true;
        } else {
            m_suppressToggle = false;
        }
    }
    QWidget::mousePressEvent(event);
}

void FloatingButton::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isDragging) {
        QPoint currentPos = event->globalPosition().toPoint();
        
        // Add a 5px deadzone. If we haven't moved more than 5px, it's still just a jittery "click".
        if (!m_movedDuringPress) {
            if ((currentPos - m_dragStartPosition).manhattanLength() < 5) {
                return; // Ignore micro movements
            }
            m_movedDuringPress = true; // Threshold breached, it's a drag!
        }

        move(currentPos - m_dragOffset);
        
        // Hide the popup while dragging if it is visible to be smooth
        if (m_popup->isVisible()) {
            m_popup->hide();
        }
    }
    QWidget::mouseMoveEvent(event);
}

void FloatingButton::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_isDragging = false;
        
        if (m_movedDuringPress) {
            // Save the new position when drag completes
            QSettings settings("Developer", "MiniTasks");
            settings.setValue("buttonPosition", pos());
        } else if (!m_suppressToggle) {
            // It was a pure click, not a drag, and it wasn't a closing click. Toggle it!
            togglePopup();
        }
    }
    QWidget::mouseReleaseEvent(event);
}

void FloatingButton::togglePopup()
{
    if (m_popup->isVisible()) {
        m_popup->hide();
    } else {
        auto tasks = m_storage.load();
        m_popup->reloadTasks(tasks);
        repositionPopup(); // Guarantee exact position before showing
        m_popup->show();
    }
}

void FloatingButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    // Left completely blank, as QSvgWidget handles display.
}

void FloatingButton::handleTaskAdded(const QString& task)
{
    m_storage.add(task);
    auto tasks = m_storage.load();
    m_popup->reloadTasks(tasks);
}

void FloatingButton::handleTaskDeleted(int index)
{
    m_storage.remove(index);
    auto tasks = m_storage.load();
    m_popup->reloadTasks(tasks);
}

void FloatingButton::handleTaskEdited(int index, const QString& newText)
{
    m_storage.update(index, newText);
    auto tasks = m_storage.load();
    m_popup->reloadTasks(tasks);
}
