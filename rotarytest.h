#ifndef ROTARYTEST_H
#define ROTARYTEST_H

#include <QWidget>
#include <QMultiMap>

#include "spcom.h"

struct AngleVoltageType
{
    AngleVoltageType() : angle(0) , testVol(-1) , verifyVol(-1) ,  testVolXP(0) , testVolYP(0) , verifyVolXP(0) , verifyVolYP(0){}


    AngleVoltageType & operator =(AngleVoltageType &other)
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

    float angle;
    float testVol;
    float verifyVol;

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

private:
    Ui::RotaryTest *ui;

    SPCom *m_spcom;

    float m_angleCurrTest;
    float m_voltageCurrTest;

    QMultiMap<float , AngleVoltageType> m_angleVoltageMaps;

    bool m_isTestAngleVoltage;
    bool m_isVerifyAngleVoltage;
};

#endif // ROTARYTEST_H
