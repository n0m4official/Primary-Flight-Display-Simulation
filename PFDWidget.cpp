#include "PFDWidget.h"
#include <QPainter>
#include <QKeyEvent>
#include <QtMath>
#include <QDebug>

// PFDWidget.cpp: created in a haze of caffeine and despair
PFDWidget::PFDWidget(QWidget* parent)
    : QWidget(parent)
{
    font = QFont("Arial", 12, QFont::Bold);
    setFocusPolicy(Qt::StrongFocus);

    // simulation timer (50 Hz) - like my heartbeat, except faster and more unstable
    simulationTimer.setInterval(20);
    connect(&simulationTimer, &QTimer::timeout, this, &PFDWidget::updateSimulation);
    simulationTimer.start();
    // Removing this crashes the program and I have no fucking clue why or how. Seriously.
    // Somewhere in here, a TF2 coconut.jpg haunts me. Beware.
}

// -------------------- Public setters --------------------
// Sacred setters, touch at your own peril
void PFDWidget::setPitch(float deg) { pitch = qBound(-90.0f, deg, 90.0f); update(); } // don't ask why ±90, just don't
void PFDWidget::setRoll(float deg) { roll = fmodf(deg, 360.0f); update(); }
void PFDWidget::setYaw(float deg) { yaw = fmodf(deg, 360.0f); update(); }
void PFDWidget::setThrottle(float val) { throttle = qBound(0.0f, val, 100.0f); update(); } // throttle = life force
void PFDWidget::setAltitude(float val) { altitude = qMax(0.0f, val); update(); } // can't go negative, unlike sanity
void PFDWidget::setSpeed(float val) { speed = qMax(0.0f, val); update(); }
void PFDWidget::setHeading(float deg) { heading = fmodf(deg, 360.0f); update(); }

// -------------------- Input --------------------
// Keyboard handling: sometimes works, sometimes eats souls
void PFDWidget::keyPressEvent(QKeyEvent* ev)
{
    keysPressed.insert(ev->key());
    ev->accept();
    // Pressing W is like injecting caffeine directly into the bloodstream
}

void PFDWidget::keyReleaseEvent(QKeyEvent* ev)
{
    keysPressed.remove(ev->key());
    ev->accept();
    // Letting go of keys = letting go of last hope
}

