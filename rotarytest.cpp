#include "rotarytest.h"
#include "ui_rotarytest.h"

#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QMessageBox>
#include <QFileDialog>


#define RIGHT_BORDER_MAX (237)
#define LEFT_BORDER_MAX (-237)

#define STEP_MONITOR_ANGLE_STEP (180.0) //放大100倍，精确到0.01
#define REDUCTION_RATE (60.0)
#define MONITOR_DRIVER_SUBDIVIDE (4.0)
#define ROTARY_STEPS_PRE_ANGLE ((REDUCTION_RATE * MONITOR_DRIVER_SUBDIVIDE) / STEP_MONITOR_ANGLE_STEP)

RotaryTest::RotaryTest(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RotaryTest)
{
    ui->setupUi(this);

    //////////////////////////////////////////////////////////////////////////////////
    m_angleCurrTest = 0;
    m_voltageCurrTest = 0;

    m_isTestAngleVoltage = false;
    m_isRightHalfTestFinished = false;
    m_isleftHalfTestFinished = false;

    m_isVerifyAngleVoltage = false;
    m_isReturnZeroing = false;

    //////////////////////////////////////////////////////////////////////////////////
    m_spcomCollecter = new SPCom(SPM_CollectMode , QString("COM5"));
    connect(m_spcomCollecter , SIGNAL(SignalDataRecv(TEGRawData)) , this , SLOT(slotRecvSPComData(TEGRawData)));

    m_spcomControl = new SPCom(SPM_ControlMode , QString("COM4"));
    connect(m_spcomControl , SIGNAL(SignalControlDataRecv(RotaryProtocolType)) , this , SLOT(slotRecvSPComControlData(RotaryProtocolType)));

    /////////////////////////////////////////////////////////////////////////////////
    ui->tableWidget->setColumnCount(36);
    ui->tableWidget->setRowCount(42);
    ui->tableWidget->horizontalHeader()->setVisible(false);
//    ui->tableWidget->verticalHeader()->setVisible(false);

    QStringList hheaders;
    for(int i = 0 ; i < 42 ;)
    {
        hheaders << "角度" << "记录" << "测试";
        i += 3;
    }

    ui->tableWidget->setVerticalHeaderLabels(hheaders);

    int displayValue = -237;
    for(int row = 0 ; row < 42 ;)
    {
        for(int column = 0 ; column < 36 ; column++)
        {
            QString str = QString("%1").arg(QString::number(float(displayValue/100.0) , 'f' , 2));
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
    ui->voltageLcdNumber->display(QString::number(m_voltageCurrTest));

    //////////////////////////////////////////////////////////////////////////////////
    connect(ui->testPushButton , SIGNAL(toggled(bool)) , this , SLOT(slotTestPushButtonToggled(bool)));
    connect(ui->verifyPushButton , SIGNAL(toggled(bool)) , this , SLOT(slotVerifyPushButtonToggled(bool)));
    connect(ui->recordPushButton , SIGNAL(pressed()) , this , SLOT(slotRecordPushButtonPressed()));
    connect(ui->analysisPushButton , SIGNAL(pressed()) , this , SLOT(slotAnalysisPushButtonPressed()));
    connect(ui->returnZeroPushButton , SIGNAL(pressed()) , this , SLOT(slotReturnZeroPushButtonPressed()));
    connect(ui->increasePushButton , SIGNAL(pressed()) , this , SLOT(slotIncreasePushButtonPressed()));
    connect(ui->decreasePushButton , SIGNAL(pressed()) , this , SLOT(slotDecreasePushButtonPressed()));

    //////////////////////////////////////////////////////////////////////////////////
    m_avRingBuffer = new RingBuffer();

    RotaryProtocolType rotary;
    qDebug() << "sizeof(rotary.header) = " << sizeof(rotary.header);
    qDebug() << "sizeof(rotary.control) = " << sizeof(rotary.control);
    qDebug() << "sizeof(rotary) = " << sizeof(rotary);
}

RotaryTest::~RotaryTest()
{
    delete ui;
}

void RotaryTest::slotRecvSPComData(TEGRawData data)
{
//    qDebug() << "TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT" << data.timestamp << "  " << data.data;

    m_voltageCurrTest = data.data;
    ui->voltageLcdNumber->display(QString::number(m_voltageCurrTest));

    if(m_isReturnZeroing)
        return;

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

            //////////////////////////////////////////////////////////////////////////////
            autoTextNextAngleVoltage();
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

            //////////////////////////////////////////////////////////////////////////////
            autoTextNextAngleVoltage();
        }
    }
}

void RotaryTest::slotRecvSPComControlData(RotaryProtocolType data)
{
    m_currRotaryProData = data;

    if(m_isReturnZeroing)
    {
        int angleDiff = 0;
        if(m_angleCurrTest > 0)
            angleDiff = m_angleCurrTest - data.totalRotaryStep;
        else
            angleDiff = m_angleCurrTest + data.totalRotaryStep;

        ui->angleLcdNumber->display(QString::number(angleDiff , 'f' , 3));

        //////////////////////////////////////////////////////////////////////

        if(qAbs(m_angleCurrTest) == data.totalRotaryStep)
        {
            m_isReturnZeroing = false;
            m_angleCurrTest = 0;

            autoTextNextAngleVoltage();
        }
    }
}

