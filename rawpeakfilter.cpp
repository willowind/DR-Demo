
#include <QDebug>
#include <QPointF>

#include "rawpeakfilter.h"

#define FILTER_WINDOW_SIZE (5)
#define CYCLE_WINDOW_SIZE (55) //周期窗口大小

#define THRESHOLD_SCALE	(0.4)  //阈值比例系数
#define NEIGHBOUR_RADIUS (35)  //搜索邻域半径
#define MAX_SUB_LEN (30)  //中值滤波或者均值滤波时子序列最大长度

static int compare(float a,float b){
    if(a - b > 0.000001)
        return 1;
    if(a - b < -0.000001)
        return -1;
    return 0;
}

static bool compareTegData(const TEGRawData &t1 , const TEGRawData &t2)
{
    if(compare(t1.data , t2.data) < 0)
        return true;

    return false;
}

RawPeakFilter::RawPeakFilter(QObject *parent) : QObject(parent)
{
    m_isPeakLocked = false;
    m_peakValue = -1;
    m_rawDatas.clear();

    m_soomthHistoryRawDatas.clear();
    m_soomthCurrRawDatas.clear();
    m_peakDatas.clear();

    m_cycleWindowSize = CYCLE_WINDOW_SIZE;
    m_thresholdScale = THRESHOLD_SCALE;
}

RawPeakFilter::~RawPeakFilter()
{
    m_rawDatas.clear();
}

void RawPeakFilter::AppendRawData(qint16 data)
{
    if(m_rawDatas.size() < FILTER_WINDOW_SIZE - 1)
    {
        m_rawDatas.append(data);
        return;
    }

    ///////////////////////////////////////////////////////

    m_rawDatas.append(data);

    int mid = FILTER_WINDOW_SIZE / 2;

#if 0
    if((m_rawDatas[mid] >= m_rawDatas[mid-1]) && \
            (m_rawDatas[mid] >= m_rawDatas[mid-2]) && \
            (m_rawDatas[mid] >= m_rawDatas[mid-3]) && \
            (m_rawDatas[mid] >= m_rawDatas[mid+1]) && \
            (m_rawDatas[mid] >= m_rawDatas[mid+2]) && \
            (m_rawDatas[mid] >= m_rawDatas[mid+3]))
    {
        if((m_rawDatas[mid-1] >= m_rawDatas[mid-2]) && \
                (m_rawDatas[mid-2] >= m_rawDatas[mid-3]) && \
                (m_rawDatas[mid+1] >= m_rawDatas[mid+2]) && \
                (m_rawDatas[mid+2] >= m_rawDatas[mid+3]))
        {
            m_isPeakLocked = true;
            m_peakValue = m_rawDatas[mid];
        }
    }
    else if((m_rawDatas[mid] < m_rawDatas[mid-1]) && \
            (m_rawDatas[mid] < m_rawDatas[mid-2]) && \
            (m_rawDatas[mid] < m_rawDatas[mid-3]) && \
            (m_rawDatas[mid] < m_rawDatas[mid+1]) && \
            (m_rawDatas[mid] < m_rawDatas[mid+2]) && \
            (m_rawDatas[mid] < m_rawDatas[mid+3]))
    {
        if((m_rawDatas[mid-1] < m_rawDatas[mid-2]) && \
                (m_rawDatas[mid-2] < m_rawDatas[mid-3]) && \
                (m_rawDatas[mid+1] < m_rawDatas[mid+2]) && \
                (m_rawDatas[mid+2] < m_rawDatas[mid+3]))
        {
            m_isPeakLocked = true;
            m_peakValue = m_rawDatas[mid];
        }
    }
    else
    {
        m_isPeakLocked = false;
        m_peakValue = -1;
    }
#else
    if((m_rawDatas[mid] >= m_rawDatas[mid-1]) && \
            (m_rawDatas[mid] >= m_rawDatas[mid-2]) && \
            (m_rawDatas[mid] >= m_rawDatas[mid+1]) && \
            (m_rawDatas[mid] >= m_rawDatas[mid+2]))
    {
        if((m_rawDatas[mid-1] >= m_rawDatas[mid-2]) && \
                (m_rawDatas[mid+1] >= m_rawDatas[mid+2]))
        {
            m_isPeakLocked = true;
            m_peakValue = m_rawDatas[mid];
        }
    }
    else if((m_rawDatas[mid] < m_rawDatas[mid-1]) && \
            (m_rawDatas[mid] < m_rawDatas[mid-2]) && \
            (m_rawDatas[mid] < m_rawDatas[mid+1]) && \
            (m_rawDatas[mid] < m_rawDatas[mid+2]))
    {
        if((m_rawDatas[mid-1] < m_rawDatas[mid-2]) && \
                (m_rawDatas[mid+1] < m_rawDatas[mid+2]))
        {
            m_isPeakLocked = true;
            m_peakValue = m_rawDatas[mid];
        }
    }
    else
    {
        m_isPeakLocked = false;
        m_peakValue = -1;
    }
#endif
    ////////////////////////////////////////////////////////////
    m_rawDatas.removeFirst();
}

