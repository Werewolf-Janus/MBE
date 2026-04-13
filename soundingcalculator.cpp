#include "soundingcalculator.h"
#include <QDebug>
#include <qlogging.h>
#include <QtMath>

SoundingCalculator::SoundingCalculator()
    : m_scene(nullptr),
    m_mountRoll(0),
    m_mountPitch(0),
    m_mountYaw(0)
{
}

SoundingCalculator::~SoundingCalculator()
{
    delete m_scene;
}

void SoundingCalculator::setScene(SceneObject* scene)
{
    m_scene = scene;
}

void SoundingCalculator::setSoundSpeedProfile(QVector<SoundSpeedProfile>& profile)
{
    m_speedProfile = profile;
}

void SoundingCalculator::setMountAngles(double roll, double pitch, double yaw)
{
    m_mountRoll = roll;
    m_mountPitch = pitch;
    m_mountYaw = yaw;
}

double SoundingCalculator::getSoundSpeed(double depth) const
{
    int idx = -1;
    for (int i = 0; i < m_speedProfile.size(); ++i){
        if(m_speedProfile[i].depth <= depth){
            idx = i;
        }
        else break;
    }
    if (idx == -1) return m_speedProfile[0].speed;

    if (idx == m_speedProfile.size() -1) return m_speedProfile.last().speed;

    const SoundSpeedProfile& p0 = m_speedProfile[idx];
    const SoundSpeedProfile& p1 = m_speedProfile[idx +1];
    double t = (depth - p0.depth) / (p1.depth - p0.depth);
    return p0.speed * (1.0 - t) + p1.speed * t;
}

double SoundingCalculator::findFirstIntersection(const Ray& ray) const
{
    if (!m_scene)
        return -1;

    QVector<double> allT = m_scene->intersections(ray);

    std::sort(allT.begin(), allT.end());
    for (double t : allT) {
        // проверка на выход луча за пределы проверяемого объекта
        if (t <= 0) continue;
        Point hit = ray.pointAt(t);
        Point inside = hit + ray.direction * 0.0001; // сравниваем относительно крайне малого шага
        if (m_scene->isInside(inside))
            return t;
    }
    return -1;
}


// Поменять логику вычислений (поменять шаги на временные интервалы,
//так как на расчеты будет уходить много вычислительных ресурсов, при условии,
//что скорость звука расчитывается в профиле
double SoundingCalculator::traceRay(const Point& origin, const Vector3d& dir) const
{
//  double step = 0.5;               // длина шага (м)
    double timeInterval = 0.01; // интервал времени по которому будут производится проверки
    Point p = origin;
    double time = 0.0;

    // Начальная глубина (положительная)
    double depth0 = -p.z;
    double c0 = getSoundSpeed(depth0);

    // Горизонтальная проекция начального направления
    double sinTheta0 = qSqrt(dir.x * dir.x + dir.y * dir.y);
    double p_param = 0.0;                       // горизонтальный медленный параметр (p = sinθ / c)
    double cosPhi = 1.0, sinPhi = 0.0;          // направляющие косинусы азимута
    const double VERTICAL_THRESHOLD = 1e-9;    // порог для избежания деления на ноль

    if (sinTheta0 > VERTICAL_THRESHOLD) {
        p_param = sinTheta0 / c0;
        cosPhi = dir.x / sinTheta0;
        sinPhi = dir.y / sinTheta0;
    }

    while (true) {
        // Текущая глубина
        double depth = -p.z;
        double c = getSoundSpeed(depth);

        // Закон Снеллиуса: sinθ = p_param * c
        double sinTheta = p_param * c;
        if (sinTheta > 1.0) {
            return -1.0;                      // полное внутреннее отражение
        }
        double cosTheta = qSqrt(qMax(0.0, 1.0 - sinTheta * sinTheta));

        // Текущее направление луча (азимут сохраняется)
        Vector3d dir_curr;
        if (sinTheta0 > VERTICAL_THRESHOLD) {
            dir_curr.x = sinTheta * cosPhi;
            dir_curr.y = sinTheta * sinPhi;
        }
        else {
            dir_curr.x = 0.0;
            dir_curr.y = 0.0;
        }
        dir_curr.z = -cosTheta;                // всегда вниз
        dir_curr.normalize();

        double wayInterval = c * timeInterval;

        // Проверяем, не пересекает ли луч дно на отрезке [текущая точка, следующая точка]
        Ray stepRay(p, dir_curr);
        double t_hit = findFirstIntersection(stepRay);

        // Если пересечение найдено и оно в пределах шага
        if (t_hit > 0 && t_hit <= wayInterval) {
            // Точка пересечения
            Point p_hit = p + dir_curr * t_hit;
            // Средняя глубина на отрезке от p до p_hit
            double depth_mid = -(p.z + p_hit.z) * 0.5;
            double c_mid = getSoundSpeed(depth_mid);
            time += t_hit / c_mid;
            return time;
        }
        // Пересечения нет – двигаемся дальше
        time += timeInterval;
        p = p + dir_curr * timeInterval;
    }
    return -1.0;
}

QVector<double> SoundingCalculator::computeTimes(
    double x, double y, double z,
    double course, double roll, double pitch,
    QVector<double>& beamAngles)
{
    QVector<double> times;

    Point origin(x, y, z);

    // Матрица поворота от связанной с носителем системы к мировой
    Matrix3d R_ship = rotationMatrixZYX(course, pitch, roll);
    // Матрица поворота от системы МЛЭ к связанной с носителем
    Matrix3d R_mount = rotationMatrixZYX(m_mountYaw, m_mountPitch, m_mountRoll);

    for (double angle : beamAngles)
    {
        // Локальное направление в системе МЛЭ (ось Z вниз, Y вправо)
        Vector3d dir_local(0, qSin(angle), qCos(angle));
        Vector3d dir_ship = R_mount * dir_local;
        Vector3d dir_world = R_ship * dir_ship;
        dir_world.normalize();
        double time = traceRay(origin, dir_world);
        times.push_back(time);
    }

    return times;
}
