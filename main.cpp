#include "mainwindow.h"
#include <QApplication>

#include "tegrawdatatype.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    qRegisterMetaType<TEGRawData>("TEGRawData");
    qRegisterMetaType<TEGRawData>("TEGRawData&");

    MainWindow w;
    w.show();

    return a.exec();
}
