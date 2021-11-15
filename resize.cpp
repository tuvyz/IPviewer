#include "graphics.h"

using namespace std;
using namespace cv;





Mat Graphics::resize(const Mat &originalImage, const cv::Size resizeSize, MyPoint cursorPosition, int zoom, bool isMoving) {

    this->resizeSize = resizeSize;
    originalSize.height = originalImage.rows;
    originalSize.width = originalImage.cols;
    
    if (originalSize.height == 0 || originalSize.width == 0)
        return Mat(1, 1, CV_8UC3);

    // ПЕРЕРАСЧЁТ ЗУМА И РАСЧЁТ НУЛЕВОЙ КООРДИНАТЫ ДЛЯ СКЕЙЛА
    double lastZoom = currentZoom;
    double ratioHeight = (double)resizeSize.height / originalSize.height;
    double ratioWidth = (double)resizeSize.width / originalSize.width;

    // Если уровень зума равен 1, то изображение вписывается в заданные рамки 1:1. Для этого
    // расчитывается реальный уровень зума (с плавающей запятой) и размер полос по краям
    if (zoom == 1) {

        currentZoom = qMin<double>(ratioHeight, ratioWidth);
        if (ratioHeight == ratioWidth)
            indent = MyPoint(0, 0);
        else
            indent = MyPoint(resizeSize.width - currentZoom * originalSize.width, resizeSize.height - currentZoom * originalSize.height) / 2;

        indent = MyPoint::ceil(indent);
        nullPointToZoom = MyPoint(0, 0);

    } else {

        // Если уровень зума настолько низок, что не позволяет растянуть изображение на заданный
        // размер, то увеличиваем его во избежание полос по краям
        currentZoom = qMax<double>(qMax<double>(zoom, ratioWidth), ratioHeight);

        if (lastZoom != currentZoom) {

            // Расчёт нулевой координаты, (той, с которой начинается цикл для оригинального изображения)
            nullPointToZoom = anchorPointToZoom - cursorPosition / currentZoom;
            nullPointToZoom.x = qBound<double>(0, nullPointToZoom.x, originalSize.width - resizeSize.width / currentZoom);
            nullPointToZoom.y = qBound<double>(0, nullPointToZoom.y, originalSize.height - resizeSize.height / currentZoom);

            indent = MyPoint(0, 0);
        }
    }

    // Это та координата (в координатной системе изображения), на которую указывает курсор
    // при приближении. Если курсор не двигается, то эта координата так же не должна меняться
    anchorPointToZoom = nullPointToZoom + cursorPosition / currentZoom - indent / lastZoom;


    // Перемещение изображения
    if (isMoving) {
        if (anchorPointToMove == MyPoint(-1, -1))
            anchorPointToMove = anchorPointToZoom;

        nullPointToZoom.x = qBound<double>(0, nullPointToZoom.x + anchorPointToMove.x - anchorPointToZoom.x, originalSize.width - resizeSize.width / currentZoom);
        nullPointToZoom.y = qBound<double>(0, nullPointToZoom.y + anchorPointToMove.y - anchorPointToZoom.y, originalSize.height - resizeSize.height / currentZoom );
    }
    else
        anchorPointToMove = MyPoint(-1, -1);







    // ФОРМИРОВАНИЕ НОВОГО ИЗОБРАЖЕНИЯ
    double iterator = 1.0 / currentZoom;
    MyPoint currentOriginalKadrPoint = nullPointToZoom;

    // Два случая, где изображение с альфа-каналом и без
    if (originalImage.channels() == 3) {

        Mat resizeImage(resizeSize.height, resizeSize.width, CV_8UC3, Scalar(0));
        for (int y = indent.y; y < resizeSize.height - indent.y; y++) {

            Vec3b *rowKadrResize = resizeImage.ptr<Vec3b>(y);
            const Vec3b *rowKadrOriginal = originalImage.ptr<Vec3b>(int(currentOriginalKadrPoint.y));

            for (int x = indent.x; x < resizeSize.width - indent.x; x++) {

                rowKadrResize[x] = rowKadrOriginal[int(currentOriginalKadrPoint.x)];
                currentOriginalKadrPoint.x += iterator;

            }

            currentOriginalKadrPoint.x = nullPointToZoom.x;
            currentOriginalKadrPoint.y += iterator;
        }

        return resizeImage;

    } else {

        Mat resizeImage(resizeSize.height, resizeSize.width, CV_8UC4, Scalar(0));
        for (int y = indent.y; y < resizeSize.height - indent.y; y++) {

            Vec4b *rowKadrResize = resizeImage.ptr<Vec4b>(y);
            const Vec4b *rowKadrOriginal = originalImage.ptr<Vec4b>(int(currentOriginalKadrPoint.y));

            for (int x = indent.x; x < resizeSize.width - indent.x; x++) {

                rowKadrResize[x] = rowKadrOriginal[int(currentOriginalKadrPoint.x)];
                currentOriginalKadrPoint.x += iterator;

            }

            currentOriginalKadrPoint.x = nullPointToZoom.x;
            currentOriginalKadrPoint.y += iterator;

        }

        return resizeImage;
    }
}
