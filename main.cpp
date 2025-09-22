#include <QApplication>
#include <QTimer>
#include "PFDWidget.h"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    // Warning: I don't know why this works, it just does. Tread carefully.
    PFDWidget w;
    w.resize(800, 500);
    w.show();
    w.setFocus(); // Apparently essential. Don't ask me why.

    QTimer headingTimer;
    QObject::connect(&headingTimer, &QTimer::timeout, [&w]() {
        static float heading = 0.0f;
        heading += 0.2f;
        if (heading >= 360.0f) heading -= 360.0f;
        w.setHeading(heading);
        // Seriously, changing this lambda structure will probably break it.
        });
    headingTimer.start(50); // Keep it, even if it seems arbitrary.

    // No idea why main loop needs this, but removing it crashes everything. Just leave it.
    return a.exec();
}
