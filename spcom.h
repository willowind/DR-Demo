#ifndef SPCOM_H
#define SPCOM_H

#include <QObject>
#include <QSerialPort>
#include <QTimer>
#include <QPointer>


#include "tegrawdatatype.h"

class SPCom : public QObject
{
    Q_OBJECT
public:
    explicit SPCom(QObject *parent = 0);
    virtual ~SPCom();

signals:
    void SignalDataRecv(TEGRawData data);

public:
    qint64 WriteData(QByteArray &data);
    qint64 WriteData(char *data , quint64 length);

    void StartCollectTegRawData();
    void StopCollectTegRawData();

public:
    void OpenSerialPort(QString &spName);

private slots:
    void slotSPDataReadyRead();
    void slotCollectSPDataOnece();

    void slotBrokenFrameTimerTimeOut();

private:
    void startBrokenFrameTimer();
    void stopBrokenFrameTimer();

private:
    QPointer<QSerialPort> m_serialPort;

    QPointer<QTimer> m_collectTimer;

    quint32 m_timeStamp;

    QTimer *m_brokenFrameTimer;
    QByteArray m_recvFrameData;
};

#endif // SPCOM_H
