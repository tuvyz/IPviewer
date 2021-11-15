#ifndef KADRGUI_H
#define KADRGUI_H


#include <QDebug>
#include <iostream>
#include "opencv2/opencv.hpp"
#include "Global.h"



class Graphics // Для масштабирования видео, рисования линий и вставки картинок
{

private:

    double currentZoom = 0;

    MyPoint nullPointToZoom; // Локальная координата (0, 0) масштабирования
    MyPoint anchorPointToZoom; // Координата центра масштабирования
    MyPoint anchorPointToMove = MyPoint(-1, -1); // Координата для перемещения изображения курсором
    MyPoint indent; // Отступы от границ
    cv::Size resizeSize;
    cv::Size originalSize;

public:

    MyPoint getIndent() {
        return indent;
    };

    // Преобразование из локальных (масштабных) координат в реальные (работает только если через Resize была пропущена картинка хоть раз)
    MyPoint ResizeToOriginalPoint(MyPoint resizePoint)
    {
        resizePoint = resizePoint - indent;

        if (nullPointToZoom.x == -1 || nullPointToZoom.y == -1) {
            qDebug() << "Невозможно преобразовать координаты: изображение не было инициализировано в KADR_GUI";
            return MyPoint(-1, -1);
        } else {
            MyPoint point = nullPointToZoom + resizePoint / currentZoom;
            point.x = qBound<double>(0, point.x, originalSize.width - 1);
            point.y = qBound<double>(0, point.y, originalSize.height - 1);
            return point;
        }
    }

    // Преобразование из реальных координат в локальные
    MyPoint OriginalToResizePoint(MyPoint originalPoint) {
        if (nullPointToZoom.x == -1 || nullPointToZoom.y == -1) {
            qDebug() << "Невозможно преобразовать координаты: изображение не было инициализировано в KADR_GUI";
            return MyPoint(-1, -1);
        } else
            return (originalPoint - nullPointToZoom) * currentZoom + indent;
    }

    cv::Mat resize(const cv::Mat &originalImage, const cv::Size resizeSize, MyPoint cursorPosition = MyPoint(0, 0), int zoom = 1, bool isMoving = 0);

    void drawLines(cv::Mat &image, PairPoint_t lineCoordinates, cv::Vec3b lineColor = cv::Vec3b(0, 0, 0),
                      bool isGlobalCoordinates = false, bool dottedLine = false, float opacity = 1.0f);

    void insertPicture(cv::Mat &image, const cv::Mat &insertableImage, MyPoint centerForInsertionPoint, bool isGlobalCoordinates = false, float opacity = 1.0f);

};



#endif // KADRGUI_H

















































