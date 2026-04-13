/*
Включает matrix3d.h.

Определяет тип Point как аналог Vector3d.

Содержит структуру Ray (начальная точка и направление) с методом pointAt.

Объявляет вспомогательные функции: (скалярное произведение), rotationMatrixZYX (повороты для судовой системы).

Используется в модулях сцены и расчетов.
*/
#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "matrix3d.h"

using Point = Vector3d;

inline double dot(const Vector3d& a, const Vector3d& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

struct Ray {
    Point origin; // Точка начала луча
    Vector3d direction; // Вектор направления луча
    Ray(const Point& o, const Vector3d& d) : origin(o), direction(d) {
        direction.normalize();
    }
    Point pointAt(double t) const { return origin + direction * t; }
};

// Повороты: курс (yaw), дифферент (pitch), крен(roll)
inline Matrix3d rotationMatrixZYX(double yaw, double pitch, double roll) {
    return RotationYaw(yaw) * RotationPitch(pitch) * RotationRoll(roll);
}

#endif // GEOMETRY_H
