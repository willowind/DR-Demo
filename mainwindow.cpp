#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QPainter>
#include <QDateTime>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_isBaseLineTest = false;
    m_baseLineValue = 2085;

    m_rawPeakFilter = new RawPeakFilter();
    connect(m_rawPeakFilter , SIGNAL(SIGNALPeakFilterDataReady(QList<TEGRawData>)) , this , SLOT(slotPeakFilterDataReady(QList<TEGRawData>)));

    m_extractTegParam = new ExtractTegParam();

    m_spcom = new SPCom(SPM_CollectMode , QString("COM3"));
    connect(m_spcom , SIGNAL(SignalDataRecv(TEGRawData)) , this , SLOT(slotRecvSPComData(TEGRawData)));

    connect(ui->startPushButton , SIGNAL(pressed()) , this , SLOT(slotStartPushButtonClicked()));
    connect(ui->stopPushButton , SIGNAL(pressed()) , this , SLOT(slotStopPushButtonClicked()));
    connect(ui->actionOpen_File , SIGNAL(triggered(bool)) , SLOT(slotOpenFileActionTriggered()));

    connect(ui->baseLinePushButton , SIGNAL(pressed()) , this , SLOT(slotBaseLinePushButtonClicked()));

    connect(ui->windowLineEdit , SIGNAL(returnPressed()) , this , SLOT(slotWindowLineEditReturnPressed()));
    connect(ui->scanLineEdit , SIGNAL(returnPressed()) , this , SLOT(slotScalLineEditReturnPressed()));

    ui->startPushButton->setEnabled(true);
    ui->stopPushButton->setEnabled(false);

    ui->windowLineEdit->setText(QString::number(m_rawPeakFilter->CycleWindowSize()));
    ui->scanLineEdit->setText(QString::number(m_rawPeakFilter->ThresholdScale() , 'f'));

    ui->baseLineValueLabel->setText(QString::number(m_baseLineValue , 'f'));
}

void MainWindow::slotRecvSPComData(TEGRawData data)
{
    qDebug() << "TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT" << data.timestamp << "  " << data.data;

    ui->curveWidget->UpdateCurveData(data.timestamp , data.data);

    saveTegRawDataToFile(data);
}

void MainWindow::slotStartPushButtonClicked()
{
    ui->startPushButton->setEnabled(false);
    ui->stopPushButton->setEnabled(true);

    ///////////////////////////////////////////////////////////
    QDateTime currDateTime = QDateTime::currentDateTime();
    m_dataFileName = QString("%1%2").arg("tegrawdata").arg(currDateTime.toString("yyyy-MM-dd-HH-mm-ss"));

    if(m_rawDataFile.isOpen())
    {
        m_rawDataFile.flush();
        m_rawDataFile.close();
    }

    m_rawDataFile.setFileName(m_dataFileName);
    if(!m_rawDataFile.open(QIODevice::Text | QIODevice::ReadWrite | QIODevice::Append))
    {
        qDebug() << "open file failed...  " << m_dataFileName;
        return;
    }

    //////////////////////////////////////////////////////////////
    ui->curveWidget->ClearCurveDatas();

    //////////////////////////////////////////////////////////////
    m_spcom->StartCollectTegRawData();
}

void MainWindow::slotStopPushButtonClicked()
{
    ui->startPushButton->setEnabled(true);
    ui->stopPushButton->setEnabled(false);

    //////////////////////////////////////////////////////////////
    m_spcom->StopCollectTegRawData();

    //////////////////////////////////////////////////////////////
    if(m_rawDataFile.isOpen())
    {
        m_rawDataFile.flush();
        m_rawDataFile.close();
    }
}

