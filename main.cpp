#include "mainwindow.h"
#include <QApplication>

#include "tegrawdatatype.h"
#include "rotarytest.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    qRegisterMetaType<TEGRawData>("TEGRawData");
    qRegisterMetaType<TEGRawData>("TEGRawData&");
    qRegisterMetaType<RotaryProtocolType>("RotaryProtocolType");


    RotaryTest w;
//    MainWindow w;
    w.show();

    return a.exec();
}
