#include "alarmwindow.h"

#include <QApplication>
#include <QLocale>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QLocale::setDefault(QLocale::system());

    const QIcon appIcon(QStringLiteral(":/tray/logo.png"));
    if (!appIcon.isNull()) {
        app.setWindowIcon(appIcon);
    }

    AlarmWindow window;
    if (!appIcon.isNull()) {
        window.setWindowIcon(appIcon);
    }
    window.show();

    return app.exec();
}
