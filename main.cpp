#include <QApplication>
#include <QFontDatabase>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QApplication::setOrganizationName("WhatsMyIP");
    QApplication::setApplicationName("What's My IP");
    QApplication::setWindowIcon(QIcon(":/images/icon.png"));

    QFontDatabase::addApplicationFont(":/fonts/Fragment Core.otf");
    QFontDatabase::addApplicationFont(":/fonts/Hey Comic.ttf");

    MainWindow main;
    main.show();

    return QApplication::exec();
}
