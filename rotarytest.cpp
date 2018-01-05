#include "rotarytest.h"
#include "ui_rotarytest.h"

#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QMessageBox>
#include <QFileDialog>

RotaryTest::RotaryTest(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RotaryTest)
{
    ui->setupUi(this);

    //////////////////////////////////////////////////////////////////////////////////
    m_angleCurrTest = 0;
    m_voltageCurrTest = 0;

    m_isTestAngleVoltage = false;
    m_isVerifyAngleVoltage = false;

    //////////////////////////////////////////////////////////////////////////////////
    m_spcom = new SPCom();
    connect(m_spcom , SIGNAL(SignalDataRecv(TEGRawData)) , this , SLOT(slotRecvSPComData(TEGRawData)));

    /////////////////////////////////////////////////////////////////////////////////
    ui->tableWidget->setColumnCount(100);
    ui->tableWidget->setRowCount(144);
    ui->tableWidget->horizontalHeader()->setVisible(false);
//    ui->tableWidget->verticalHeader()->setVisible(false);

    QStringList hheaders;
    for(int i = 0 ; i < 144 ;)
    {
        hheaders << "角度" << "记录" << "测试";
        i += 3;
    }

    ui->tableWidget->setVerticalHeaderLabels(hheaders);

    int displayValue = -2375;
    for(int row = 0 ; row < 144 ;)
    {
        for(int column = 0 ; column < 100 ; column++)
        {
            QString str = QString("%1").arg(QString::number(float(displayValue/1000.0) , 'f' , 3));
            QTableWidgetItem *item = new QTableWidgetItem();

            item->setBackground(QBrush(QColor(Qt::lightGray)));
            item->setFlags(item->flags() & (~Qt::ItemIsEditable));
            item->setTextAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
            item->setText(str);

            ui->tableWidget->setColumnWidth(column , 50);
            ui->tableWidget->setItem(row , column , item);

            displayValue += 1;

            ///////////////////////////////////////////////////////////////
            AngleVoltageType av;
            av.angle = displayValue;
            av.testVolXP = row + 1;
            av.testVolYP = column;
            av.verifyVolXP = row + 2;
            av.verifyVolYP = column;

            m_angleVoltageMaps.insert(displayValue , av);
        }

        row += 3;
    }

    ////////////////////////////////////////////////////////////////////////////
    ui->angleLcdNumber->setDigitCount(6);
    ui->voltageLcdNumber->setDigitCount(6);

    ui->angleLcdNumber->display(QString::number(m_angleCurrTest , 'f' , 3));
    ui->voltageLcdNumber->display(QString::number(m_voltageCurrTest , 'f' , 3));

    //////////////////////////////////////////////////////////////////////////////////
    connect(ui->testPushButton , SIGNAL(toggled(bool)) , this , SLOT(slotTestPushButtonToggled(bool)));
    connect(ui->verifyPushButton , SIGNAL(toggled(bool)) , this , SLOT(slotVerifyPushButtonToggled(bool)));
    connect(ui->recordPushButton , SIGNAL(pressed()) , this , SLOT(slotRecordPushButtonPressed()));
    connect(ui->analysisPushButton , SIGNAL(pressed()) , this , SLOT(slotAnalysisPushButtonPressed()));

    //////////////////////////////////////////////////////////////////////////////////
    m_avRingBuffer = new RingBuffer();
}

RotaryTest::~RotaryTest()
{
    delete ui;
}