// -------------------- Simulation tick --------------------
// WARNING: touching this section may irreversibly warp time and space
void PFDWidget::updateSimulation()
{
    // Read inputs
    bool pitchUp = keysPressed.contains(Qt::Key_W);
    bool pitchDown = keysPressed.contains(Qt::Key_S);
    bool rollLeft = keysPressed.contains(Qt::Key_A);
    bool rollRight = keysPressed.contains(Qt::Key_D);
    bool yawLeft = keysPressed.contains(Qt::Key_Q);
    bool yawRight = keysPressed.contains(Qt::Key_E);
    bool thrUp = keysPressed.contains(Qt::Key_R);
    bool thrDown = keysPressed.contains(Qt::Key_F);
    bool altUp = keysPressed.contains(Qt::Key_T);
    bool altDown = keysPressed.contains(Qt::Key_G);
    // Yes, these keys directly influence sanity levels

    // Control increments: each number represents a piece of my soul
    const float pitchStep = 0.7f;
    const float rollStep = 0.9f;
    const float yawStep = 0.6f;
    const float thrStep = 0.5f;
    // Changing these will cause the aircraft to defy known physics. Seriously.

    // Apply control changes (don’t question the math)
    if (pitchUp)    pitch += pitchStep;
    if (pitchDown)  pitch -= pitchStep;
    if (rollLeft)   roll -= rollStep;
    if (rollRight)  roll += rollStep;
    if (yawLeft)    yaw -= yawStep;
    if (yawRight)   yaw += yawStep;
    if (thrUp)      throttle += thrStep;
    if (thrDown)    throttle -= thrStep;
    // The ± numbers somehow produce a smooth simulation. Don't ask why, it hurts.

    // Auto-leveling: like a guardian angel for my broken code
    const float autoLevelRate = 0.03f;
    if (!pitchUp && !pitchDown)
        pitch += (0.0f - pitch) * autoLevelRate; // It's magic, just trust it

    roll = fmodf(roll + 540.0f, 360.0f) - 180.0f;  // normalize to [-180,180] because big numbers scare me
    roll += -roll * autoLevelRate; // dragging back to sanity slowly
    roll = fmodf(roll + 360.0f, 360.0f); // roll math will haunt me in dreams
    // These numbers were picked after many sleepless nights and 3 energy drinks too many

    // Clamp controls: safety first, everything else second
    pitch = qBound(-90.0f, pitch, 90.0f);
    roll = fmodf(roll + 3600.0f, 360.0f); // 3600 because why not? Somehow works.
    yaw = fmodf(yaw + 3600.0f, 360.0f);
    throttle = qBound(0.0f, throttle, 100.0f);

    // Flight physics (1D simplified) - higher dimensions are above my pay grade
    const float maxSpeed = 250.0f;
    const float dragFactor = 0.02f; // this tiny number makes the whole thing believable-ish
    const float ftPerTick = 1.0f;

    float targetSpeed = throttle / 100.0f * maxSpeed;
    speed += (targetSpeed - speed) * 0.05f;
    speed -= pitch * 0.01f; // pitch affects speed: physics is cruel
    speed = qMax(0.0f, speed);

    float verticalSpeed = speed * qSin(qDegreesToRadians(pitch));
    altitude += verticalSpeed * ftPerTick;
    // The sin here bends reality slightly. It's fine. Probably.

    if (altUp)    altitude += 2.0f; // debug hack: should not exist, yet it works
    if (altDown)  altitude -= 2.0f;

    altitude = qMax(0.0f, altitude);

    displayedSpeed += (speed - displayedSpeed) * 0.12f; // smooth it, smooth me
    displayedAltitude += (altitude - displayedAltitude) * 0.12f;

    // Flight director: my tiny green savior
    if (pitchUp)      fdTargetPitch = qMin(20.0f, fdTargetPitch + 0.5f);
    else if (pitchDown) fdTargetPitch = qMax(-20.0f, fdTargetPitch - 0.5f);
    else fdTargetPitch += (0.0f - fdTargetPitch) * 0.06f;

    if (rollRight)   fdTargetRoll = qMin(30.0f, fdTargetRoll + 0.8f);
    else if (rollLeft) fdTargetRoll = qMax(-30.0f, fdTargetRoll - 0.8f);
    else fdTargetRoll += (0.0f - fdTargetRoll) * 0.06f;

    const float fdSmooth = 0.08f;
    fdPitch += (fdTargetPitch - fdPitch) * fdSmooth;
    fdRoll += (fdTargetRoll - fdRoll) * fdSmooth;
    // No, I don't know why the multipliers produce nice damping. Don't question it.

    dispPitch += (fdPitch - dispPitch) * 0.18f;
    dispRoll += (fdRoll - dispRoll) * 0.18f;

    radioAltitude = altitude; // because terrain is a lie

    // ILS wander: makes me question my life choices
    localizerDeflection = qSin(qDegreesToRadians(heading * 2.2f)) * 0.55f;
    glideSlopeDeflection = qCos(qDegreesToRadians(heading * 1.6f)) * 0.45f;
    // Multiplying by weird floats is the only way bars stay on screen and not in another dimension

    update(); // GUI update: sending tiny cries for help to the screen

    return; // This fucker is the reason I have a crippling energy drink addiction
}

