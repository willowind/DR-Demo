#include "spcom.h"

#include <QDebug>
#include <QSerialPortInfo>
#include <QTime>

SPCom::SPCom(QObject *parent):QObject(parent)
{
    QList<QSerialPortInfo> spInfoList = QSerialPortInfo::availablePorts();
    QListIterator<QSerialPortInfo> i(spInfoList);

    qDebug() << spInfoList.size();
    m_errTimes = 1;

    while(i.hasNext())
    {
        QSerialPortInfo spInfo = i.next();
        qDebug() << "######################################################";
        qDebug() << "portName: " << spInfo.portName();
        qDebug() << "serialNumber: " << spInfo.serialNumber();
        qDebug() << "description: " << spInfo.description();
        qDebug() << "manufacturer: " << spInfo.manufacturer();
        qDebug() << "systemLocation: " << spInfo.systemLocation();
    }

    m_serialPort = new QSerialPort("COM3");

    m_serialPort->setBaudRate(QSerialPort::Baud9600);
    m_serialPort->setDataBits(QSerialPort::Data8);
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);
    m_serialPort->setStopBits(QSerialPort::OneStop);
    m_serialPort->setParity(QSerialPort::NoParity);
    m_serialPort->setReadBufferSize(1024);

    if(!m_serialPort->open(QIODevice::ReadWrite))
    {
        qDebug() << "open serialPort failed!!! " << m_serialPort->portName();
        return;
    }

    connect(m_serialPort , SIGNAL(readyRead()) , this , SLOT(slotSPDataReadyRead()));

    ///////////////////////////////////////////////////////////
    m_brokenFrameTimer = new QTimer(NULL);
    m_brokenFrameTimer->setInterval(2);

    ///////////////////////////////////////////////////////////
    m_recvFrameData.clear();


    //////////////////////////////////////////////////////////
    m_timeStamp = 0;
    m_collectTimer = new QTimer();

    m_collectTimer->setInterval(1000);
//    m_collectTimer->setSingleShot(true);
    connect(m_collectTimer , SIGNAL(timeout()) , this , SLOT(slotCollectSPDataOnece()) , Qt::UniqueConnection);

    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));

    /////////////////////////////////////////////////////////////////////
}

void SPCom::OpenSerialPort(QString &spName)
{

}

void SPCom::slotSPDataReadyRead()
{
//    qDebug("recv data throw SPCom...");

//    while(m_serialPort->bytesAvailable() >= 4)
//    {
//        m_recvFrameData += m_serialPort->readAll();
//    }

    if(m_serialPort->bytesAvailable() < 4)
        return;

    m_recvFrameData += m_serialPort->readAll();
    m_serialPort->clear(QSerialPort::Input);

    startBrokenFrameTimer();
}

void SPCom::slotCollectSPDataOnece()
{
    qDebug("write data to module throw SPCom");

//    QByteArray data(1 , '1');

//    WriteData(data);
//    qsrand(4095);


    TEGRawData rawData;
    rawData.channel = 1;
    rawData.timestamp = m_timeStamp;
    rawData.data = qrand() % 4096;

    emit this->SignalDataRecv(rawData);

    m_timeStamp++;
}

void SPCom::slotBrokenFrameTimerTimeOut()
{

//    qDebug() << "................................. recv frame data............................";

    stopBrokenFrameTimer();

//    TEGRawData rawData;
//    rawData.channel = 1;
//    rawData.timestamp = m_timeStamp;
//    rawData.data = m_recvFrameData.toFloat();

//    emit this->SignalDataRecv(rawData);



    /////////////////////////////////////////////////////////
    if(m_recvFrameData.size() != 4)
    {
        qDebug() << "................................. recv frame data. times = " << m_errTimes++;
        qDebug() << "tmpdata.size = " << m_recvFrameData.size();
        for(int i = 0 ; i < m_recvFrameData.size() ; i++)
            qDebug("0x%2x" , m_recvFrameData.at(i));
    }
    /////////////////////////////////////////////////////////
    m_timeStamp++;
    m_recvFrameData.clear();
}

void SPCom::startBrokenFrameTimer()
{
    if(m_brokenFrameTimer)
    {
        connect(m_brokenFrameTimer , SIGNAL(timeout()) , this , SLOT(slotBrokenFrameTimerTimeOut()) , Qt::UniqueConnection);
        m_brokenFrameTimer->start();
    }
}

void SPCom::stopBrokenFrameTimer()
{
    if(m_brokenFrameTimer)
    {
        disconnect(m_brokenFrameTimer , SIGNAL(timeout()) , this , SLOT(slotBrokenFrameTimerTimeOut()));
        m_brokenFrameTimer->stop();
    }
}

qint64 SPCom::WriteData(QByteArray &data)
{
    return WriteData(data.data() , data.length());
}

qint64 SPCom::WriteData(char *data, quint64 length)
{
    if(!m_serialPort)
        return 0;

    qint64 writelen = m_serialPort->write(data , length);
    m_serialPort->flush();

    return writelen;
}

void SPCom::StartCollectTegRawData()
{
    if(!m_collectTimer)
    {
        qDebug() << "SPCom collectTimer is Null, can not start.....";
        return;
    }

    connect(m_collectTimer , SIGNAL(timeout()) , this , SLOT(slotCollectSPDataOnece()) , Qt::UniqueConnection);
    m_collectTimer->start();
}

void SPCom::StopCollectTegRawData()
{
    if(!m_collectTimer)
        return;

    disconnect(m_collectTimer , SIGNAL(timeout()) , this , SLOT(slotCollectSPDataOnece()));
    m_collectTimer->stop();
}

SPCom::~SPCom()
{
    if(m_serialPort)
    {
        disconnect(m_serialPort , SIGNAL(readyRead()) , this , SLOT(slotSPDataReadyRead()));
        delete m_serialPort;
    }
}
