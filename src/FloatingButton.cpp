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
    m_isAlarmUrgent = true; // force an evaluation flip on the first call
    updateSvgState(false);

    m_popup = new TaskPopup();
    m_sidePanel = new SidePanel();
    
    // Connect popup signals to logic
    connect(m_popup, &TaskPopup::taskAdded, this, &FloatingButton::handleTaskAdded);
    connect(m_popup, &TaskPopup::taskDeleted, this, &FloatingButton::handleTaskDeleted);
    connect(m_popup, &TaskPopup::taskDone, this, &FloatingButton::handleTaskDone);
    connect(m_popup, &TaskPopup::taskSnoozed, this, &FloatingButton::handleTaskSnoozed);
    connect(m_popup, &TaskPopup::taskEdited, this, &FloatingButton::handleTaskEdited);
    
    // Qt::Popup hides automatically on focus loss (like clicking the button).
    // Track when it hides so toggle doesn't immediately reopen it.
    connect(m_popup, &TaskPopup::popupHidden, this, [this]() {
        m_lastPopupHideTime = QDateTime::currentMSecsSinceEpoch();
        if (m_sidePanel->isVisible()) {
            m_sidePanel->hide();
        }
    });

    // Connect SidePanel navigation -> FloatingButton -> TaskPopup
    connect(m_sidePanel, &SidePanel::scrollTargetRequested, this, [this](int targetIndex) {
        if (m_popup->isVisible() && m_popup) {
            m_popup->scrollToTask(targetIndex);
        }
    });

    m_popup->winId(); // Ensure window handler is created for blur logic
    m_sidePanel->winId();
    
    // Apply blur to popup as well
    HWND hwndFallback = (HWND)m_popup->winId();
    if (hwndFallback) {
        DWM_BLURBEHIND bb = {0};
        bb.dwFlags = DWM_BB_ENABLE;
        bb.fEnable = true;
        bb.hRgnBlur = NULL;
        DwmEnableBlurBehindWindow(hwndFallback, &bb);
    }
    
    HWND hwndSide = (HWND)m_sidePanel->winId();
    if (hwndSide) {
        DWM_BLURBEHIND bb = {0};
        bb.dwFlags = DWM_BB_ENABLE;
        bb.fEnable = true;
        bb.hRgnBlur = NULL;
        DwmEnableBlurBehindWindow(hwndSide, &bb);
    }

    // Load saved position or use default
    QSettings settings("Developer", "MiniTasks");
    QPoint savedPos = settings.value("buttonPosition", QPoint(-1, -1)).toPoint();
    
    m_alarmTimer = new QTimer(this);
    connect(m_alarmTimer, &QTimer::timeout, this, &FloatingButton::checkAlarms);
    m_alarmTimer->start(15000); // Check every 15 seconds
    checkAlarms();
    
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
    m_sidePanel->deleteLater();
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
        
        int gap = 8;
        int sideX = popupX - m_sidePanel->width() - gap;
        if (sideX < availableGeometry.left()) {
            sideX = popupX + m_popup->width() + gap; // fallback to right side if no room on left
        }
        m_sidePanel->move(sideX, popupY);
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
            m_sidePanel->hide();
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
        m_sidePanel->hide();
    } else {
        auto tasks = m_storage.load();
        m_popup->reloadTasks(tasks);
        m_sidePanel->reloadSchedule(tasks);
        repositionPopup(); // Guarantee exact position before showing
        m_sidePanel->show(); // Show side panel first so popup takes focus afterwards
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
    m_sidePanel->reloadSchedule(tasks);
    checkAlarms();
}

void FloatingButton::handleTaskDeleted(int index)
{
    m_storage.remove(index);
    auto tasks = m_storage.load();
    m_popup->reloadTasks(tasks);
    m_sidePanel->reloadSchedule(tasks);
    checkAlarms();
}

void FloatingButton::handleTaskEdited(int index, const QString& newText)
{
    m_storage.update(index, newText);
    auto tasks = m_storage.load();
    m_popup->reloadTasks(tasks);
    m_sidePanel->reloadSchedule(tasks);
    checkAlarms();
}

