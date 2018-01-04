#ifndef RAWSMOOTHFILTER_H
#define RAWSMOOTHFILTER_H

#include <QObject>
#include <QList>

#include "tegrawdatatype.h"

class RawSmoothFilter : public QObject
{
    Q_OBJECT
public:
    explicit RawSmoothFilter(QObject *parent = 0);
    virtual ~RawSmoothFilter();

signals:
    void SIGNALSmoothFilterData(QList<TEGRawData> data);

public:
    void AppendRawData(TEGRawData &rawData);
    QList<TEGRawData> GetHistoryRawDatas();

private:
    bool smoothRawData(QList<TEGRawData> &rawDatas);

private:
    QList<TEGRawData> m_currRawDatas;
    QList<TEGRawData> m_historyRawDatas;
};

#endif // RAWSMOOTHFILTER_H
