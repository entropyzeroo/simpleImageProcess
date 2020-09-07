#ifndef DOUBLESLIDER_H
#define DOUBLESLIDER_H

#include <QMainWindow>

class DoubleSlider : public QWidget
{
    Q_OBJECT
public:
    DoubleSlider(QWidget* parent = 0);
    void setRange(int min, int max);
    void setSingleStep(int step);

    enum State{ MinHandle,MaxHandle,None};

    int minValue() const;
    int maxValue() const;

    int minRange() const;
    int maxRange() const;

    QString labelText();

public slots:
    void setLabel(const QString& label);
    void setMaxValue(int val);
    void setMinValue(int val);

signals:
    void boundChanged();
//    void minValueChanged(QWidget*);
//    void maxValueChanged(QWidget*);

private:
    int m_min;
    int m_max;
    int m_singleStep;

    int m_minValue;
    int m_maxValue;

    QRect minHandleRegion;
    QRect maxHandleRegion;

    State m_state;

    QString m_label;

protected:
    void paintEvent(QPaintEvent* event);
    void paintColoredRect(QRect rect, QColor color, QPainter* painter);
    void paintValueLabel(QPainter* painter);

    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);

    void keyPressEvent(QKeyEvent *event);
    void leaveEvent(QEvent* event);

};

#endif // DOUBLESLIDER_H
