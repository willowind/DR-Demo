#ifndef RAWPEAKFILTER_H
#define RAWPEAKFILTER_H

#include <QObject>
#include <QVector>

#include "tegrawdatatype.h"

class RawPeakFilter : public QObject
{
    Q_OBJECT
public:
    explicit RawPeakFilter(QObject *parent = 0);
    virtual ~RawPeakFilter();

signals:
    void SIGNALPeakFilterDataReady(QList<TEGRawData> data);

public:
    void AppendRawData(qint16 data);
    void AppendRawData(TEGRawData &data);
    void AppendRawData(QList<TEGRawData> &datas);

    bool IsRawPeakValueLocked();
    qint16 RawPeakValue();

    void SetCycleWindowSize(int size);
    int CycleWindowSize();

    void SetThresholdScale(float scale);
    float ThresholdScale();

private:
    bool findPeak(QList<TEGRawData> &rawDatas);
    void findMaxPointToList(QList<TEGRawData> &rawDatas , QList<TEGRawData> &maxDatas);
    QList<TEGRawData> filterMaxPointInterField(QList<TEGRawData> &maxDatas , float thresholdScal);

    TEGRawData findMaxPointInMaxPointList(QList<TEGRawData> &maxDatas);
    TEGRawData findMinPointInMaxPointList(QList<TEGRawData> &maxDatas);

    TEGRawData medianFilterForMaxPoints(QList<TEGRawData> &maxDatas);


private:
    QVector<qint16> m_rawDatas;
    bool m_isPeakLocked;
    qint16 m_peakValue;

    ///////////////////////////////////////////////
    QList<TEGRawData> m_soomthHistoryRawDatas;
    QList<TEGRawData> m_soomthCurrRawDatas;
    QList<TEGRawData> m_peakDatas;

    /////////////////////////////////////////////////
    int  m_cycleWindowSize;
    float m_thresholdScale;
};

#endif // RAWPEAKFILTER_H
