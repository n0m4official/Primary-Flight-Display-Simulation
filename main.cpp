#include <QApplication>
#include "PFDWidget.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    PFDWidget pfd;
    pfd.resize(800, 600);
    pfd.show();

    return app.exec();
}