void MainWindow::slotOpenFileActionTriggered()
{
    if(!ui->startPushButton->isEnabled())
        return;

    QString fileName = QFileDialog::getOpenFileName();

    if(fileName.isEmpty())
    {
        qDebug() << "the select file name is empty... failed!!!";
        return;
    }

    if(!fileName.contains("tegrawdata"))
    {
        qDebug() << "the select file name is wrong file... can not opened!!!";
        return;
    }

    m_dataFileName = fileName;

    if(m_rawDataFile.isOpen())
    {
        m_rawDataFile.flush();
        m_rawDataFile.close();
    }

    m_rawDataFile.setFileName(m_dataFileName);
    if(!m_rawDataFile.open(QIODevice::Text | QIODevice::ReadOnly))
    {
        qDebug() << "open file failed...  " << m_dataFileName;
        return;
    }

    ui->curveWidget->ClearCurveDatas();

    qint32 timeStamp = 0;
    QMap<qint32 , float> rawDatas;
    QList<TEGRawData> dataList;

    m_fileDataList.clear();
    QTextStream in(&m_rawDataFile);
    while(!in.atEnd())
    {
        QString lineData = in.readLine(256);
//        QStringList lineDataList = lineData.split(' ' , QString::SkipEmptyParts);
//        if(lineDataList.size() != 2)
//            continue;

//        QString timeStampStr = lineDataList.at(0);
//        QString dataStr = lineDataList.at(1);
//        rawDatas.insert(timeStampStr.toInt() , dataStr.toShort());

//        QStringList lineDataList = lineData.split("\t" , QString::SkipEmptyParts);
//        if(lineDataList.size() != 3)
//            continue;

//        QString timeStampStr = lineDataList.at(0);
//        QString dataStr = lineDataList.at(2);
//        rawDatas.insert(timeStampStr.toInt() , dataStr.toFloat());

        if(lineData.isEmpty())
            continue;

        timeStamp++;
//        m_rawPeakFilter->AppendRawData(lineData.toInt());
//        if(m_rawPeakFilter->IsRawPeakValueLocked())
//            rawDatas.insert(timeStamp , qAbs(m_rawPeakFilter->RawPeakValue() - 2071)/**0.02342857*4*/);
//            rawDatas.insert(timeStamp , m_rawPeakFilter->RawPeakValue()*0.02342857*4);

        rawDatas.insert(timeStamp , lineData.toFloat());

        m_fileDataList.append(TEGRawData(1 , lineData.toFloat() , timeStamp));
    }

//    QMapIterator<qint32 , float> i(rawDatas);
//    while(i.hasNext())
//    {
//        i.next();

//        float value = i.value();
//        if((value > 42) && (value <45))
//        {
//            qDebug() << "key = " << i.key();
//        }

//        if((value > 420) && (value <450))
//        {
//            qDebug() << "TTTTTTTTTTTTTTTTTTTTkey = " << i.key();
//        }
//    }

    ui->rawCurveWidget->UpdateCurveData(rawDatas);

    m_rawPeakFilter->AppendRawData(m_fileDataList);
}

void MainWindow::slotPeakFilterDataReady(QList<TEGRawData> peakDatas)
{
    dealPeakCurveData(peakDatas);
    dealHoleCurveData(peakDatas);
}

void MainWindow::slotWindowLineEditReturnPressed()
{
    QString value = ui->windowLineEdit->text();

    m_rawPeakFilter->SetCycleWindowSize(value.toInt());
}

void MainWindow::slotScalLineEditReturnPressed()
{
    QString value = ui->scanLineEdit->text();

    m_rawPeakFilter->SetThresholdScale(value.toFloat());
}

void MainWindow::slotBaseLinePushButtonClicked()
{
    m_isBaseLineTest = true;

    slotOpenFileActionTriggered();
}

void MainWindow::saveTegRawDataToFile(TEGRawData &data)
{
    if(!m_rawDataFile.isOpen())
        m_rawDataFile.open(QIODevice::Text | QIODevice::ReadWrite | QIODevice::Append);

    QTextStream out(&m_rawDataFile);
    out << data.timestamp << " " << data.data << endl;
    out.flush();
}

