#pragma once

#include <QWidget>
#include <QFont>
#include <QTimer>
#include <QSet>
#include <QPaintEvent>
#include <QKeyEvent>

class PFDWidget : public QWidget
{
	Q_OBJECT

public:
	explicit PFDWidget(QWidget* parent = nullptr);

	// Setters
	void setPitch(float deg);   // Just clamps it, don't ask why
	void setRoll(float deg);    // Roll: ±infinity in my nightmares
	void setYaw(float deg);     // Yaw, because why not
	void setThrottle(float val); // Don't go full 100 unless you want panic
	void setAltitude(float val); // Can't go negative, unlike my patience
	void setSpeed(float val);    // Speed limits: reality bites
	void setHeading(float deg);  // Wraps around, like my understanding of flight dynamics
	void setSelectedSpeed(int kt) { selectedSpeed = kt; update(); } // MCP selected speed, sigh

protected:
	void paintEvent(QPaintEvent* ev) override; // Draws stuff, somehow works
	void keyPressEvent(QKeyEvent* ev) override;  // Handle WASD madness
	void keyReleaseEvent(QKeyEvent* ev) override;

private slots:
	void updateSimulation(); // Timer calls this, probably too fast for my brain

private:
	// Flight parameters
	float pitch = 0.0f;     // degrees, sleep deprived sanity
	float roll = 0.0f;      // degrees, slowly rotating my patience
	float yaw = 0.0f;       // degrees, just trust me
	float throttle = 0.0f;  // percent 0-100, don’t break it
	float altitude = 10000; // feet, like my will to debug
	float speed = 250;      // knots, apparently fast enough
	float heading = 0.0f;   // degrees, rotates endlessly

	// V-Speeds / MCP selected speed (typical 737-800 values)
	float V1 = 150.0f;      // Takeoff decision speed, why do I care?
	float Vr = 160.0f;      // Rotation speed, feels fast
	float V2 = 170.0f;      // Takeoff safety speed, okay fine
	float Vref = 140.0f;    // Approach speed, sigh

	int selectedSpeed = 250; // MCP selected speed, again

	// These don't work, trying to fix them... sigh

	// ILS / guidance demo
	float localizerDeflection = 0.0f; // -1.0 to +1.0, barely makes sense
	float glideSlopeDeflection = 0.0f; // -1.0 to +1.0, same

	// Flight Director (commanded altitude)
	float fdPitch = 0.0f;   // smoothed FD pitch, hope this works
	float fdRoll = 0.0f;    // smoothed FD roll, good luck
	float fdTargetPitch = 0.0f;
	float fdTargetRoll = 0.0f;

	// Smoothing display helpers
	float dispPitch = 0.0f;
	float dispRoll = 0.0f;

	// UI
	QFont font; // Arial, default, I give up
	QTimer simulationTimer; // Drives the madness
	QSet<int> keysPressed; // Currently pressed keys, chaos ensues

	// Speed limits (737-800 approx)
	const int STALL_WARNING_SPEED = 140; // KIAS
	const int STALL_SPEED = 120;         // KIAS, crash optional
	const int VMO = 340;                 // Max operating speed, sure
	const int MMO = 0.82f;               // Max Mach, dreams die here
	// IDK why these don't work, need to spend more time fixing this

	// Attitude bugs and settings
	int decisionHeight = 200;       // DH in feet
	int baroPressure = 29.92f;    // inHg, because units are fun
	bool baroStd = true;      // Standard baro setting, obviously

	// Radio Altimeter
	float radioAltitude = 0.0f;		// feet AGL
	bool showRadioAlt = true;		// Show below 2500 ft, maybe

	// For smooth tape animation
	float displayedSpeed = 0.0f;
	float displayedAltitude = 0.0f;

	// Drawing helpers
	void drawHorizon(QPainter& p, int cx, int cy, int radius); // Horizon: rotates everything I know
	void drawFlightDirector(QPainter& p, int cx, int cy); // Green cross, sanity optional
	void drawSpeedTape(QPainter& p, int x, int y, int width, int height); // Numbers
	void drawAltitudeTape(QPainter& p, int x, int y, int width, int height); // Numbers that make me want to cry

	// Generic helper for smooth tapes
	void drawTape(QPainter& p, int x, int y, int width, int height,
		float displayedValue, float tickSpacing, int majorTick,
		const QString& unit, int decisionHeightBug,
		const std::vector<std::pair<int, QString>>& vSpeeds,
		int selectedBug);

	void drawHSI(QPainter& p, int cx, int cy, int radius); // Compass that works, somehow...
	void drawILS(QPainter& p, int cx, int cy); // Pink bars, don't ask

	// Altitude bug (altitude selector/bug for tape)
	int altBug = 0; // default to 0
	// THIS STILL DOESN'T WORK, removing it screws up the tape drawing for some reason, i give up

	// Altitude bug selected (highlighted/selected bug for tape)
	int altBugSelected = 0; // default to 0
	// Same here; doesn't work, but removing it breaks the tape drawing, so whatever

	// V-speeds for altitude tape (empty by default, can be filled as needed)
	std::vector<std::pair<int, QString>> vSpeeds; // None right now, maybe later
	// Also doesn't work, but removing it breaks literally everything, so yeah...
};