void RawPeakFilter::AppendRawData(TEGRawData &data)
{
    if(data.channel == 0)
        return;

    m_soomthHistoryRawDatas.append(data);
    m_soomthCurrRawDatas.append(data);

    if(findPeak(m_soomthCurrRawDatas))
        m_soomthCurrRawDatas.clear();
}

void RawPeakFilter::AppendRawData(QList<TEGRawData> &datas)
{
    m_soomthHistoryRawDatas.clear();
    m_soomthHistoryRawDatas.append(datas);

    findPeak(m_soomthHistoryRawDatas);
}

bool RawPeakFilter::IsRawPeakValueLocked()
{
    return m_isPeakLocked;
}

qint16 RawPeakFilter::RawPeakValue()
{
    return m_peakValue;
}

void RawPeakFilter::SetCycleWindowSize(int size)
{
    m_cycleWindowSize = size;

    findPeak(m_soomthHistoryRawDatas);
}

int RawPeakFilter::CycleWindowSize()
{
    return m_cycleWindowSize;
}

void RawPeakFilter::SetThresholdScale(float scale)
{
    m_thresholdScale = scale;

    findPeak(m_soomthHistoryRawDatas);
}

float RawPeakFilter::ThresholdScale()
{
    return m_thresholdScale;
}

bool RawPeakFilter::findPeak(QList<TEGRawData> &rawDatas)
{
    int validSize = rawDatas.size() / m_cycleWindowSize;

    if(validSize <= 0)
        return false;

    QList<TEGRawData> peekList;

    for(int i = 0 ; i < validSize ; i++)
    {
        QList<TEGRawData> maxDataList;
        QList<TEGRawData> midRawData = rawDatas.mid(i*m_cycleWindowSize , m_cycleWindowSize);

//        if(i == 150)
//            qDebug();

        findMaxPointToList(midRawData , maxDataList);

        // once filter
        QList<TEGRawData> filterDataList = filterMaxPointInterField(maxDataList , m_thresholdScale);
        TEGRawData filterData = medianFilterForMaxPoints(filterDataList);

 //       TEGRawData peekData = medianFilterForMaxPoints(maxDataList);

        if(filterData.channel != 0)
            peekList.append(filterData);
//        peekList.append(maxDataList);
    }

    emit this->SIGNALPeakFilterDataReady(peekList);

    return true;
}

/*
int find_peaks(int sig[],int n1,int peaks[] )
{
    int maximums[MAX_SIGNAL_LEN];
    int n2 = find_maximums(sig,n1,maximums);
    int max = get_max(sig,n1);
    int min = get_min(sig,n1);
    double threshold = (max - min)*SCALE;
    int i,j,num=0;
    int found = 0;
    for(i=0;i<n2;i++){
        int index = maximums[i];
        if(max - sig[index] <= threshold)
            found = 1;
        else
            continue;

        for(j=index-NEIGHBOUR;j<=index+NEIGHBOUR;j++){
            if(j<0 || j>=n1)
                continue;
            if(sig[index] < sig[j])
                found = 0;
            if(sig[index] == sig[j] && is_exist(peaks,num,j))
                found = 0;
        }
        if(found == 1)
            peaks[num++] = index;
    }
    return num;
}
*/

