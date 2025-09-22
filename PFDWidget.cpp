#include "PFDWidget.h"
#include <QPainter>
#include <QKeyEvent>
#include <QtMath>

// --- Constructor ---
PFDWidget::PFDWidget(QWidget* parent)
    : QWidget(parent)
{
    font = QFont("Arial", 12, QFont::Bold);
    setFocusPolicy(Qt::StrongFocus);

    // Timer for continuous simulation (50 Hz)
    connect(&simulationTimer, &QTimer::timeout, this, &PFDWidget::updateSimulation);
    simulationTimer.start(20);
}

// --- Setters ---
void PFDWidget::setPitch(float deg) { pitch = qBound(-90.0f, deg, 90.0f); update(); }
void PFDWidget::setRoll(float deg) { roll = fmodf(deg, 360.0f); update(); }
void PFDWidget::setYaw(float deg) { yaw = fmodf(deg, 360.0f); update(); }
void PFDWidget::setThrottle(float val) { throttle = qBound(0.0f, val, 100.0f); update(); }
void PFDWidget::setAltitude(float val) { altitude = qMax(0.0f, val); update(); }
void PFDWidget::setSpeed(float val) { speed = qMax(0.0f, val); update(); }

// --- Key input ---
void PFDWidget::keyPressEvent(QKeyEvent* event) { keysPressed.insert(event->key()); }
void PFDWidget::keyReleaseEvent(QKeyEvent* event) { keysPressed.remove(event->key()); }

// --- Continuous simulation ---
void PFDWidget::updateSimulation()
{
    // Step sizes
    const float pitchStep = 0.5f;
    const float rollStep = 0.5f;
    const float yawStep = 0.5f;
    const float throttleStep = 0.5f;
    const float altStep = 10.0f;

    // --- Apply keypresses ---
    if (keysPressed.contains(Qt::Key_W)) pitch += pitchStep;
    if (keysPressed.contains(Qt::Key_S)) pitch -= pitchStep;
    if (keysPressed.contains(Qt::Key_A)) roll -= rollStep;
    if (keysPressed.contains(Qt::Key_D)) roll += rollStep;
    if (keysPressed.contains(Qt::Key_Q)) yaw -= yawStep;
    if (keysPressed.contains(Qt::Key_E)) yaw += yawStep;
    if (keysPressed.contains(Qt::Key_R)) throttle += throttleStep;
    if (keysPressed.contains(Qt::Key_F)) throttle -= throttleStep;
    if (keysPressed.contains(Qt::Key_T)) altitude += altStep;
    if (keysPressed.contains(Qt::Key_G)) altitude -= altStep;

    // --- Auto-level target ---
    float targetPitch = pitch;
    float targetRoll = roll;

    if (!keysPressed.contains(Qt::Key_W) && !keysPressed.contains(Qt::Key_S))
        targetPitch = 0.0f;
    if (!keysPressed.contains(Qt::Key_A) && !keysPressed.contains(Qt::Key_D))
        targetRoll = 0.0f;

    // --- Smooth flight director movement ---
    const float alpha = 0.05f; // smoothing factor
    fdPitch += (targetPitch - fdPitch) * alpha;
    fdRoll += (targetRoll - fdRoll) * alpha;

    // Displayed values (used for small FD tilt offsets)
    dispPitch = fdPitch;
    dispRoll = fdRoll;

    // --- Clamp values ---
    pitch = qBound(-90.0f, pitch, 90.0f);
    roll = fmodf(roll, 360.0f);
    yaw = fmodf(yaw, 360.0f);
    throttle = qBound(0.0f, throttle, 100.0f);
    altitude = qMax(0.0f, altitude);

    // Speed depends on throttle
    speed = throttle * 5.0f + 50;

    // --- ILS demo ---
    localizerDeflection = qSin(qDegreesToRadians(heading * 3.0f)) * 0.5f;
    glideSlopeDeflection = qSin(qDegreesToRadians(heading * 2.0f)) * 0.5f;

    update(); // repaint
}

