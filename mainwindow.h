#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include <QMap>

#include "spcom.h"
#include "rawpeakfilter.h"
#include "extracttegparam.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void slotRecvSPComData(TEGRawData data);

    void slotStartPushButtonClicked();
    void slotStopPushButtonClicked();
    void slotOpenFileActionTriggered();

    void slotPeakFilterDataReady(QList<TEGRawData> peakDatas);

    void slotWindowLineEditReturnPressed();
    void slotScalLineEditReturnPressed();

    void slotBaseLinePushButtonClicked();
private:
    void saveTegRawDataToFile(TEGRawData &data);
    void saveTegRawDataToFile(QMap<quint32 , qint16> &datas);

    void dealPeakCurveData(QList<TEGRawData> &datas);
    void dealHoleCurveData(QList<TEGRawData> &datas);

private:
    Ui::MainWindow *ui;

    SPCom *m_spcom;
    RawPeakFilter *m_rawPeakFilter;

    QString m_dataFileName;
    QFile m_rawDataFile;

    QList<TEGRawData> m_fileDataList;

    bool m_isBaseLineTest;
    float m_baseLineValue;

    ExtractTegParam *m_extractTegParam;
};

#endif // MAINWINDOW_H
