#ifndef SIMPLECURVEGRAPH_H
#define SIMPLECURVEGRAPH_H

#include <QWidget>
#include <QMap>

#include "tegrawdatatype.h"

class SimpleCurveGraph : public QWidget
{
    Q_OBJECT
public:
    explicit SimpleCurveGraph(QWidget *parent = nullptr);
    virtual ~SimpleCurveGraph();

signals:

public slots:

public:
    void UpdateCurveData(qint32 timeStamp , float data);
    void UpdateCurveData(QMap<qint32 , float> &datas);

    void UpdateCurveUpperData(QMap<qint32, float> &datas);

    void ClearCurveDatas();

    void SetCurveBaseLineValue(const float baseLine);
    void SetCurveRPointValue(TegParamPoint rpoint);
    void SetCurveKPointValue(TegParamPoint kpoint);

protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);

private:
    void calculatScaleXY();
    void calculatScaleX();
    void calculatScaleY();

    void paintBaseLines(QPainter &painter);
    void paintEnvelopingLine(QPainter &painter);
    void paintTegParamLines(QPainter &painter);

private:
    QMap<qint32 , float> m_curveDatas;
    QMap<qint32 , float> m_curveUpperDatas;

    qreal m_scaleX;
    qreal m_scaleY;

    int m_virtualX;
    float m_baseLineValue;
    float m_topLineValue;
    float m_bottomLineValue;

    TegParamPoint m_rpoint;
    TegParamPoint m_kpoint;
};

#endif // SIMPLECURVEGRAPH_H
