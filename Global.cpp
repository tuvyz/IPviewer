#include "Global.h"


// Проверка находится ли точка в области, ограниченной двумя другими точками
bool pointInAreaBetweenPoints(const MyPoint &point,
                              std::pair <const MyPoint&, const MyPoint&> twoPoints,
                              bool pointCanBeOnBorder) {

    if (pointCanBeOnBorder == 0) {
        return  (
                    (twoPoints.first.x < point.x && point.x < twoPoints.second.x) ||
                    (twoPoints.second.x < point.x && point.x < twoPoints.first.x)
                    ) && (
                    (twoPoints.first.y < point.y && point.y < twoPoints.second.y) ||
                    (twoPoints.second.y < point.y && point.y < twoPoints.first.y)
                    );
    } else {
        return  (
                    (twoPoints.first.x <= point.x && point.x <= twoPoints.second.x) ||
                    (twoPoints.second.x <= point.x && point.x <= twoPoints.first.x)
                    ) && (
                    (twoPoints.first.y <= point.y && point.y <= twoPoints.second.y) ||
                    (twoPoints.second.y <= point.y && point.y <= twoPoints.first.y)
                    );
    }
}