// --- Main paint event ---
void PFDWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int w = width();
    int h = height();
    int cx = w / 2;
    int cy = h / 2;

    // --- Background ---
    painter.fillRect(rect(), Qt::black);

    // --- Horizon + pitch ladder + roll arc ---
    drawHorizon(painter, cx, cy, h / 3);

    // --- Flight director (green cross) tied to horizon ---
    drawFlightDirector(painter, cx, cy);

    // --- Speed and altitude tapes ---
    drawSpeedTape(painter, 20, 50, 60, h - 100);
    drawAltitudeTape(painter, w - 80, 50, 60, h - 100);

    // --- Numeric readouts ---
    painter.setFont(font);
    painter.setPen(Qt::white);
    painter.drawText(cx - 50, h - 30, QString("ALT: %1").arg(int(altitude)));
    painter.drawText(cx - 50, h - 10, QString("SPD: %1").arg(int(speed)));
    painter.drawText(cx + 60, h - 30, QString("THR: %1%").arg(int(throttle)));
    painter.drawText(cx + 60, h - 10, QString("YAW: %1").arg(int(yaw)));

    // --- Heading indicator ---
    drawHSI(painter, cx, cy, h / 3);

    // --- ILS bars (purple), independent of horizon ---
    drawILS(painter, cx, cy);
}

// --- Horizon & pitch ladder ---
void PFDWidget::drawHorizon(QPainter& painter, int cx, int cy, int radius)
{
    painter.save();
    painter.translate(cx, cy);
    painter.rotate(-roll); // roll rotation

    float scale = 2.0f;
    float pitchOffset = -pitch * scale;

    // Sky
    painter.setBrush(QColor(80, 180, 255));
    painter.setPen(Qt::NoPen);
    painter.drawRect(-radius, -radius + pitchOffset, radius * 2, radius);

    // Ground
    painter.setBrush(QColor(160, 82, 45));
    painter.drawRect(-radius, 0 + pitchOffset, radius * 2, radius);

    // Horizon line
    painter.setPen(QPen(Qt::white, 2));
    painter.drawLine(-radius, 0 + pitchOffset, radius, 0 + pitchOffset);

    // Pitch ladder
    painter.setFont(QFont("Arial", 10));
    for (int p = -90; p <= 90; p += 10)
    {
        int y = pitchOffset + (-p) * scale;
        if (y < -radius || y > radius) continue;

        QPen pen(p == 0 ? Qt::white : Qt::lightGray, 2);
        painter.setPen(pen);

        int lineLength = (p == 0) ? radius : radius / 2;
        painter.drawLine(-lineLength, y, lineLength, y);

        if (p != 0) {
            painter.drawText(-lineLength - 25, y + 5, QString("%1").arg(p));
            painter.drawText(lineLength + 5, y + 5, QString("%1").arg(p));
        }
    }

    painter.restore();
}

// --- Roll arc ---
void PFDWidget::drawRollIndicator(QPainter& painter, int cx, int cy, int radius)
{
    painter.save();
    painter.translate(cx, cy);
    painter.setPen(QPen(Qt::white, 2));

    // Roll scale
    int arcRadius = radius + 20;
    painter.drawArc(-arcRadius, -arcRadius, arcRadius * 2, arcRadius * 2, 45 * 16, 90 * 16);

    // Bank pointer
    painter.rotate(-roll);
    painter.drawLine(0, 0, 0, -arcRadius);
    painter.restore();
}

// --- Speed tape ---
void PFDWidget::drawSpeedTape(QPainter& painter, int x, int y, int width, int height)
{
    painter.save();
    painter.setPen(Qt::white);
    painter.setBrush(QColor(30, 30, 30, 180));
    painter.drawRect(x, y, width, height);

    int centerY = y + height / 2;
    const int pixelsPerKt = 2; // 1 kt = 2 pixels, adjust for better spacing
    int minTick = int(speed) - 20; // show 40 kt range
    int maxTick = int(speed) + 20;

    for (int tickVal = minTick; tickVal <= maxTick; tickVal += 5) {
        int tickY = centerY + (speed - tickVal) * pixelsPerKt; // invert so higher speeds move up
        if (tickY < y || tickY > y + height) continue;

        int lineLength = (tickVal % 10 == 0) ? 15 : 8; // longer line every 10 kt
        painter.drawLine(x + width - lineLength, tickY, x + width, tickY);
        if (tickVal % 10 == 0)
            painter.drawText(x + 2, tickY + 5, QString("%1").arg(tickVal));
    }

    painter.restore();
}

