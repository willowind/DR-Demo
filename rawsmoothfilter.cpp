#include "rawsmoothfilter.h"

#define SAMPLE_POINT_SIZE (36) //采样点个数

RawSmoothFilter::RawSmoothFilter(QObject *parent) : QObject(parent)
{
    m_currRawDatas.clear();
    m_historyRawDatas.clear();
}

RawSmoothFilter::~RawSmoothFilter()
{
    m_currRawDatas.clear();
    m_historyRawDatas.clear();
}

void RawSmoothFilter::AppendRawData(TEGRawData &rawData)
{
    m_historyRawDatas.append(rawData);
    m_currRawDatas.append(rawData);

    if(smoothRawData(m_currRawDatas))
        m_currRawDatas.clear();
}

QList<TEGRawData> RawSmoothFilter::GetHistoryRawDatas()
{
    return m_historyRawDatas;
}

bool RawSmoothFilter::smoothRawData(QList<TEGRawData> &rawDatas)
{
    int validSize = rawDatas.size() / SAMPLE_POINT_SIZE;

    if(validSize <= 0)
        return false;

    QList<TEGRawData> smoothData;

    for(int i = 0 ; i < validSize ; i++)
    {
        float smoothValue = 0;
        quint32 smoothTimestamp = 0;
        TEGRawData tmpData;

        for(int j = 0 ; j < SAMPLE_POINT_SIZE ; j++)
        {
            tmpData = rawDatas.at(i*SAMPLE_POINT_SIZE + j);

            smoothValue += tmpData.data;
            smoothTimestamp += tmpData.timestamp;
        }

        smoothValue /= SAMPLE_POINT_SIZE;
        smoothTimestamp /= SAMPLE_POINT_SIZE;

        smoothData.append(TEGRawData(tmpData.channel , smoothValue , smoothTimestamp));
    }

    emit this->SIGNALSmoothFilterData(smoothData);

    return true;
}
