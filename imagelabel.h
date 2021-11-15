#ifndef IMAGELABEL_H
#define IMAGELABEL_H

#include <QLabel>
#include <QObject>
#include <QMouseEvent>
#include <QDebug>
#include "Global.h"



class ImageLabel : public QLabel
{
    Q_OBJECT
    
private:
    MyPoint positionCursor;
    
public:
    ImageLabel(QWidget *parent) : QLabel(parent)
    {
    
    }
    
    MyPoint pos() {
        return positionCursor;
    }
    
    bool leftButton = 0;
    bool rightButton = 0;
    
    bool right = 0;
    bool left = 0;
    
private slots:
    void mouseMoveEvent(QMouseEvent *event)
    {
        setFocus();
        int x = event->pos().x();
        int y = event->pos().y();
        
        (x < 0) ? (positionCursor.x = 0) : ( (x >= width()) ? (positionCursor.x = width() - 1) : (positionCursor.x = x) ); // Граничные условия положения курсора
        (y < 0) ? (positionCursor.y = 0) : ( (y >= height()) ? (positionCursor.y = height() - 1) : (positionCursor.y = y) );
        
    }
    
    void keyPressEvent(QKeyEvent *event) // Нажатие кнопки
    {
        if(event->key() == Qt::Key_Space)
            emit pressSpace();
        if(event->key() == Qt::Key_Right)
            right = 1;
        if(event->key() == Qt::Key_Left)
            left = 1;
    }
    
    void keyReleaseEvent(QKeyEvent *event) // Отжатие кнопки
    {
        if(event->key() == Qt::Key_Right)
            right = 0;
        if(event->key() == Qt::Key_Left)
            left = 0;
    }
    
    void wheelEvent(QWheelEvent* event) // Кручение колёсика
    {
        QPoint Wheel = event->angleDelta() / 8;
        if (Wheel.y() > 0)
            emit wheelSpinsSignal(1);
        if (Wheel.y() < 0)
            emit wheelSpinsSignal(-1);
    }
    
    virtual void mousePressEvent(QMouseEvent *event) // Клик мыши
    {
        if(event->button() == Qt::RightButton)
            rightButton = 1;
        if(event->button() == Qt::LeftButton) {
            leftButton = 1;
            setCursor(Qt::ClosedHandCursor);
        }
    }
    
    virtual void mouseReleaseEvent(QMouseEvent *event) // Обратный клик мыши
    { 
        emit mouseClickEvent();
        if(event->button() == Qt::RightButton)
            rightButton = 0;
        if(event->button() == Qt::LeftButton) {
            leftButton = 0;
            setCursor(Qt::CrossCursor);
        }
    }
    
    void mouseDoubleClickEvent(QMouseEvent *event) // Двойной клик
    { 
        if(event->button() == Qt::LeftButton)
            emit pressLeftButtonDouble();
    }
    
    void resizeEvent(QResizeEvent *)
    {
        emit sigEventResize();
    }
    
signals:
    
    void sigEventResize();
    void pressSpace();
    void pressLeftButtonDouble();
    void mouseClickEvent();
    
    void wheelSpinsSignal(int upOrDown);
    
public slots:
    void setFrameShapeSlot(QFrame::Shape type)
    {
        setFrameShape(type);
    }
    
    void setCursorSlot(const QCursor &cursor)
    {
        setCursor(cursor);
    }
    
};

#endif // IMAGELABEL_H