// --- Altitude tape ---
void PFDWidget::drawAltitudeTape(QPainter& painter, int x, int y, int width, int height)
{
    painter.save();
    painter.setPen(Qt::white);
    painter.setBrush(QColor(30, 30, 30, 180));
    painter.drawRect(x, y, width, height);

    int centerY = y + height / 2;
    const int pixelsPer100Ft = 2; // 100 ft = 2 pixels
    int minTick = int(altitude) - 500; // show ±500 ft range
    int maxTick = int(altitude) + 500;

    for (int tickVal = minTick; tickVal <= maxTick; tickVal += 100) {
        int tickY = centerY + (altitude - tickVal) * pixelsPer100Ft; // invert so higher alt moves up
        if (tickY < y || tickY > y + height) continue;

        int lineLength = (tickVal % 500 == 0) ? 15 : 8; // longer line every 500 ft
        painter.drawLine(x, tickY, x + lineLength, tickY);
        if (tickVal % 500 == 0)
            painter.drawText(x + 12, tickY + 5, QString("%1").arg(tickVal));
    }

    painter.restore();
}

// --- HSI / Heading indicator ---
void PFDWidget::drawHSI(QPainter& painter, int cx, int cy, int radius)
{
    painter.save();
    painter.translate(cx, cy - radius - 40); // Top of PFD

    int w = 120;
    int h = 40;
    painter.setBrush(QColor(20, 20, 20, 200));
    painter.setPen(Qt::white);
    painter.drawRect(-w / 2, -h / 2, w, h);

    // Draw heading numbers
    for (int hdg = 0; hdg < 360; hdg += 30) {
        int dx = (hdg - heading) * w / 360;
        if (dx < -w / 2 || dx > w / 2) continue;
        painter.drawLine(dx, h / 2 - 5, dx, h / 2);
        painter.drawText(dx - 10, h / 2 - 10, QString("%1").arg(hdg));
    }

    // Heading bug (fixed marker at center)
    painter.setPen(QPen(Qt::yellow, 2));
    painter.drawLine(0, -h / 2, 0, h / 2);

    painter.restore();
}

// --- ILS indicators ---
void PFDWidget::drawILS(QPainter& painter, int cx, int cy)
{
    painter.save();
    painter.translate(cx, cy);

    int locX = localizerDeflection * 50;
    int gsY = -glideSlopeDeflection * 40;

    painter.setPen(QPen(Qt::magenta, 3));
    painter.drawLine(locX, -40, locX, 40);   // localizer
    painter.drawLine(-20, gsY, 20, gsY);     // glide slope

    painter.restore();
}

// --- Flight director ---
void PFDWidget::drawFlightDirector(QPainter& painter, int cx, int cy)
{
    painter.save();
    painter.translate(cx, cy); // center of PFD

    // Scale for pitch (pixels per degree)
    const float pitchScale = 2.0f;
    float fdPitchOffset = -dispPitch * pitchScale;

    // Small tilt for roll (just visual cue)
    const float rollTiltScale = 0.3f;
    float fdRollTilt = dispRoll * rollTiltScale;

    painter.setPen(QPen(Qt::green, 2));

    // Horizontal line
    painter.save();
    painter.rotate(fdRollTilt);
    painter.drawLine(-20, fdPitchOffset, 20, fdPitchOffset);
    painter.restore();

    // Vertical line
    painter.save();
    painter.rotate(fdRollTilt);
    painter.drawLine(0, fdPitchOffset - 20, 0, fdPitchOffset + 20);
    painter.restore();

    painter.restore();
}
