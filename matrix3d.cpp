/*
Содержит определения конструкторов, операторов и статических методов класса Matrix3d.

Реализует функции RotationYaw, RotationPitch, RotationRoll.
*/
#include "matrix3d.h"

Matrix3d::Matrix3d() : m_data(3, QVector<double>(3, 0.0)) {}

// добавть методы для ручного обращения к индексам
Matrix3d::Matrix3d(const QVector<QVector<double>>& data) : m_data(data) {}

Matrix3d Matrix3d::zeros() {
    QVector<QVector<double>> data(3, QVector<double>(3, 0.0));
    return Matrix3d(data);
}

Matrix3d Matrix3d::identity() {
    QVector<QVector<double>> data(3, QVector<double>(3,0.0));
    for (int i = 0; i < 3; ++i)
        data[i][i] = 1.0;
    return Matrix3d(data);
}

Matrix3d Matrix3d::operator*(const Matrix3d& other) const {
    QVector<QVector<double>> res(3, QVector<double>(3, 0.0));
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            for (int k = 0; k < 3; ++k) {
                res[i][j] += m_data[i][k] * other.m_data[i][k];
            }
        }
    }
    return Matrix3d(res);
}

Vector3d Matrix3d::operator*(const Vector3d& vec) const {
    return {
        m_data[0][0] * vec.x + m_data[0][1] * vec.y + m_data[0][2] * vec.z,
        m_data[1][0] * vec.x + m_data[1][1] * vec.y + m_data[1][2] * vec.z,
        m_data[2][0] * vec.x + m_data[2][1] * vec.y + m_data[2][2] * vec.z
    };
}

Matrix3d RotationYaw(double yaw) {
    double c = qCos(yaw);
    double s = qSin(yaw);
    QVector<QVector<double>> data ={
                {c, -s, 0},
                {s,  c, 0},
                {0,  0, 1}
        };
    return Matrix3d(data);
}

Matrix3d RotationPitch(double pitch) {
    double c = qCos(pitch);
    double s = qSin(pitch);
    QVector<QVector<double>> data ={
                {c, 0, s},
                {0, 1, 0},
                {-s, 0, c}
    };
    return Matrix3d(data);
}

Matrix3d RotationRoll(double roll) {
    double c = qCos(roll);
    double s = qSin(roll);
    QVector<QVector<double>> data ={
                {1, 0, 0},
                {0, c, -s},
                {0, s, c}
    };
    return Matrix3d(data);
}
