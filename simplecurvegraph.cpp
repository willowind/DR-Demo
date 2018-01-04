#include "simplecurvegraph.h"

#include <QPainter>
#include <QDebug>
#include <QStyle>
#include <QStyleOption>

#define DISPLAY_TIME_MAX (2*60*60*10) //100ms
#define DISPLAY_VOLTAGE_MAX (4096)

#define EQUAL_ZERO_FLOAT(a) (((a < 0.00001) && (a > -0.00001)) ? 1 : 0)

SimpleCurveGraph::SimpleCurveGraph(QWidget *parent) : QWidget(parent)
{
    setStyleSheet("background-color:black;");

    m_baseLineValue = 0;
    m_virtualX = width();

    calculatScaleXY();
}

SimpleCurveGraph::~SimpleCurveGraph()
{

}

void SimpleCurveGraph::UpdateCurveData(qint32 timeStamp, float data)
{
    m_curveDatas.insert(timeStamp , data);

    if(timeStamp > m_virtualX)
    {
        m_virtualX *= 2;
        calculatScaleX();
    }

    update();
}

void SimpleCurveGraph::UpdateCurveData(QMap<qint32, float> &datas)
{
    m_curveDatas = datas;

//    int size = m_curveDatas.size();
//    m_virtualX = m_virtualX * (size / width() + 1);

    m_virtualX = m_curveDatas.size();

    update();
}

void SimpleCurveGraph::UpdateCurveUpperData(QMap<qint32, float> &datas)
{
    m_curveUpperDatas = datas;
}

void SimpleCurveGraph::ClearCurveDatas()
{
    m_curveDatas.clear();
}

void SimpleCurveGraph::SetCurveBaseLineValue(const float baseLine)
{
    m_baseLineValue = baseLine;

    update();
}

void SimpleCurveGraph::SetCurveRPointValue(TegParamPoint rpoint)
{
    m_rpoint = rpoint;
}

void SimpleCurveGraph::SetCurveKPointValue(TegParamPoint kpoint)
{
    m_kpoint = kpoint;
}

void SimpleCurveGraph::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QStyleOption opt;
    opt.init(this);

    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

    painter.setRenderHint(QPainter::Antialiasing , true);
    painter.translate(0 , -height()/8);

    ////////////////////////////////////////////////////////////////////////
    calculatScaleXY();

    ////////////////////////////////////////////////////////////////////////
    paintBaseLines(painter);
    paintEnvelopingLine(painter);
    paintTegParamLines(painter);
}

void SimpleCurveGraph::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    calculatScaleXY();
}

void SimpleCurveGraph::calculatScaleXY()
{
   calculatScaleX();
   calculatScaleY();
}

void SimpleCurveGraph::calculatScaleX()
{
    float scaleX = (width()*1.0) / (DISPLAY_TIME_MAX);
    if(scaleX < 0.001)
        scaleX = 0.001;
    else if(scaleX > 1)
        scaleX = 1;

    m_scaleX = scaleX;
}

void SimpleCurveGraph::calculatScaleY()
{
    float scaleY = (height()*1.0) / DISPLAY_VOLTAGE_MAX;
    if(scaleY < 0.001)
        scaleY = 0.001;
    else if(scaleY > 1)
        scaleY = 1;

    m_scaleY = scaleY;
}

void SimpleCurveGraph::paintBaseLines(QPainter &painter)
{
    if(m_baseLineValue < 0.1)
        return;

    painter.save();

    QPen pen;
    pen.setColor(QColor(255, 255, 255));
    pen.setWidth(1);
    pen.setDashOffset(1);
    pen.setStyle(Qt::DashLine);
    painter.setPen(pen);

    float midY = m_baseLineValue;
    float interval = DISPLAY_VOLTAGE_MAX / 16.0;

    //middle line
    painter.drawLine(QPointF(0 , midY*m_scaleY) , QPointF(width() , midY*m_scaleY));

    //upper lines , 4 lines
    int step;
    for(step = 1 ; step <= 4 ; step++)
    {
        float y = (midY - (interval * step)) * m_scaleY;
        QPointF point1(0 , y);
        QPointF point2(width() , y);

        painter.drawLine(point1 , point2);
    }

    m_topLineValue = midY - (interval * (step - 1));

    //downer lines , 4 lines
    for(step = 1 ; step <= 4 ; step++)
    {
        float y = (midY + (interval * step)) * m_scaleY;
        QPointF point1(0 , y);
        QPointF point2(width() , y);

        painter.drawLine(point1 , point2);
    }

    m_bottomLineValue = midY + (interval * (step - 1));

    painter.restore();
}

