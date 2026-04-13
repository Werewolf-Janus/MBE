/*
Включает sceneobject.h.

Содержит класс SoundingCalculator, который управляет сценой, профилем скорости звука, углами установки МЛЭ и выполняет трассировку лучей.

Метод computeTimes принимает координаты МЛЭ, углы носителя, массив углов лучей и возвращает времена пробега сигнала.
*/
#ifndef SOUNDINGCALCULATOR_H
#define SOUNDINGCALCULATOR_H

#include "sceneobject.h"
#include <QVector>

struct SoundSpeedProfile {
    double depth;
    double speed;
};

class SoundingCalculator {
public:
    SoundingCalculator();
    ~SoundingCalculator();

    void setScene(SceneObject* scene);
    void setMountAngles(double roll, double pitch, double yaw);
    void setSoundSpeedProfile(QVector<SoundSpeedProfile>& profile);

    QVector<double> computeTimes(double x, double y, double z,
        double course, double roll, double pitch,
        QVector<double>& beamAngles);

private:
    SceneObject* m_scene;
    QVector<SoundSpeedProfile> m_speedProfile;
    double m_mountRoll, m_mountPitch, m_mountYaw;

    double traceRay(const Point& origin, const Vector3d& dir) const;
    double findFirstIntersection(const Ray& ray) const;
    double getSoundSpeed(double depth) const;
};

#endif // SOUNDINGCALCULATOR_H
