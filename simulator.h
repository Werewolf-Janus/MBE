#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <QString>
#include <QVector>
#include <QJsonObject>
#include <QJsonArray>
#include <QMap>
#include "soundingcalculator.h"

class SceneObject;

class Simulator
{
public:
    Simulator();
    ~Simulator();

    bool loadFromFile(const QString& filename);

    QVector<double> run(
                        double x, double y, double z,
                        double course, double roll, double pitch,
                        const QVector <double> &beamAnglesDeg
                        );

    QVector<double> getBeamAnglesDeg() const { return m_beamAnglesDeg; }

private:
    QVector<SoundSpeedProfile> m_profile;
    double m_mountRollRad, m_mountPitchRad, m_mountYawRad;
    bool m_isBottom; // являеется ли объект дном

    struct Sphere {
        double xc, yc, zc; // координаты центра сферы
        double r; // радиус
    };

    struct Plane{
        double x0, y0, z0; // начальные координаты
        double nx, ny, nz; // вектора нормали
    };

    Plane m_bottom; // плоскость дна

    // Поменял на QMap, так как надо хранить ключ (вычитаемы или слагаемы объект) и знчение (сам объект рельефа)
    QMap<QString, Sphere> m_sphere;
    QMap<QString, Plane> m_plane;
    QStringList m_sumObjects; // Список имен объектов для сложения
    QMap<QString, QStringList> m_diffObjects; // Поменял на QMap, где ключ - имя объекта из которого идет вычитания и список имен объектов,  которые  вычитаются

    SceneObject* buildScene();

    bool parseSoundSpeedProfile(const QJsonArray& arr);
    void parseSpheres(const QJsonObject& spheresObj);
    void parsePlanes(const QJsonObject& planeObj);
    bool parseRelief(const QJsonObject& reliefObj);


    QVector<double> m_beamAnglesDeg;
};

#endif // SIMULATOR_H