void SimpleCurveGraph::paintEnvelopingLine(QPainter &painter)
{
    // 设置字体：微软雅黑、点大小50、斜体
//    QFont font;
//    font.setFamily("Microsoft YaHei");
//    font.setPointSize(70);
//    font.setItalic(true);
//    painter.setFont(font);

    painter.save();

    QPen pen;
    pen.setColor(QColor(255, 255, 255));
    pen.setWidth(2);
    painter.setPen(pen);

    QMap<qint32 , float> curveDatas;
    QMapIterator<qint32 , float> cd(m_curveDatas);
    while(cd.hasNext())
    {
        cd.next();

        qint32 key = cd.key() * m_scaleX;
        float value = cd.value() * m_scaleY;

        curveDatas.insert(key , value);
    }

    //upper
    QMap<qint32 , float> curveUpperDatas;
    QMapIterator<qint32 , float> cud(m_curveUpperDatas);
    while(cud.hasNext())
    {
        cud.next();

        qint32 key = cud.key() * m_scaleX;
        float value = cud.value() * m_scaleY;

        curveUpperDatas.insert(key , value);
    }

    /////////////////////////////////////////////////////////
    QList<qint32> keys = curveDatas.keys();
    QList<float> values = curveDatas.values();
    for(int i = 0 ; i < values.size() - 1 ; i++)
    {
        qint32 currKey = keys.at(i);
        qint32 nextKey = keys.at(i+1);

        float currValue = values.at(i);
        float nextValue = values.at(i+1);

        QPointF currPoint(currKey , currValue);
        QPointF nextPoint(nextKey , nextValue);

        painter.drawLine(currPoint , nextPoint);
    }

    //upper
    keys = curveUpperDatas.keys();
    values = curveUpperDatas.values();
    for(int i = 0 ; i < values.size() - 1 ; i++)
    {
        qint32 currKey = keys.at(i);
        qint32 nextKey = keys.at(i+1);

        float currValue = values.at(i);
        float nextValue = values.at(i+1);

        QPointF currPoint(currKey , currValue);
        QPointF nextPoint(nextKey , nextValue);

        painter.drawLine(currPoint , nextPoint);
    }

    painter.restore();
}

void SimpleCurveGraph::paintTegParamLines(QPainter &painter)
{
    painter.save();

    QPen pen;
    pen.setWidth(1);

    ////////////////////////////////////////////////////////////////////////
    //paint R line
    if(m_rpoint.ttx != 0)
    {
        pen.setColor(QColor(255, 170, 0));
        painter.setPen(pen);

        float x = m_rpoint.ttx * m_scaleX;
        QPointF point1(x , m_topLineValue*m_scaleY);
        QPointF point2(x , m_bottomLineValue*m_scaleY);

        painter.drawLine(point1 , point2);
    }

    //////////////////////////////////////////////////////////////////////////
    //paint K line
    if(m_kpoint.ttx != 0)
    {
        pen.setColor(QColor(0, 85, 0));
        painter.setPen(pen);

        float x = m_kpoint.ttx * m_scaleX;
        QPointF point1(x , m_topLineValue*m_scaleY);
        QPointF point2(x , m_bottomLineValue*m_scaleY);

        painter.drawLine(point1 , point2);
    }

    //////////////////////////////////////////////////////////////////////////
    //paint angle line
    if((m_rpoint.ttx != 0) && (m_kpoint.ttx != 0))
    {
        pen.setColor(QColor(0, 0, 255));
        painter.setPen(pen);

        float diffr = m_rpoint.vvy - m_baseLineValue;
        float diffk = m_kpoint.vvy - m_baseLineValue;

        float x1 = m_rpoint.ttx;
        float y1 = (m_rpoint.vvy - diffr * 2);
        float x2 = m_kpoint.ttx;
        float y2 = (m_kpoint.vvy - diffk * 2);
        float scopy = y2 - y1;
        float scopx = x2 - x1;
        if(!EQUAL_ZERO_FLOAT(scopx))
        {
            float scop = scopy / scopx;
            if(!EQUAL_ZERO_FLOAT(scop))
            {
                float extra = y1 - (x1 * scop);
                float angley = m_topLineValue;
                float anglex = (angley - extra) / scop;

                QPointF point1(x1*m_scaleX , y1*m_scaleY);
                QPointF point2(anglex*m_scaleX , m_topLineValue*m_scaleY);

                painter.drawLine(point1 , point2);

                float piAngle = atan2(point2.y() - point1.y() , point2.x() - point1.x());
                float angle = (piAngle * 180) / 3.14159;

                qDebug() << "====== piangle = " << piAngle;
                qDebug() << "====== angle = " << angle;
            }
        }
    }

    painter.restore();
}
