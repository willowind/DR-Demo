#ifndef EXTRACTTEGPARAM_H
#define EXTRACTTEGPARAM_H

#include <QObject>
#include <QMap>
#include <QPoint>

#include "tegrawdatatype.h"

class ExtractTegParam : public QObject
{
    Q_OBJECT
public:
    explicit ExtractTegParam(QObject *parent = 0);
    virtual ~ExtractTegParam();

public:
    void SetBaseLineValue(const float baseLine);
    void AppendPeakFilterData(qint32 timestamp , float value);
    void AppendPeakFilterData(QMap<qint32 , float> &datas);

    float ExtractRValue();
    float ExtractRValue(const float baseLine , QMap<qint32 , float> &datas);
    TegParamPoint ExtractRPoint() const;

    float ExtractKValue();
    TegParamPoint ExtractKPoint() const;

    float ExtractAngleValue();

    float ExtractMaValue();

private:
    float m_baseLineValue;
    QMap<qint32 , float> m_paramOrgDatas;

    TegParamPoint m_rvaluePoint;
    TegParamPoint m_kvaluePoint;
};

#endif // EXTRACTTEGPARAM_H
