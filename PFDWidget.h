#pragma once
#include <QWidget>
#include <QFont>
#include <QTimer>
#include <QSet>

class PFDWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PFDWidget(QWidget* parent = nullptr);

    // Setters
    void setPitch(float deg);
    void setRoll(float deg);
    void setYaw(float deg);
    void setThrottle(float val);
    void setAltitude(float val);
    void setSpeed(float val);

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

private slots:
    void updateSimulation(); // Called by timer for continuous input

private:
    // Flight parameters
    float pitch = 0.0f;     // degrees
    float roll = 0.0f;      // degrees
    float yaw = 0.0f;       // degrees
    float throttle = 0.0f;  // percent 0-100
    float altitude = 10000; // feet
    float speed = 250;      // knots


    QFont font;
    QTimer simulationTimer;
    QSet<int> keysPressed; // Tracks keys currently held down

    // Drawing helpers
    void drawHorizon(QPainter& painter, int cx, int cy, int radius);
    void drawSpeedTape(QPainter& painter, int x, int y, int width, int height);
    void drawAltitudeTape(QPainter& painter, int x, int y, int width, int height);
    void drawRollIndicator(QPainter& painter, int cx, int cy, int radius);

    // HSI / heading
    float heading = 0.0f; // degrees

    // Autopilot modes
    bool apRollHold = false;
    bool apPitchHold = false;
    bool apNavMode = false;
    bool apApproachMode = false;

    // ILS / GS/LOC
    float glideSlopeDeflection = 0.0f;  // -1.0 to +1.0
    float localizerDeflection = 0.0f;   // -1.0 to +1.0

    // Flight director
    float fdPitch = 0.0f;
    float fdRoll = 0.0f;

    void drawHSI(QPainter& painter, int cx, int cy, int radius);
    void drawILS(QPainter& painter, int cx, int cy);
    void drawFlightDirector(QPainter& painter, int cx, int cy);

private:
    float fdTargetPitch = 0.0f;  // desired FD pitch
    float fdTargetRoll = 0.0f;  // desired FD roll

    float dispPitch = 0.0f;      // displayed FD pitch (smoothed)
    float dispRoll = 0.0f;      // displayed FD roll (smoothed)

};