void RotaryTest::slotRecvSPComData(TEGRawData data)
{
    qDebug() << "TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT" << data.timestamp << "  " << data.data;

    m_voltageCurrTest = data.data;
    ui->voltageLcdNumber->display(QString::number(m_voltageCurrTest , 'f' , 3));

    ///////////////////////////////////////////////////////////////////////////////////////////////
    //test
    if(m_isTestAngleVoltage)
    {
        m_avRingBuffer->PushDataToBuffer(data.data);
        if(m_avRingBuffer->IsBufferDataEqual())
        {
            int bufferData = m_avRingBuffer->GetBufferAverageData();

            ///////////////////////////////////////////////////////////////////////////////
            AngleVoltageType avItem = m_angleVoltageMaps.value(m_angleCurrTest);
            avItem.testVol = bufferData;
            m_angleVoltageMaps.replace(m_angleCurrTest , avItem);

            //////////////////////////////////////////////////////////////////////////////
            QString str = QString("%1").arg(QString::number(bufferData));
            QTableWidgetItem *item = new QTableWidgetItem();

            item->setTextColor(QColor(Qt::darkGreen));
            item->setFlags(item->flags() & (~Qt::ItemIsEditable));
            item->setTextAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
            item->setText(str);
            ui->tableWidget->setItem(avItem.testVolXP , avItem.testVolYP , item);

            //////////////////////////////////////////////////////////////////////////////
            m_avRingBuffer->Clear();
            m_angleCurrTest += 1;
            if(m_angleCurrTest > 2375)
                ui->testPushButton->setChecked(false);
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////
    //verify
    if(m_isVerifyAngleVoltage)
    {
        m_avRingBuffer->PushDataToBuffer(data.data);
        if(m_avRingBuffer->IsBufferDataEqual())
        {
            int bufferData = m_avRingBuffer->GetBufferAverageData();

            ///////////////////////////////////////////////////////////////////////////////
            AngleVoltageType avItem = m_angleVoltageMaps.value(m_angleCurrTest);
            avItem.verifyVol = bufferData;
            m_angleVoltageMaps.replace(m_angleCurrTest , avItem);

            //////////////////////////////////////////////////////////////////////////////
            QString str = QString("%1").arg(QString::number(bufferData));
            QTableWidgetItem *item = new QTableWidgetItem();

            item->setTextColor(QColor(Qt::darkMagenta));
            item->setFlags(item->flags() & (~Qt::ItemIsEditable));
            item->setTextAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
            item->setText(str);
            ui->tableWidget->setItem(avItem.verifyVolXP , avItem.verifyVolYP , item);

            //////////////////////////////////////////////////////////////////////////////
            m_avRingBuffer->Clear();
            m_angleCurrTest -= 1;
            if(m_angleCurrTest < -2375)
                ui->verifyPushButton->setChecked(false);
        }
    }
}

void RotaryTest::slotTestPushButtonToggled(bool toggled)
{
    m_isTestAngleVoltage = toggled;
    m_avRingBuffer->Clear();

    /////////////////////////////////////////////////////////////////
    if(m_isTestAngleVoltage)
    {
        ui->testPushButton->setText("停止测试");

        ui->verifyPushButton->setEnabled(false);
        ui->recordPushButton->setEnabled(false);
        ui->analysisPushButton->setEnabled(false);
    }
    else
    {
        ui->testPushButton->setText("测试");

        ui->verifyPushButton->setEnabled(true);
        ui->recordPushButton->setEnabled(true);
        ui->analysisPushButton->setEnabled(true);
    }
}

void RotaryTest::slotVerifyPushButtonToggled(bool toggled)
{
    m_isVerifyAngleVoltage = toggled;
    m_avRingBuffer->Clear();

    ////////////////////////////////////////////////////////////////
    if(m_isVerifyAngleVoltage)
    {
        ui->verifyPushButton->setText("停止验证");

        ui->testPushButton->setEnabled(false);
        ui->recordPushButton->setEnabled(false);
        ui->analysisPushButton->setEnabled(false);
    }
    else
    {
        ui->verifyPushButton->setText("验证");

        ui->testPushButton->setEnabled(true);
        ui->recordPushButton->setEnabled(true);
        ui->analysisPushButton->setEnabled(true);
    }
}

void RotaryTest::slotRecordPushButtonPressed()
{
    QString fileName("avtvd-");
    QDateTime currDateTime = QDateTime::currentDateTime();
    fileName += currDateTime.toString("yyyy-MM-dd-hh-mm-ss");

    //////////////////////////////////////////////////////////////////////////
    //  open file
    QFile file(fileName);
    if(!file.open(QIODevice::Text | QIODevice::WriteOnly))
    {
        qDebug() << "open file failed...  " << fileName;
        return;
    }

    //////////////////////////////////////////////////////////////////////////
    // write data to file
    QTextStream out(&file);

    QMapIterator<int , AngleVoltageType> i(m_angleVoltageMaps);
    while(i.hasNext())
    {
        i.next();

        AngleVoltageType av = i.value();
        out << av.angle << " " << av.testVol << " " << av.verifyVol << '\n';
    }

    ///////////////////////////////////////////////////////////////////////////
    file.flush();
    file.close();

    QMessageBox::information(this , "记录数据" , "数据记录完成");
}

void RotaryTest::slotAnalysisPushButtonPressed()
{
    QList<QMultiMap<int , AngleVoltageType> > avMapList;

    QStringList fileNames = QFileDialog::getOpenFileNames();
    for(int i = 0 ; i < fileNames.size() ; i++)
    {
        QString fileName = fileNames.at(i);
        QFile file(fileName);
        if(!file.open(QIODevice::Text | QIODevice::ReadOnly))
        {
            qDebug() << "open file failed...  " << fileName;
            continue;
        }

        QMultiMap<int , AngleVoltageType> avMap;
        QTextStream in(&file);
        while(!in.atEnd())
        {
            QString lineData = in.readLine(256);
            QStringList datas = lineData.split(" ");
            if(datas.size() != 3)
                continue;

            QString angle = datas.at(0);
            QString testValue = datas.at(1);
            QString verifyValue = datas.at(2);

            AngleVoltageType  av = m_angleVoltageMaps.value(angle.toInt());
            av.angle = angle.toInt();
            av.testVol = testValue.toInt();
            av.verifyVol = verifyValue.toInt();
            avMap.insert(av.angle , av);
        }

        avMapList.append(avMap);
        file.close();
    }


    //////////////////////////////////////////////////////////////////////////
    //  analysis datas , 1 sum
    QMultiMap<int , AngleVoltageType> analysisAvMap;
    for(int index = 0 ; index < avMapList.size() ; index++)
    {
        QMultiMap<int , AngleVoltageType> avMap = avMapList.at(index);
        if(analysisAvMap.size() != avMap.size())
        {
            analysisAvMap = avMap;
            continue;
        }

        QMapIterator<int , AngleVoltageType> i(avMap);
        while(i.hasNext())
        {
            i.next();

            int angle = i.key();
            AngleVoltageType av = i.value();

            AngleVoltageType avSrc = analysisAvMap.value(angle);

            avSrc.testVol += av.testVol;
            avSrc.verifyVol += av.verifyVol;

            analysisAvMap.replace(angle , avSrc);
        }
    }

    // 2 average
    int size = avMapList.size();
    QMultiMap<int , AngleVoltageType> displayAvMap;
    QMapIterator<int , AngleVoltageType> i(analysisAvMap);
    while(i.hasNext())
    {
        i.next();

        int angle = i.key();
        AngleVoltageType av = i.value();

        av.testVol /= size;
        av.verifyVol /= size;

        displayAvMap.insert(angle , av);
    }

    //3 display
    m_angleVoltageMaps = displayAvMap;
    QMapIterator<int , AngleVoltageType> j(displayAvMap);
    while(j.hasNext())
    {
        j.next();

        AngleVoltageType av = j.value();

        ////////////////////////////////////////////////////////////////////////////
        // display test data
        QString str = QString("%1").arg(QString::number(av.testVol));
        QTableWidgetItem *item = new QTableWidgetItem();

        item->setTextColor(QColor(Qt::darkGreen));
        item->setFlags(item->flags() & (~Qt::ItemIsEditable));
        item->setTextAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
        item->setText(str);
        ui->tableWidget->setItem(av.testVolXP , av.testVolYP , item);

        //////////////////////////////////////////////////////////////////////////
        // display verify data
        str = QString("%1").arg(QString::number(av.verifyVol));
        item = new QTableWidgetItem();

        item->setTextColor(QColor(Qt::darkMagenta));
        item->setFlags(item->flags() & (~Qt::ItemIsEditable));
        item->setTextAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
        item->setText(str);
        ui->tableWidget->setItem(av.verifyVolXP , av.verifyVolYP , item);
    }

    qDebug();

}
