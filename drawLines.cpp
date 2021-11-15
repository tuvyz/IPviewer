#include "graphics.h"

using namespace cv;

void Graphics::drawLines(cv::Mat &image, PairPoint_t linePairPoints, Vec3b lineColor,
                         bool isGlobalCoordinates, bool dottedLine, float opacity)
{
    Size imageSize(image.cols, image.rows);

    MyPoint nullPointKadr;
    MyPoint endPointKadr;

    // Если переданы глобальные координаты, то координаты линии сначала переводятся в локальные
    // координаты и учитываютсяотступы (для увеличенных изображений)
    if (isGlobalCoordinates == true) {
        linePairPoints.first = OriginalToResizePoint(linePairPoints.first);
        linePairPoints.second = OriginalToResizePoint(linePairPoints.second);
        nullPointKadr = indent;
        endPointKadr = MyPoint(imageSize.width - indent.x, imageSize.height - indent.y);
    } else {
        nullPointKadr = MyPoint(0, 0);
        endPointKadr = MyPoint(imageSize.width - 1, imageSize.height - 1);
    }


    linePairPoints.first = MyPoint::floor(linePairPoints.first);
    linePairPoints.second = MyPoint::floor(linePairPoints.second);


    // Если координата линии выходит за границы изображения, то ищется пересечение с границей
    // и принимается как координата вышедшей за границы точки
    {
        // 4 возможных пересения с бесконечно протяжёнными прямыми, проложенными вдоль сторон изображения
        array <MyPoint, 4> borderIntersections {
            MyPoint((nullPointKadr.y - linePairPoints.first.y) * (linePairPoints.second.x - linePairPoints.first.x) / (linePairPoints.second.y - linePairPoints.first.y) + linePairPoints.first.x, nullPointKadr.y),
                    MyPoint((endPointKadr.y - linePairPoints.first.y) * (linePairPoints.second.x - linePairPoints.first.x) / (linePairPoints.second.y - linePairPoints.first.y) + linePairPoints.first.x, endPointKadr.y),
                    MyPoint(nullPointKadr.x, (nullPointKadr.x - linePairPoints.first.x) * (linePairPoints.second.y - linePairPoints.first.y) / (linePairPoints.second.x - linePairPoints.first.x) + linePairPoints.first.y),
                    MyPoint(endPointKadr.x, (endPointKadr.x - linePairPoints.first.x) * (linePairPoints.second.y - linePairPoints.first.y) / (linePairPoints.second.x - linePairPoints.first.x) + linePairPoints.first.y)
        };

        PairPoint_t ImageBoundingPair(nullPointKadr, endPointKadr);

        const PairPoint_t constLineCoordinates = linePairPoints;
        for (auto borderIntersection : borderIntersections) {
            if (!isinf(borderIntersection.x) && !isinf(borderIntersection.y)) { // Проверка на параллельность линии одной из сторон изображения
                if (pointInAreaBetweenPoints(borderIntersection, ImageBoundingPair) && // Проверка на то, что пересечение находится на границе изображения, а не вне него
                        pointInAreaBetweenPoints(borderIntersection, constLineCoordinates)) { // Проверка на принадлежность пересечения линии

                    // Если точка не находится внутри изображения, то ей присваевается точка пересечения с границей картинки
                    if (!pointInAreaBetweenPoints(linePairPoints.first, ImageBoundingPair) || linePairPoints.first == borderIntersection)
                        linePairPoints.first = borderIntersection;
                    else if (!pointInAreaBetweenPoints(linePairPoints.second, ImageBoundingPair))
                        linePairPoints.second = borderIntersection;

                }
            }
        }
        // Если линия не пересекает изображение выходим из функции
        if (!pointInAreaBetweenPoints(linePairPoints.first, ImageBoundingPair) ||
                !pointInAreaBetweenPoints(linePairPoints.second, ImageBoundingPair))
            return;
    }




    // Вычисление итератора для продвижения текущей позиции отрисовки линии (самая протяжённая сторона получает 1, вторая значение от 0 до 1)
    MyPoint iterator;
    if (abs(linePairPoints.second.x - linePairPoints.first.x) > abs(linePairPoints.second.y - linePairPoints.first.y)) {
        (linePairPoints.second.x - linePairPoints.first.x) > 0 ? iterator.x = 1 : iterator.x = -1;
        iterator.y = (linePairPoints.second.y - linePairPoints.first.y) / abs(linePairPoints.second.x - linePairPoints.first.x);
    } else {
        (linePairPoints.second.y - linePairPoints.first.y) > 0 ? iterator.y = 1 : iterator.y = -1;
        iterator.x = (linePairPoints.second.x - linePairPoints.first.x) / abs(linePairPoints.second.y - linePairPoints.first.y);
    }







    int counterDotted = 0;
    MyPoint currentPoint = linePairPoints.first;

    for (int i = 0; i < qMax<double>(abs(linePairPoints.second.x - linePairPoints.first.x), abs(linePairPoints.second.y - linePairPoints.first.y)); i++, currentPoint = currentPoint + iterator) {

        // Счётчик штриховой линии
        (counterDotted == 9) ? (counterDotted = 0) : (counterDotted++);

        if (dottedLine == false || counterDotted <= 6) {
            if (opacity == 1.0f)
                image.at<Vec3b>(currentPoint.y, currentPoint.x) = lineColor;
            else {
                // Если изображение должно быть полупрозрачным
                for (int i = 0; i < 3; i++)
                    image.at<Vec3b>(currentPoint.y, currentPoint.x)[i] = opacity *
                            (lineColor[i] - image.at<Vec3b>(currentPoint.y, currentPoint.x)[i]) +
                            image.at<Vec3b>(currentPoint.y, currentPoint.x)[i];
            }
        }
    }


    return;
}
