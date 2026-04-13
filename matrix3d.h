/*
Содержит класс Vector3d (структура с координатами x, y, z и основными операциями) и класс Matrix3d (матрица 3×3 с операциями умножения и доступа).

Предоставляет функции создания матриц поворота вокруг осей: RotationYaw, RotationPitch, RotationRoll.

Используется во всех остальных модулях для геометрических вычислений.
*/

#ifndef MATRIX3D_H
#define MATRIX3D_H

#include <QVector>
#include <QtMath>

struct Vector3d {
    double x, y, z;
    Vector3d(double x_ = 0, double y_ = 0, double z_ = 0) : x(x_), y(y_), z(z_) {}
    Vector3d operator+(const Vector3d& other) const { return { x + other.x, y + other.y, z + other.z }; }
    Vector3d operator-(const Vector3d& other) const { return { x - other.x, y - other.y, z - other.z }; }
    Vector3d operator*(double s) const { return { x * s, y * s, z * s }; }
    Vector3d& operator+=(const Vector3d& other) { x += other.x; y += other.y; z += other.z; return *this; }
    double norm() const { return std::sqrt(x * x + y * y + z * z); }
    void normalize() { double n = norm(); if (n > 0) { x /= n; y /= n; z /= n; } }
};

class Matrix3d {
public:
    Matrix3d();
    Matrix3d(const QVector<QVector<double>>& data);
    static Matrix3d zeros();
    static Matrix3d identity();

    Matrix3d operator*(const Matrix3d& other) const;
    Vector3d operator*(const Vector3d& vec) const;

    double& operator()(int i, int j) { return m_data[i][j]; }
    double operator()(int i, int j) const { return m_data[i][j]; }

private:
    QVector<QVector<double>> m_data;
};

Matrix3d RotationYaw(double yaw);   // вокруг оси Z
Matrix3d RotationPitch(double pitch); // вокруг оси Y
Matrix3d RotationRoll(double roll);   // вокруг оси X

#endif // MATRIX3D_H
