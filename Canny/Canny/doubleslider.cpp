#include "doubleslider.h"
#include "QMouseEvent"
#include "QPainter"
#include "math.h"


DoubleSlider::DoubleSlider(QWidget* parent)
    : QWidget(parent)
    , m_min(0)
    , m_max(99)
    , m_singleStep(1)
    , m_minValue(0)
    , m_maxValue(99)
    , m_state(None)
{
    setFixedHeight(50);
    setFocusPolicy(Qt::StrongFocus);
}

void DoubleSlider::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    paintValueLabel(&painter);
}

void DoubleSlider::paintColoredRect(QRect rect, QColor color ,QPainter* painter)
{
    painter->fillRect(rect,QBrush(color));
}

void DoubleSlider::paintValueLabel(QPainter* painter)
{
    painter->setBrush(Qt::NoBrush);
    painter->setPen(Qt::black);
    painter->setFont(QFont("Arial",8));

    QString minValueString = QString::number(m_minValue);
    QString maxValueString = QString::number(m_maxValue);

    QFontMetrics metrics = painter->fontMetrics();
    int mintextWidth = metrics.width(minValueString);
    int maxtextWidth = metrics.width(maxValueString);
    int textHeight = metrics.height();

    //---- paint text
    painter->drawText(QRectF(4, 12,mintextWidth,textHeight),minValueString);
    painter->drawText(QRectF(width() - maxtextWidth -4, 12, maxtextWidth,textHeight), maxValueString);


    //----- paint label
    painter->setFont(QFont("Arial",8));
    metrics = painter->fontMetrics();
    int labelWidth = metrics.width(m_label);
    int labelHeight = metrics.height();

    QRectF textRect = QRectF((width()/2 - labelWidth/2), 10,labelWidth,labelHeight);

    painter->drawText(textRect,m_label);


    int minPos = ( m_minValue - m_min ) * width() / (m_max - m_min);
    int maxPos = (m_maxValue - m_min ) * width() /  (m_max - m_min);

    if(minPos <= 4){
        minPos = 4;
    }else if(minPos >= width() - 8){
        minPos = width() - 8;
    }else{}

    if(maxPos <= 4){
        maxPos = 4;
    }else if(maxPos >= width() -8){
        maxPos = width() - 8;
    }else{}


    //----- paint groove
    paintColoredRect(QRect(4,37,width() - 8,2),Qt::gray,painter);
    paintColoredRect(QRect(minPos + 4,37,maxPos - minPos,2),QColor(51,153,155),painter);

    //----- handle

    minHandleRegion = QRect(minPos ,30,8,16);
    maxHandleRegion = QRect(maxPos ,30,8,16);

    //-----paint Handle
    QColor minColor  = (m_state == MinHandle) ? QColor(51,153,155) : Qt::darkGray;
    QColor maxColor  = (m_state == MaxHandle) ? QColor(51,153,155) : Qt::darkGray;
    paintColoredRect(minHandleRegion,minColor,painter);
    paintColoredRect(maxHandleRegion,maxColor,painter);
}

inline int getValidValue(int val, int min, int max)
{
    int tmp = std::max(val, min);
    return std::min(tmp, max);
}


void DoubleSlider::setRange(int min, int max)
{
    m_min = min;
    m_max = max;

    if(minValue() < m_min)
        setMinValue(m_min);

    if(maxValue() > m_max){
        setMaxValue(m_max);
    }
}

void DoubleSlider::setSingleStep(int step)
{
    m_singleStep = step;
}

int DoubleSlider::minValue() const
{
    return m_minValue;
}
void DoubleSlider::setMinValue(int val)
{
    if(fabs( m_minValue - val ) > 0.0001){
        m_minValue = val;
        emit boundChanged();
    }
}

int DoubleSlider::maxValue() const
{
    return m_maxValue;
}
void DoubleSlider::setMaxValue(int val)
{
    if(fabs(m_maxValue - val) > 0.0001){
        m_maxValue = val;
        emit boundChanged();
    }
}

QString DoubleSlider::labelText(){
    return m_label;
}
void DoubleSlider::setLabel(const QString& label)
{
   m_label = label;
   update();
}

void DoubleSlider::leaveEvent(QEvent* event)
{
    QWidget::leaveEvent(event);
    m_state = None;
    update();
}

int DoubleSlider::minRange() const
{
    return m_min;
}
int DoubleSlider::maxRange() const
{
    return m_max;
}


void DoubleSlider::mousePressEvent(QMouseEvent* event)
{
    if(minHandleRegion.contains(event->pos())){
        m_state = MinHandle;
    }else if(maxHandleRegion.contains(event->pos())){
        m_state = MaxHandle;
    }else{
        m_state = None;
    }

    if(fabs(minValue() - maxValue()) < 0.001){
        if(fabs(minValue() - m_min) < 0.001){
            m_state = MaxHandle;
        }else if(fabs(minValue() - m_max) < 0.001){
            m_state = MinHandle;
        }else{}
    }

    update();
}

void DoubleSlider::mouseMoveEvent(QMouseEvent* event)
{
    if(event->buttons() & Qt::LeftButton){
        float move = event->x() * (m_max - m_min) * 1.0/ width() + m_min;

        switch(m_state){
            case MinHandle:{
                float val = getValidValue( move, m_min,maxValue());
                setMinValue(val);
                break;
            }

            case MaxHandle:{
                    float val = getValidValue(move, minValue(), m_max);
                    setMaxValue(val);
                    break;
            }
            case None:default: break;
        }
    }
    update();
}


void DoubleSlider::keyPressEvent(QKeyEvent *event)
{
    QWidget::keyPressEvent(event);

    if(event->key() == Qt::Key_Left){
        if(m_state == MinHandle){
            float val = getValidValue(minValue()- m_singleStep, m_min ,maxValue());
            setMinValue(val);
        }else if(m_state == MaxHandle) {
            float val = getValidValue(maxValue()- m_singleStep, minValue() ,m_max);
            setMaxValue(val);
        }else{}
    }else if(event->key() == Qt::Key_Right){
        if(m_state == MinHandle){
            float val = getValidValue(minValue() + m_singleStep, m_min ,maxValue());
            setMinValue(val);
        }else if(m_state == MaxHandle) {
            float val = getValidValue(maxValue() + m_singleStep, minValue() ,m_max);
            setMaxValue(val);
        }else{}
    }

    update();

}
