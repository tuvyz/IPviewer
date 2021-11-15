#ifndef GLOBAL_H
#define GLOBAL_H

#include <chrono>
#include <QDebug>
#include <QtGui>



using namespace std;



static mutex Locker;
#define LOCK Locker.lock();
#define UNLOCK Locker.unlock();






class MyTime {
private:
    chrono::time_point<chrono::steady_clock> begin = chrono::steady_clock::now();
public:
    MyTime () {

    }
    MyTime (uint8_t measurementNumber1or2, bool output2Console = 1) {
        operator ()(measurementNumber1or2, output2Console);
    }
    double operator ()(uint8_t measurementNumber1or2, bool output2Console = 1) {
        switch (measurementNumber1or2) {
        case 1: {
            begin = chrono::steady_clock::now(); // Первый замер времени
            return 1;
        }
        case 2: {
            auto end = chrono::steady_clock::now(); // Второй замер времени
            auto elapsed_mc = chrono::duration_cast
                    <chrono::microseconds>(end - begin); // Разница в микросекундах;
            double delay = elapsed_mc.count();

            if (output2Console == 1) { // Вывод в консоль по-умолчанию
                if (delay >= 1000000)
                    qDebug() << delay / 1000000 << "секунд";
                if (delay >= 1000 && delay < 1000000)
                    qDebug() << delay / 1000 << "миллисекунд";
                if (delay < 1000)
                    qDebug() << delay << "микросекунд";
            }
            return delay;
        }
        default:
            return 0;
        }
    }
    static void sleep(int delayInMicroseconds, bool processEvents = 0)
    {
        if (delayInMicroseconds <= 0)
            return;
        MyTime time(1);
        while (time(2, false) < delayInMicroseconds) {
            if (processEvents == 1)
                qApp->processEvents();
        }
    }
    static void sleepWhile(const std::function<bool()> &predicate,  bool processEvents = 0) {
        while (predicate() == true) {
            if (processEvents == 1)
                qApp->processEvents();
        }
    }
};






struct MyPoint
{
    MyPoint(){};
    MyPoint(double x, double y) {
        this->x = x, this->y = y;
    }
    MyPoint operator - (const MyPoint &point) {
        return MyPoint(x - point.x, y - point.y);
    }
    MyPoint operator + (const MyPoint &point) {
        return MyPoint(x + point.x, y + point.y);
    }
    bool operator == (const MyPoint &point) {
        return x == point.x && y == point.y;
    }
    bool operator != (const MyPoint &point) {
        return x != point.x || y != point.y;
    }

    MyPoint operator + (double numeric) {
        return MyPoint(x + numeric, y + numeric);
    }
    MyPoint operator - (double numeric) {
        return MyPoint(x - numeric, y - numeric);
    }
    MyPoint operator * (double numeric) {
        return MyPoint(x * numeric, y * numeric);
    }
    MyPoint operator / (double numeric) {
        return MyPoint(x / numeric, y / numeric);
    }

    void print() {
        qDebug() << "x =" << x << "| y =" << y;
    }
    
    static MyPoint floor(MyPoint point) {
        return MyPoint(std::floor(point.x), std::floor(point.y));
    }
    static MyPoint ceil(MyPoint point) {
        return MyPoint(std::ceil(point.x), std::ceil(point.y));
    }
    static MyPoint round(MyPoint point) {
        return MyPoint(std::round(point.x), std::round(point.y));
    }

    double x = 0;
    double y = 0;
};







typedef std::pair<MyPoint, MyPoint> PairPoint_t;

// Проверка находится ли точка в области, ограниченной двумя другими точками
bool pointInAreaBetweenPoints(const MyPoint &point,
                              std::pair <const MyPoint&, const MyPoint&> twoPoints,
                              bool pointCanBeOnBorder = 1);








#endif // GLOBAL_H