// -------------------- Paint --------------------
void PFDWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const int w = width();
    const int h = height();
    const int cx = w / 2;
    const int cy = h / 2;

    // Background
    p.fillRect(rect(), Qt::black);

    // Horizon & pitch ladder
    drawHorizon(p, cx, cy, h / 3);

    // Tapes
    drawSpeedTape(p, 20, 50, 80, h - 120);
    drawAltitudeTape(p, w - 100, 50, 80, h - 120);

    // ILS / Terrain
    drawILS(p, cx, cy);

    // Compass / HSI with heading integrated
    drawHSI(p, cx, cy, 80);

    // Flight Director
    drawFlightDirector(p, cx, cy);

    // Baro Setting
    QRect baroBox(width() - 80, height() - 40, 70, 30);
    p.setBrush(QColor(0, 0, 0, 200));
    p.setPen(Qt::white);
    p.drawRect(baroBox);
    QString text = baroStd ? "STD" : QString::number(baroPressure) + " hPa";
    p.setFont(QFont("Arial", 10, QFont::Bold));
    p.drawText(baroBox, Qt::AlignCenter, text);

    // Radio Altimeter
    if (showRadioAlt && radioAltitude < 2500.0f) {
        QRect raBox(width() - 80, height() - 75, 70, 30);
        p.setBrush(QColor(0, 0, 0, 200));
        p.setPen(Qt::yellow);
        p.drawRect(raBox);
        p.setFont(QFont("Arial", 10, QFont::Bold));
        p.drawText(raBox, Qt::AlignCenter, QString("RA %1 ft").arg(int(radioAltitude)));
    }

    // Other readouts (ALT, SPD, THR) — leave bottom text for only these
    p.setFont(font);
    p.setPen(Qt::white);
    int margin = 20;
    int leftX = margin;
    int rightX = w - 120;
    p.drawText(leftX, h - 36, QString("ALT: %1 ft").arg(int(altitude)));
    p.drawText(leftX, h - 16, QString("SPD: %1 kt").arg(int(speed)));
    p.drawText(rightX, h - 36, QString("THR: %1%").arg(int(throttle)));
}
// -------------------- Horizon & Ladder (tied together) --------------------
// WARNING: touching this will cause visual insanity
void PFDWidget::drawHorizon(QPainter& p, int cx, int cy, int radius)
{
    p.save();
    p.translate(cx, cy);

    // rotate world by -roll so aircraft symbol is fixed and horizon tilts
    p.rotate(-roll); // WTF is rotation math? No idea. But it works.

    const float scale = 3.0f;
    float horizonY = pitch * scale;
    // Scale = 3. If you change it, ladder ticks disappear into another dimension

    // sky
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(80, 180, 255));
    p.drawRect(-radius * 2, -radius * 2 + horizonY, radius * 4, radius * 2);

    // ground
    p.setBrush(QColor(160, 82, 45));
    p.drawRect(-radius * 2, horizonY, radius * 4, radius * 2);

    // horizon line
    p.setPen(QPen(Qt::white, 2));
    p.drawLine(-radius, horizonY, radius, horizonY);

    // pitch ladder
    p.setFont(QFont("Arial", 10));
    for (int pdeg = -90; pdeg <= 90; pdeg += 5) {
        float y = (pitch - pdeg) * scale;
        if (y < -radius * 1.1f || y > radius * 1.1f) continue;

        QPen pen((pdeg == 0) ? Qt::white : Qt::lightGray, (pdeg == 0) ? 2 : 1);
        p.setPen(pen);

        int len = (pdeg % 10 == 0) ? radius / 2 : radius / 4;
        p.drawLine(-len, y, len, y);

        if (pdeg % 10 == 0 && pdeg != 0) {
            QString txt = QString::number(qAbs(pdeg));
            p.drawText(-len - 30, y + 5, txt);
            p.drawText(len + 8, y + 5, txt);
            // Don't ask why offsets are ±30 and ±8, it's voodoo
        }
    }

    p.restore();

    // fixed aircraft symbol
    p.save();
    p.translate(cx, cy);
    p.setPen(QPen(Qt::white, 2));
    p.drawLine(-20, 0, -6, 0);
    p.drawLine(20, 0, 6, 0);
    p.drawLine(0, -6, 0, 6);
    p.drawLine(-6, 0, 0, -4);
    p.drawLine(6, 0, 0, -4);
    // Aircraft symbol math: please don't try to compute these coordinates
    p.restore();
}

