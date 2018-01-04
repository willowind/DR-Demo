#ifndef TEGRAWDATATYPE_H
#define TEGRAWDATATYPE_H

#include <qglobal.h>
#include <QMetaType>

struct TEGRawData
{
    TEGRawData(quint8 ch = 0 , float dt = 0 , quint32 tt = 0) : channel(ch) , data(dt) , timestamp(tt) {}

    quint8 channel;
    float data;
    quint32 timestamp;
};

struct TegParamPoint
{
    TegParamPoint(qint32 tx = 0 , float vy = 0) : ttx(tx) , vvy(vy) {}

    TegParamPoint & operator =(TegParamPoint &other)
    {
        ttx = other.ttx;
        vvy = other.vvy;

        return *this;
    }

    qint32 ttx;
    float vvy;
};



Q_DECLARE_METATYPE(TEGRawData);

#endif // TEGRAWDATATYPE_H
