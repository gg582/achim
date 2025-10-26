#include "alarmwindow.h"

#include <QApplication>
#include <QLocale>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QLocale::setDefault(QLocale::system());

    AlarmWindow window;
    window.show();

    return app.exec();
}
