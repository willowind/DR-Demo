#ifndef ROTARYTEST_H
#define ROTARYTEST_H

#include <QWidget>
#include <QMultiMap>

#include "spcom.h"
#include "ringbuffer.h"

struct AngleVoltageType
{
    AngleVoltageType() : angle(0) , testVol(-1) , verifyVol(-1) ,  testVolXP(0) , testVolYP(0) , verifyVolXP(0) , verifyVolYP(0){}


    AngleVoltageType & operator =(const AngleVoltageType &other)
    {
         angle = other.angle;
         testVol = other.testVol;
         verifyVol = other.verifyVol;

         testVolXP = other.testVolXP;
         testVolYP = other.testVolYP;

         verifyVolXP = other.verifyVolXP;
         verifyVolYP = other.verifyVolYP;

        return *this;
    }

    int angle;
    int testVol;
    int verifyVol;

    int testVolXP;
    int testVolYP;

    int verifyVolXP;
    int verifyVolYP;
};

//////////////////////////////////////////////////////////////////////////////////////////////

namespace Ui {
class RotaryTest;
}

class RotaryTest : public QWidget
{
    Q_OBJECT

public:
    explicit RotaryTest(QWidget *parent = 0);
    ~RotaryTest();

private slots:
    void slotRecvSPComData(TEGRawData data);
    void slotTestPushButtonToggled(bool toggled);
    void slotVerifyPushButtonToggled(bool toggled);
    void slotRecordPushButtonPressed();
    void slotAnalysisPushButtonPressed();

    void slotReturnZeroPushButtonPressed();
    void slotIncreasePushButtonPressed();
    void slotDecreasePushButtonPressed();

private:
    Ui::RotaryTest *ui;

    SPCom *m_spcom;

    int m_angleCurrTest;
    int m_voltageCurrTest;

    QMultiMap<int , AngleVoltageType> m_angleVoltageMaps;

    bool m_isTestAngleVoltage;
    bool m_isVerifyAngleVoltage;

    RingBuffer *m_avRingBuffer;
};

#endif // ROTARYTEST_H