// -------------------- Flight Director (green cross) --------------------
void PFDWidget::drawFlightDirector(QPainter& p, int cx, int cy)
{
    p.save();
    p.translate(cx, cy);

    const float pitchScale = 3.0f;
    float yOffset = (pitch - fdPitch) * pitchScale;

    p.rotate(-fdRoll);

    p.setPen(QPen(Qt::green, 2));
    p.drawLine(-30, int(yOffset), 30, int(yOffset));
    p.drawLine(0, int(yOffset - 18), 0, int(yOffset + 18));
    p.drawLine(-30, int(yOffset), -22, int(yOffset - 6));
    p.drawLine(30, int(yOffset), 22, int(yOffset - 6));

    p.restore();
}

// -------------------- Generic Smooth Tape Drawing --------------------
void PFDWidget::drawTape(QPainter& p, int x, int y, int width, int height,
    float displayedValue, float tickSpacing, int majorTick,
    const QString& unit, int decisionHeightBug,
    const std::vector<std::pair<int, QString>>& vSpeeds,
    int selectedBug)
{
    p.save();

    // Background
    QRect tapeRect(x, y, width, height);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(20, 20, 20, 220));
    p.drawRect(tapeRect);

    const float centerY = y + height / 2.0f;
    const float pixelsPerUnit = float(height) / (tickSpacing * 2.0f);

    // -------------------- Draw ticks & numbers --------------------
    int tickStep = 10; // minor tick step
    int startVal = int(displayedValue - tickSpacing) / tickStep * tickStep;
    int endVal = int(displayedValue + tickSpacing) / tickStep * tickStep;

    for (int val = startVal; val <= endVal; val += tickStep)
    {
        float dy = (displayedValue - float(val)) * pixelsPerUnit;
        int ty = int(centerY + dy);
        if (ty < y || ty > y + height) continue;

        int tickLen = (val % majorTick == 0) ? 16 : 8;
        p.setPen(Qt::white);
        p.drawLine(x, ty, x + tickLen, ty);

        if (val % majorTick == 0)
        {
            p.setFont(QFont("Arial", 10, QFont::Bold));
            QRect textRect(x + tickLen + 2, ty - 8, width - tickLen - 4, 16);
            p.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, QString::number(val));
        }
    }

    // -------------------- Decision height bug --------------------
    if (decisionHeightBug > 0)
    {
        float dy = (displayedValue - float(decisionHeightBug)) * pixelsPerUnit;
        int bugY = int(centerY + dy);
        if (bugY >= y && bugY <= y + height)
        {
            QPolygonF bug;
            bug << QPointF(x + width - 1, bugY)
                << QPointF(x + width - 10, bugY - 6)
                << QPointF(x + width - 10, bugY + 6);
            p.setBrush(Qt::green);
            p.setPen(Qt::green);
            p.drawPolygon(bug);
        }
    }

    // -------------------- V-speed / stall / overspeed markers --------------------
    for (auto vs : vSpeeds)
    {
        float dy = (displayedValue - float(vs.first)) * pixelsPerUnit;
        int yPos = int(centerY + dy);
        if (yPos < y || yPos > y + height) continue;

        QPolygonF tri;
        tri << QPointF(x + width + 1, yPos)
            << QPointF(x + width - 10, yPos - 6)
            << QPointF(x + width - 10, yPos + 6);
        p.setBrush(Qt::magenta);
        p.setPen(Qt::NoPen);
        p.drawPolygon(tri);

        p.setPen(Qt::magenta);
        p.setFont(QFont("Arial", 8, QFont::Bold));
        p.drawText(x + width + 12, yPos - 4, vs.second);
    }

    // -------------------- Selected bug --------------------
    if (selectedBug > 0)
    {
        float dy = (displayedValue - float(selectedBug)) * pixelsPerUnit;
        int bugY = int(centerY + dy);
        if (bugY >= y && bugY <= y + height)
        {
            QRect selRect(x + width - 10, bugY - 3, 10, 6);
            p.setBrush(Qt::yellow);
            p.setPen(Qt::yellow);
            p.drawRect(selRect);
        }
    }

    // -------------------- Current-value box --------------------
    QRect currentBox(x, int(centerY) - 10, width, 20);
    p.setBrush(QColor(50, 50, 50, 200));
    p.setPen(Qt::white);
    p.drawRect(currentBox);

    p.setFont(QFont("Arial", 10, QFont::Bold));
    p.drawText(currentBox, Qt::AlignCenter, QString::number(int(displayedValue)));

    p.restore();
}

