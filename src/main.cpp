#include "alarmwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    AlarmWindow window;
    window.show();

    return app.exec();
}
