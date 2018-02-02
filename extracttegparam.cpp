#include "extracttegparam.h"
#include <QDebug>

ExtractTegParam::ExtractTegParam(QObject *parent) : QObject(parent)
{
    m_baseLineValue = 0;
    m_paramOrgDatas.clear();
}

ExtractTegParam::~ExtractTegParam()
{

}

void ExtractTegParam::SetBaseLineValue(const float baseLine)
{
    m_baseLineValue = baseLine;
}

void ExtractTegParam::AppendPeakFilterData(qint32 timestamp, float value)
{
    m_paramOrgDatas.insert(timestamp , value);
}

void ExtractTegParam::AppendPeakFilterData(QMap<qint32, float> &datas)
{
    m_paramOrgDatas.clear();
    m_paramOrgDatas = datas;
}

float ExtractTegParam::ExtractRValue()
{
    float rbase = 15.0;

    float scale = m_baseLineValue * 0.01;
    float rbaseLine = m_baseLineValue + scale;

    float rscale = rbase * 0.15;
    float rvalueMin = rbaseLine + (rbase - rscale);
    float rvalueMax = rbaseLine + (rbase + rscale);

    QMap<qint32 , float> rmaps;

    QMapIterator<qint32 , float> i(m_paramOrgDatas);
    while(i.hasNext())
    {
        i.next();

        float value = i.value();
        if(value > rvalueMin)
        {
            if(value < rvalueMax)
                rmaps.insert(i.key() , value);
            else
            {
                if(rmaps.isEmpty())
                    rmaps.insert(i.key() , value);
                break;
            }
        }
    }

    if(rmaps.isEmpty())
        return 0;

    qDebug() << "rmaps.size() = " << rmaps.size();

    ////////////////////////////////////////////////////////////////////
    qint32 rtimestamp = 0;
    float rvalues = 0;
    QMapIterator<qint32 , float> r(rmaps);
    while(r.hasNext())
    {
        r.next();

        rtimestamp += i.key();
        rvalues += i.value();
    }

    rtimestamp /= rmaps.size();
    rvalues /= rmaps.size();

    m_rvaluePoint.ttx = rtimestamp;
    m_rvaluePoint.vvy = rvalues;

    int rvalue = rtimestamp - m_paramOrgDatas.firstKey();
    return rvalue / 600.0;
}

float ExtractTegParam::ExtractRValue(const float baseLine, QMap<qint32, float> &datas)
{
    SetBaseLineValue(baseLine);
    AppendPeakFilterData(datas);

    return ExtractRValue();
}

TegParamPoint ExtractTegParam::ExtractRPoint() const
{
    return m_rvaluePoint;
}

float ExtractTegParam::ExtractKValue()
{
    if(m_rvaluePoint.ttx == 0)
        return 0;
#if 0
    //斜率法
    float slopeMax = 0;

    float max = 0;
    QMapIterator<qint32 , float> i(m_paramOrgDatas);
    while(i.hasNext())
    {
        i.next();

        qint32 key = i.key();
        float value = i.value();

        if(value > max)
            max = value;

        if(key < m_rvaluePoint.ttx)
            continue;

        qint32 diffttx = key - m_rvaluePoint.ttx;
        float diffvvy = value - m_rvaluePoint.vvy;

        if(diffttx > 0)
        {
            float slope = diffvvy / diffttx;
            if(slope > slopeMax)
            {
                slopeMax = slope;
                m_kvaluePoint.ttx = key;
                m_kvaluePoint.vvy = value;
            }
        }
    }

    ///////////////////////////////////////////////////////////////////
    if(m_kvaluePoint.ttx == 0)
        return 0;

    int kvalue = m_kvaluePoint.ttx - m_rvaluePoint.ttx;
    return kvalue / 600.0;
#else
    float kbase = 180.0;

    float scale = m_baseLineValue * 0.01;
    float kbaseLine = m_baseLineValue + scale;

    float kscale = kbase * 0.1;
    float kvalueMin = kbaseLine + (kbase - kscale);
    float kvalueMax = kbaseLine + (kbase + kscale);

    QMap<qint32 , float> kmaps;

    QMapIterator<qint32 , float> i(m_paramOrgDatas);
    while(i.hasNext())
    {
        i.next();

        float value = i.value();
        if(value > kvalueMin)
        {
            if(value < kvalueMax)
                kmaps.insert(i.key() , value);
            else
            {
                if(kmaps.isEmpty())
                    kmaps.insert(i.key() , value);
                break;
            }
        }
    }

    if(kmaps.isEmpty())
        return 0;

    ////////////////////////////////////////////////////////////////////
    qint32 ktimestamp = 0;
    float kvalues = 0;
    QMapIterator<qint32 , float> k(kmaps);
    while(k.hasNext())
    {
        k.next();

        ktimestamp += i.key();
        kvalues += i.value();
    }

    ktimestamp /= kmaps.size();
    kvalues /= kmaps.size();

    m_kvaluePoint.ttx = ktimestamp;
    m_kvaluePoint.vvy = kvalues;

    int kvalue = ktimestamp - m_rvaluePoint.ttx;
    return kvalue / 600.0;
#endif
}

TegParamPoint ExtractTegParam::ExtractKPoint() const
{
    return m_kvaluePoint;
}

float ExtractTegParam::ExtractAngleValue()
{
    if(m_rvaluePoint.ttx == 0)
        return 0;

    if(m_kvaluePoint.ttx == 0)
        return 0;

    qint32 diffttx = m_kvaluePoint.ttx - m_rvaluePoint.ttx;
    float diffvvy = m_kvaluePoint.vvy - m_rvaluePoint.vvy;

    if(diffttx <= 0)
        return 0;

    //float tan = diffvvy / diffttx;
    float piAngle = atan2(diffvvy , diffttx);
    float angle = 90 - (piAngle * 180) / 3.14159;

    qDebug() << "piangle = " << piAngle;
    qDebug() << "angle = " << angle;

    return angle;
}

float ExtractTegParam::ExtractMaValue()
{
    if(m_kvaluePoint.ttx == 0)
        return 0;
}