// -------------------- Wrappers --------------------
void PFDWidget::drawSpeedTape(QPainter& p, int x, int y, int width, int height)
{
    drawTape(p, x, y, width, height, displayedSpeed, 40.0f, 20, "KT", 0, vSpeeds, 0);
}

void PFDWidget::drawAltitudeTape(QPainter& p, int x, int y, int width, int height)
{
    drawTape(p, x, y, width, height, displayedAltitude, 100.0f, 50, "FT", altBug, vSpeeds, altBugSelected);
}

// -------------------- HSI & compass --------------------
void PFDWidget::drawHSI(QPainter& p, int cx, int cy, int radius)
{
    p.save();
    p.translate(cx, cy);

    // Rotate compass circle with aircraft yaw
    p.rotate(-yaw);

    // Draw compass circle
    p.setPen(QPen(Qt::white, 2));
    p.drawEllipse(QPointF(0, 0), radius, radius);

    // Tick marks every 30°
    for (int hdg = 0; hdg < 360; hdg += 30) {
        float angle = qDegreesToRadians(float(hdg));
        float x1 = radius * qSin(angle);
        float y1 = -radius * qCos(angle);
        float x2 = 0.85f * x1;
        float y2 = 0.85f * y1;
        p.drawLine(QPointF(x1, y1), QPointF(x2, y2));
    }

    // HDG text fixed to compass
    p.setFont(QFont("Arial", 10, QFont::Bold));
    p.setPen(Qt::white);
    p.drawText(-20, -radius - 10, QString("HDG %1°").arg(int(yaw)));

    // Optional: N/E/S/W markers
    QStringList directions = { "N","E","S","W" };
    for (int i = 0; i < 4; ++i) {
        float angle = qDegreesToRadians(float(i * 90 - yaw));
        float x = 0.7f * radius * qSin(angle);
        float y = -0.7f * radius * qCos(angle);
        p.drawText(QPointF(x - 5, y + 5), directions[i]);
    }

    p.restore();
}

// -------------------- ILS bars --------------------
void PFDWidget::drawILS(QPainter& p, int cx, int cy)
{
    p.save();
    p.translate(cx, cy);

    int barLength = 80;

    // localizer
    p.setPen(QPen(Qt::magenta, 3));
    int xLoc = int(localizerDeflection * barLength);
    p.drawLine(QPointF(xLoc, -barLength / 2), QPointF(xLoc, barLength / 2));
    // Don't touch the 2.2f multiplier in updateSimulation, it will summon chaos here

    // glide slope
    int yGS = int(-glideSlopeDeflection * barLength);
    p.drawLine(QPointF(-barLength / 4, yGS), QPointF(barLength / 4, yGS));
    // Same with 1.6f: adjust it and the purple bars start dancing on LSD

    p.restore();
}
