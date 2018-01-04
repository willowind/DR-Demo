#include "rotarytest.h"
#include "ui_rotarytest.h"

#include <QDebug>

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

    float displayValue = -2.375;
    for(int row = 0 ; row < 144 ;)
    {
        for(int column = 0 ; column < 100 ; column++)
        {
            QString str = QString("%1").arg(QString::number(displayValue , 'f' , 3));
            QTableWidgetItem *item = new QTableWidgetItem();

            item->setBackground(QBrush(QColor(Qt::lightGray)));
            item->setFlags(item->flags() & (~Qt::ItemIsEditable));
            item->setText(str);

            ui->tableWidget->setColumnWidth(column , 50);
            ui->tableWidget->setItem(row , column , item);

            displayValue += 0.001;

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

}

void RotaryTest::slotTestPushButtonToggled(bool toggled)
{
    m_isTestAngleVoltage = toggled;
}

void RotaryTest::slotVerifyPushButtonToggled(bool toggled)
{
    m_isVerifyAngleVoltage = toggled;
}

void RotaryTest::slotRecordPushButtonPressed()
{

}

void RotaryTest::slotAnalysisPushButtonPressed()
{

}
