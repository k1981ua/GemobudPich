#include "mainwindow.h"
#include <QApplication>
#include <QStyleFactory>
#include <QDesktopWidget>




int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    //w.show();

    a.setStyle(QStyleFactory::create("fusion"));
    //show at center
    w.move(QApplication::desktop()->screen()->rect().center() - w.rect().center());

    //w.showFullScreen();
    w.show();

    return a.exec();
}
