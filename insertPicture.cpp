#include "mainwindow.h"
#include "ui_mainwindow.h"

using namespace std;
using namespace cv;





void Graphics::insertPicture(Mat &image, const Mat &insertableImage, MyPoint centerForInsertionPoint, bool isGlobalCoordinates, float opacity) {

    int widthOriginal = image.cols;
    int heightOriginal = image.rows;

    int widthInsertable = insertableImage.cols;
    int heightInsertable = insertableImage.rows;

    MyPoint nullPointToInsert; // Нулевая координата для вставки картинки (изначально передаются координаты центра)
    MyPoint localIndent(0, 0); // Отступы используются только если переданы глобальные координаты

    // Если переданы глобальные координаты, то координаты вставки сначала переводятся в локальные
    // координаты (для увеличенных изображений)
    if (isGlobalCoordinates == 1) {
        nullPointToInsert.x = OriginalToResizePoint(centerForInsertionPoint).x - widthInsertable / 2;
        nullPointToInsert.y = OriginalToResizePoint(centerForInsertionPoint).y - heightInsertable / 2;
        localIndent = indent; // Если координаты реальные, а не локальные, то не рисуем на полях
    } else {
        nullPointToInsert.x = centerForInsertionPoint.x - widthInsertable / 2; // Для других картинок
        nullPointToInsert.y = centerForInsertionPoint.y - heightInsertable / 2;
    }
    nullPointToInsert = MyPoint::floor(nullPointToInsert);






    // Два случая, где изображение с альфа-каналом и без
    if (insertableImage.channels() == 3) {

        for (int yOriginal = qMax<int>(nullPointToInsert.y, localIndent.y),
             yInsertable = qMax<int>(0, localIndent.y - nullPointToInsert.y);
             yOriginal < qMin<int>(nullPointToInsert.y + heightInsertable, heightOriginal - localIndent.y);
             yOriginal++, yInsertable++) {

            Vec3b *rowOriginal = image.ptr<Vec3b>(yOriginal);
            const Vec3b *rowInsertable = insertableImage.ptr<Vec3b>(yInsertable);

            for(int xOriginal = qMax<int>(nullPointToInsert.x, localIndent.x),
                xInsertable = qMax<int>(0, localIndent.x - nullPointToInsert.x);
                xOriginal < qMin<int>(nullPointToInsert.x + widthInsertable, widthOriginal - localIndent.x);
                xOriginal++, xInsertable++) {

                    if (opacity == 1)
                        rowOriginal[xOriginal] = rowInsertable[xInsertable];
                    else {
                        // Если нужна частичная прозрачность (opacity от 0 до 1)
                        for (int i = 0; i < 3; i++)
                            rowOriginal[xOriginal][i] = opacity *
                                    (rowInsertable[xInsertable][i] - rowOriginal[xOriginal][i]) +
                                    rowOriginal[xOriginal][i];
                    }
            }
        }

    } else {

        for (int yOriginal = qMax<int>(nullPointToInsert.y, localIndent.y),
             yInsertable = qMax<int>(0, localIndent.y - nullPointToInsert.y);
             yOriginal < qMin<int>(nullPointToInsert.y + heightInsertable, heightOriginal - localIndent.y);
             yOriginal++, yInsertable++) {

            Vec3b *rowOriginal = image.ptr<Vec3b>(yOriginal);
            const Vec4b *rowInsertable = insertableImage.ptr<Vec4b>(yInsertable);

            for(int xOriginal = qMax<int>(nullPointToInsert.x, localIndent.x),
                xInsertable = qMax<int>(0, localIndent.x - nullPointToInsert.x);
                xOriginal < qMin<int>(nullPointToInsert.x + widthInsertable, widthOriginal - localIndent.x);
                xOriginal++, xInsertable++) {

                for (int i = 0; i < 3; i++)
                    rowOriginal[xOriginal][i] = opacity * float(rowInsertable[xInsertable][3]) / 255 *
                            (rowInsertable[xInsertable][i] - rowOriginal[xOriginal][i]) +
                            rowOriginal[xOriginal][i];

            }
        }
    }



    return;
}