void RawPeakFilter::findMaxPointToList(QList<TEGRawData> &rawDatas, QList<TEGRawData> &maxDatas)
{
    int len = rawDatas.size();
    if(len <= 1)
        return;

    for(int i = 1 ; i < len-1 ; i++)
    {
        TEGRawData curData = rawDatas.at(i);
        TEGRawData preData = rawDatas.at(i-1);
        TEGRawData nextData = rawDatas.at(i+1);

        if(compare(curData.data , preData.data) <= 0)   // sig[i-1] >= sig[i]
            continue;

        if(compare(curData.data , nextData.data) < 0)   //  sig[i-1] < sig[i] < sig[i+1]
            continue;

        if(compare(curData.data , nextData.data) > 0)    //  sig[i-1] < sig[i] > sig[i+1]
        {
            maxDatas.append(curData);
            continue;
        }

        int j = i+2; // sig[i-1] < sig[i] == sig[i+1]
        TEGRawData jData;

        for( ; j < len ; j++)
        {
            jData = rawDatas.at(j);
            if(compare(jData.data , curData.data) == 0)
                continue;
            else
                break;
        }

        if(j == len)
            break;

        if(compare(curData.data , jData.data) > 0)
            maxDatas.append(curData);

        i = j-1;
    }
}

QList<TEGRawData> RawPeakFilter::filterMaxPointInterField(QList<TEGRawData> &maxDatas, float thresholdScal)
{
    TEGRawData max = findMaxPointInMaxPointList(maxDatas);
    TEGRawData min = findMinPointInMaxPointList(maxDatas);

    QList<TEGRawData> filterList;
    float threshold = (max.data - min.data) * thresholdScal;

    for(int i = 0 ; i < maxDatas.size() ; i++)
    {
        TEGRawData cur = maxDatas.at(i);

        if((max.data - cur.data) <= threshold)
            filterList.append(cur);
    }

    return filterList;
}

TEGRawData RawPeakFilter::findMaxPointInMaxPointList(QList<TEGRawData> &maxDatas)
{
    TEGRawData max;
    float maxValue = -10000000.0;

    foreach (TEGRawData tmpData, maxDatas)
    {
        if(compare(tmpData.data , maxValue) > 0)
        {
            maxValue = tmpData.data;
            max = tmpData;
        }
    }

    return max;
}

TEGRawData RawPeakFilter::findMinPointInMaxPointList(QList<TEGRawData> &maxDatas)
{
    TEGRawData min;
    float minValue = 10000000.0;

    foreach (TEGRawData tmpData, maxDatas)
    {
        if(compare(tmpData.data , minValue) < 0)
        {
            minValue = tmpData.data;
            min = tmpData;
        }
    }

    return min;
}

TEGRawData RawPeakFilter::medianFilterForMaxPoints(QList<TEGRawData> &maxDatas)
{
    int length = maxDatas.size();
    if(length <= 0)
    {
        qDebug() << "medianFilterForMaxPoints() failed , find max pointe length is zero";
        return TEGRawData();
    }

//    if(length > 3)
//    {
//        qSort(maxDatas.begin() , maxDatas.end() , compareTegData);
//        maxDatas.removeFirst();
//        maxDatas.removeLast();
//    }

    float medianValue = 0;
    quint32 medianTimestamp = 0;
    TEGRawData tmpData;

    for(int i = 0 ; i < maxDatas.size() ; i++)
    {
        tmpData = maxDatas.at(i);
        medianValue += tmpData.data;
        medianTimestamp += tmpData.timestamp;
    }

    medianValue /= maxDatas.size();
    medianTimestamp /= maxDatas.size();

    return TEGRawData(tmpData.channel , medianValue , medianTimestamp);
}