void RotaryTest::slotTestPushButtonToggled(bool toggled)
{
    m_isTestAngleVoltage = toggled;
    m_avRingBuffer->Clear();

    m_isleftHalfTestFinished = false;
    m_isRightHalfTestFinished = false;

    /////////////////////////////////////////////////////////////////
    if(m_isTestAngleVoltage)
    {
        ui->testPushButton->setText("测试");

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

    m_isleftHalfTestFinished = false;
    m_isRightHalfTestFinished = false;

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

void RotaryTest::slotReturnZeroPushButtonPressed()
{
    m_isReturnZeroing = true;

    //////////////////////////////////////////////////////////////////////////////////
    RotaryProtocolType rotaryProData;

    if(m_angleCurrTest > 0)
        rotaryProData.control = RCT_TurnLeft;
    else
        rotaryProData.control = RCT_TurnRight;

    rotaryProData.delayTime = 10;
    rotaryProData.rotaryStep = (qAbs(m_angleCurrTest) * ROTARY_STEPS_PRE_ANGLE) + 0.5;

    m_spcomControl->WriteData((char *)(&rotaryProData) , sizeof(rotaryProData));
}

void RotaryTest::slotIncreasePushButtonPressed()
{
    m_angleCurrTest += 1;

    RotaryProtocolType rotaryProData;

    rotaryProData.control = RCT_TurnRight;
    rotaryProData.rotaryStep = ROTARY_STEPS_PRE_ANGLE + 0.5;

    m_spcomControl->WriteData((char *)(&rotaryProData) , sizeof(rotaryProData));

    ui->angleLcdNumber->display(QString::number(m_angleCurrTest , 'f' , 3));
}

void RotaryTest::slotDecreasePushButtonPressed()
{
    m_angleCurrTest -= 1;

    RotaryProtocolType rotaryProData;

    rotaryProData.control = RCT_TurnLeft;
    rotaryProData.rotaryStep = ROTARY_STEPS_PRE_ANGLE + 0.5;

    m_spcomControl->WriteData((char *)(&rotaryProData) , sizeof(rotaryProData));

    ui->angleLcdNumber->display(QString::number(m_angleCurrTest , 'f' , 3));
}

void RotaryTest::autoTextNextAngleVoltage()
{
    RotaryProtocolType rotaryProData;

    ////////////////////////////////////////////////////////////////////////////////////////////
    //right
    if(!m_isRightHalfTestFinished)
    {
        m_angleCurrTest += 1;
        if(m_angleCurrTest > RIGHT_BORDER_MAX)
        {
            m_isRightHalfTestFinished = true;

            m_isReturnZeroing = true;

            ///////////////////////////////////////////////////
            // return origen point
            rotaryProData.control = RCT_TurnLeft;
            rotaryProData.rotaryStep  = (m_angleCurrTest * ROTARY_STEPS_PRE_ANGLE) + 0.5;
            rotaryProData.delayTime = 10;
        }
        else
        {
            rotaryProData.control = RCT_TurnRight;
            rotaryProData.rotaryStep  = ROTARY_STEPS_PRE_ANGLE + 0.5;
        }

        m_spcomControl->WriteData((char *)(&rotaryProData) , sizeof(rotaryProData));

        ui->angleLcdNumber->display(QString::number(m_angleCurrTest , 'f' , 3));

        return;
    }

    ////////////////////////////////////////////////////////////////////////////////////////
    //left
    if(!m_isleftHalfTestFinished)
    {
        m_angleCurrTest -= 1;
        if(m_angleCurrTest < LEFT_BORDER_MAX)
        {
            m_isleftHalfTestFinished = true;

            m_isReturnZeroing = true;

            ///////////////////////////////////////////////////
            // return origen point
            rotaryProData.control = RCT_TurnRight;
            rotaryProData.rotaryStep  = (qAbs(m_angleCurrTest) * ROTARY_STEPS_PRE_ANGLE) + 0.5;
            rotaryProData.delayTime = 10;
        }
        else
        {
            rotaryProData.control = RCT_TurnLeft;
            rotaryProData.rotaryStep  = ROTARY_STEPS_PRE_ANGLE + 0.5;
        }

        m_spcomControl->WriteData((char *)(&rotaryProData) , sizeof(rotaryProData));

        ui->angleLcdNumber->display(QString::number(m_angleCurrTest , 'f' , 3));

        return;
    }

    //////////////////////////////////////////////////////////////////////////////////////
    //all finished
    ui->testPushButton->setChecked(false);

    rotaryProData.control = RCT_TurnStop;
    m_spcomControl->WriteData((char *)(&rotaryProData) , sizeof(rotaryProData));
}
