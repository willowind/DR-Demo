#ifndef TEGRAWDATATYPE_H
#define TEGRAWDATATYPE_H

#include <qglobal.h>
#include <QMetaType>

struct TEGRawData
{
    TEGRawData(quint8 ch = 0 , float dt = 0 , quint32 tt = 0) : channel(ch) , data(dt) , timestamp(tt) {}

    quint8 channel;
    qint16 data;
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

/////////////////////////////////////////////////////////////////////////////////////////////
struct ChannelData
{
    ChannelData(qint16 ch1 = 0 , qint16 ch2 = 0) : chOneData(ch1) , chTwoData(ch2) {}

    qint16 chOneData;
    qint16 chTwoData;
};

#pragma pack(1)
enum RotaryControlType : unsigned char
{
    RCT_TurnStop = 0,
    RCT_TurnLeft = 1,
    RCT_TurnRight
}/*__attribute__ ((packed))*/;

struct RotaryHeaderType
{
    RotaryHeaderType(quint8 f = 0x55 , quint8 s = 0xaa , quint8 t = 0x7e) : first(f) , secend(s) , third(t) {}

    bool operator ==(const RotaryHeaderType &other)
    {
        if((other.first == first) && \
                (other.secend == secend) && \
                (other.third == third))
            return true;

        return false;
    }

    quint8 first;
    quint8 secend;
    quint8 third;
}/*__attribute__ ((packed))*/;

struct RotaryProtocolType
{
    RotaryProtocolType(RotaryHeaderType h = RotaryHeaderType() , RotaryControlType c = RCT_TurnStop , quint16 s = 0 , quint8 t = 10 , quint16 rs = 0) : header(h) , control(c) , rotaryStep(s) , delayTime(t) , totalRotaryStep(rs) {}

    RotaryHeaderType header;
    RotaryControlType control;
    quint16 rotaryStep;
    quint8 delayTime;
    quint16 totalRotaryStep;
}/*__attribute__ ((packed))*/;
#pragma pack()

Q_DECLARE_METATYPE(RotaryProtocolType);
Q_DECLARE_METATYPE(TEGRawData);

#endif // TEGRAWDATATYPE_H