void MainWindow::saveTegRawDataToFile(QMap<quint32, qint16> &datas)
{
    if(!m_rawDataFile.isOpen())
        m_rawDataFile.open(QIODevice::Text | QIODevice::ReadWrite | QIODevice::Append);

    QTextStream out(&m_rawDataFile);

    QMapIterator<quint32 , qint16> i(datas);
    while(i.hasNext())
    {
        i.next();

        out << i.key() << " " << i.value() << endl;
    }

    out.flush();
}

void MainWindow::dealPeakCurveData(QList<TEGRawData> &datas)
{
    QMap<qint32 , float> rawDatas;

    foreach (TEGRawData peak, datas)
        rawDatas.insert(peak.timestamp , peak.data);

    ui->peakCurveWidget->UpdateCurveData(rawDatas);
}

void MainWindow::dealHoleCurveData(QList<TEGRawData> &datas)
{
    QMap<qint32 , float> rawDatas;

    QMap<qint32 , float> upperDatas;
    QMap<qint32 , float> downerDatas;

    float scale = m_baseLineValue * 0.01;
    float baseLineMin = m_baseLineValue - scale;
    float baseLineMax = m_baseLineValue + scale;

    bool abandonBegainShakeData = true;
    foreach (TEGRawData peak, datas)
    {
        rawDatas.insert(peak.timestamp , peak.data);

        if(peak.data >= baseLineMin)
        {
            if(peak.data < baseLineMax)
            {
                abandonBegainShakeData = false;

                upperDatas.insert(peak.timestamp , m_baseLineValue);
                downerDatas.insert(peak.timestamp , m_baseLineValue);
            }
            else
            {
                if(!abandonBegainShakeData)
                {
                    upperDatas.insert(peak.timestamp , peak.data);

                    float diff = peak.data - m_baseLineValue;
                    downerDatas.insert(peak.timestamp , peak.data - diff*2);
                }
            }
        }
    }


    if(!m_isBaseLineTest)
    {
        /////////////////////////////////////////////////////////////////////////////////////
        float rvalue = m_extractTegParam->ExtractRValue(m_baseLineValue , upperDatas);
        ui->rvalueLabel->setText(QString::number(rvalue , 'f'));

        float kvalue = m_extractTegParam->ExtractKValue();
        ui->kvalueLabel->setText(QString::number(kvalue , 'f'));

        float angleValue = m_extractTegParam->ExtractAngleValue();
        ui->angleValueLabel->setText(QString::number(angleValue , 'f'));

        float maValue = m_extractTegParam->ExtractMaValue();
        ui->maValueLabel->setText(QString::number(maValue , 'f'));

        ///////////////////////////////////////////////////////////////////////////////////////
        ui->curveWidget->SetCurveBaseLineValue(m_baseLineValue);

        ui->curveWidget->SetCurveRPointValue(m_extractTegParam->ExtractRPoint());
        ui->curveWidget->SetCurveKPointValue(m_extractTegParam->ExtractKPoint());

        ui->curveWidget->UpdateCurveData(upperDatas);
        ui->curveWidget->UpdateCurveUpperData(downerDatas);
    }




    /////////////////////////////////////////////////////////////
    if(m_isBaseLineTest)
    {
        m_isBaseLineTest = false;

        float baseLine = 0;
        QMapIterator<qint32 , float> i(rawDatas);
        while(i.hasNext())
        {
            i.next();

            baseLine += i.value();
        }

        baseLine /= rawDatas.size();

        m_baseLineValue = baseLine;
        ui->baseLineValueLabel->setText(QString::number(m_baseLineValue , 'f'));
    }
}


MainWindow::~MainWindow()
{
    delete ui;

    if(m_spcom)
    {
        disconnect(m_spcom , SIGNAL(SignalDataRecv(TEGRawData)) , this , SLOT(slotRecvSPComData(TEGRawData)));
        delete m_spcom;
    }

    if(m_rawPeakFilter)
        delete m_rawPeakFilter;
}