void FloatingButton::handleTaskDone(int index, bool completed)
{
    m_storage.setCompleted(index, completed);
    auto tasks = m_storage.load();
    m_popup->reloadTasks(tasks);
    m_sidePanel->reloadSchedule(tasks);
    checkAlarms();
}

void FloatingButton::handleTaskSnoozed(int index)
{
    m_storage.snooze(index);
    auto tasks = m_storage.load();
    m_popup->reloadTasks(tasks);
    m_sidePanel->reloadSchedule(tasks);
    checkAlarms();
}

void FloatingButton::checkAlarms()
{
    auto tasks = m_storage.load();
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    bool hasUrgent = false;

    for (const auto& t : tasks) {
        if (!t.isCompleted && t.alarmTime > 0 && now >= t.alarmTime) {
            hasUrgent = true;
            break;
        }
    }

    if (hasUrgent != m_isAlarmUrgent) {
        updateSvgState(hasUrgent);
    }
}

void FloatingButton::updateSvgState(bool urgent)
{
    if (urgent == m_isAlarmUrgent) return;
    m_isAlarmUrgent = urgent;

    QString normalSvg = R"V0G0N(
<svg width="120" height="120" viewBox="0 0 120 120" xmlns="http://www.w3.org/2000/svg">
  <defs>
    <style>
      @keyframes float { 0%, 100% { transform: translateY(0px); } 50% { transform: translateY(-5px); } }
      @keyframes drawTick { 0% { stroke-dashoffset: 40; opacity: 0; } 10% { opacity: 1; } 100% { stroke-dashoffset: 0; opacity: 1; } }
      @keyframes pulse { 0%, 100% { filter: drop-shadow(0 0 1px #00f7ff) drop-shadow(0 0 2px #00f7ff); } 50% { filter: drop-shadow(0 0 2px #00f7ff) drop-shadow(0 0 4px #00f7ff); } }
      .bubble-group { animation: float 4s ease-in-out infinite; transform-origin: center; }
      .tick-path { stroke-dasharray: 40; stroke-dashoffset: 40; animation: drawTick 1.2s cubic-bezier(0.25, 1, 0.5, 1) forwards 0.3s, pulse 3s ease-in-out infinite 1.5s; }
    </style>
    <radialGradient id="bubble-body" cx="50%" cy="50%" r="50%" fx="40%" fy="40%">
      <stop offset="0%" stop-color="#fff" stop-opacity="0.05"/><stop offset="75%" stop-color="#a3cfff" stop-opacity="0.25"/><stop offset="95%" stop-color="#c98fff" stop-opacity="0.45"/><stop offset="100%" stop-color="#90e0ff" stop-opacity="0.6"/>
    </radialGradient>
    <linearGradient id="iridescence" x1="0%" y1="0%" x2="100%" y2="100%">
      <stop offset="20%" stop-color="#00ffff" stop-opacity="0.5"><animate attributeName="stop-color" values="#00ffff;#ff00ff;#ffff00;#00ffff" dur="5s" repeatCount="indefinite" /></stop>
      <stop offset="50%" stop-color="#ff00ff" stop-opacity="0.4"><animate attributeName="stop-color" values="#ff00ff;#ffff00;#00ffff;#ff00ff" dur="5s" repeatCount="indefinite" /></stop>
      <stop offset="80%" stop-color="#ffff00" stop-opacity="0.5"><animate attributeName="stop-color" values="#ffff00;#00ffff;#ff00ff;#ffff00" dur="5s" repeatCount="indefinite" /></stop>
    </linearGradient>
    <linearGradient id="sharp-highlight" x1="0%" y1="0%" x2="0%" y2="100%">
      <stop offset="0%" stop-color="white" stop-opacity="0.95"/><stop offset="100%" stop-color="white" stop-opacity="0"/>
    </linearGradient>
  </defs>
  <g class="bubble-group">
    <circle cx="60" cy="60" r="30" fill="url(#bubble-body)" stroke="url(#iridescence)" stroke-width="1"/>
    <path d="M 45 45 Q 60 33 75 45" stroke="url(#sharp-highlight)" stroke-width="2" fill="none" stroke-linecap="round" opacity="0.9" transform="rotate(-15 60 60)"/>
    <path d="M 69 75 Q 75 72 78 66" stroke="white" stroke-width="1.2" fill="none" stroke-linecap="round" opacity="0.6"/>
    <path class="tick-path" d="M49.8 60 L57 67.2 L70.2 52.8" stroke="#00f7ff" stroke-width="2" fill="none" stroke-linecap="round" stroke-linejoin="round" />
  </g>
</svg>
)V0G0N";

    QString urgentSvg = R"V0G0N(
<svg width="120" height="120" viewBox="0 0 120 120" xmlns="http://www.w3.org/2000/svg">
  <defs>
    <style>
      @keyframes pulseUrgent { 0%, 100% { filter: drop-shadow(0 0 4px #4f46e5) drop-shadow(0 0 10px #4f46e5); } 50% { filter: drop-shadow(0 0 12px #818cf8) drop-shadow(0 0 25px #a5b4fc); } }
      .bubble-group { animation: float 1.5s ease-in-out infinite; transform-origin: center; }
      @keyframes float { 0%, 100% { transform: translateY(0px); } 50% { transform: translateY(-8px); } }
      .tick-path { stroke-dasharray: 40; stroke-dashoffset: 0; animation: pulseUrgent 0.6s ease-in-out infinite; }
    </style>
    <radialGradient id="bubble-body" cx="50%" cy="50%" r="50%" fx="40%" fy="40%">
      <stop offset="0%" stop-color="#fff" stop-opacity="0.05"/><stop offset="75%" stop-color="#818cf8" stop-opacity="0.45"/><stop offset="95%" stop-color="#6366f1" stop-opacity="0.65"/><stop offset="100%" stop-color="#4f46e5" stop-opacity="0.8"/>
    </radialGradient>
    <linearGradient id="iridescence" x1="0%" y1="0%" x2="100%" y2="100%">
      <stop offset="20%" stop-color="#4f46e5" stop-opacity="0.7"><animate attributeName="stop-color" values="#4f46e5;#818cf8;#4f46e5" dur="1s" repeatCount="indefinite" /></stop>
      <stop offset="50%" stop-color="#818cf8" stop-opacity="0.6"><animate attributeName="stop-color" values="#818cf8;#4f46e5;#818cf8" dur="1s" repeatCount="indefinite" /></stop>
      <stop offset="80%" stop-color="#4f46e5" stop-opacity="0.7"><animate attributeName="stop-color" values="#4f46e5;#818cf8;#4f46e5" dur="1s" repeatCount="indefinite" /></stop>
    </linearGradient>
    <linearGradient id="sharp-highlight" x1="0%" y1="0%" x2="0%" y2="100%">
      <stop offset="0%" stop-color="white" stop-opacity="0.95"/><stop offset="100%" stop-color="white" stop-opacity="0"/>
    </linearGradient>
  </defs>
  <g class="bubble-group">
    <circle cx="60" cy="60" r="30" fill="url(#bubble-body)" stroke="url(#iridescence)" stroke-width="1"/>
    <path d="M 45 45 Q 60 33 75 45" stroke="url(#sharp-highlight)" stroke-width="2" fill="none" stroke-linecap="round" opacity="0.9" transform="rotate(-15 60 60)"/>
    <path class="tick-path" d="M49.8 60 L57 67.2 L70.2 52.8" stroke="#a5b4fc" stroke-width="2" fill="none" stroke-linecap="round" stroke-linejoin="round"/>
  </g>
</svg>
)V0G0N";

    m_svgWidget->load(QByteArray(urgent ? urgentSvg.toUtf8() : normalSvg.toUtf8()));
    m_svgWidget->setFixedSize(60, 60);
    m_svgWidget->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_svgWidget->setStyleSheet("background: transparent; border: none;");
}
