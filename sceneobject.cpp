/*
Деструкторы и методы isInside / intersections для Plain, Sum, Diff.
*/
#include "sceneobject.h"
#include <algorithm>
#include <QtMath>

Plain::Plain(const Point& p, const Vector3d& normal, bool isBase) : p0(p), n(normal), m_isBase(isBase) {
    n.normalize();
}

Plain::~Plain() {
    for (SceneObject* obj : m_added) delete obj;
    for (SceneObject* obj : m_subtracted) delete obj;
}

void Plain::addObject(SceneObject* obj) {
    m_added.push_back(obj);
}

void Plain::subtractObject(SceneObject* obj) {
    m_subtracted.push_back(obj);
}

bool Plain::isInside(const Point& p) const {
    // Принадлежность базовой плоскости
    double dotVal = dot(p-p0, n); // Значение между плоскостью дна и добавленной плоскостью

    bool inBase = m_isBase ? (dotVal <= 0) : (dotVal >= 0); // Если меньше нуля истина, в другом случае ложь

    // Принадлежность хотя бы одному добавленному объекту
    bool inAdded = false;
    for (SceneObject* obj : m_added) {
        if (obj->isInside(p)) {
            inAdded = true;
            break;
        }
    }

    // Принадлежность хотя бы одному вычтенному объекту
    bool inSubtracted = false;
    for (auto obj : m_subtracted) {
        if (obj->isInside(p)) {
            inSubtracted = true;
            break;
        }
    }

    // Результат: (базовая плоскость или добавленный объект) и не вычтенный объект
    return (inBase || inAdded) && !inSubtracted;
}

// Выводить только ближайший до МЛЭ отрезок луча до объекта (с которым столкнулся только один луч)
QVector<double> Plain::intersections(const Ray& ray) const {
    QVector<double> all; // все значения параметра t, соответствующие точкам пересечения луча с объектами сцены

    // Пересечение с базовой плоскостью
    double denom = dot(ray.direction, n);
    if (qAbs(denom) >= 1e-12) {
        double t = dot(p0 - ray.origin, n) / denom;
        if (t >= 0) all.push_back(t);
    }

    // Пересечения со всеми добавленными объектами
    for (SceneObject* obj : m_added) {
        QVector<double> tlist = obj->intersections(ray);
        all.append(tlist);
    }

    // Пересечения со всеми вычтенными объектами
    for (SceneObject* obj : m_subtracted) {
        QVector<double> tlist = obj->intersections(ray);
        all.append(tlist);
    }

    return all;
}

// Добавление объектов на рельеф дна (сфер)
createSphere::createSphere(const Point& center, double radius)
    :m_center(center), m_radius(radius) {}

createSphere::~createSphere(){}

bool createSphere::isInside(const Point &p) const{
    double dx = p.x - m_center.x;
    double dy = p.y - m_center.y;
    double dz = p.z - m_center.z;
    return (dx * dx + dy * dy + dz * dz) <= m_radius * m_radius; // проверка по Евклидову квадрату принадлежности точки сфере
}

QVector<double> createSphere::intersections(const Ray &ray) const{
    //P(t) = P(0) + t * d (для сферы: |P(t) - C| ^2 = R^2
    //ray.origin - P(0) ; ray.direction - d ; m_center - C; m_radius - R
    // Далее приводим к виду квадратного уравнения: a(t^2) + bt + c
    // a - d^2 (нормализованный, поэтому используется метод dot)
    // b - 2 * ((ray.origin - center) * d) так же нормализовано
    // c - |oc| ^ 2 - R^2
    QVector<double> result;
    Vector3d oc = ray.origin - m_center; // луч от начала до середины сферы (origin->center)
    double a = dot(ray.direction, ray.direction);
    double b = 2.0 * dot(oc, ray.direction);
    double c = dot(oc, oc) - m_radius * m_radius;
    double discriminant = b*b - 4*a*c;

    if (discriminant < 0) return result;

    double sqrtD = qSqrt(discriminant);
    double t1 = (-b - sqrtD) / (2*a);
    double t2 = (-b + sqrtD) / (2*a);

    if (t1 >= 0) result.push_back(t1);
    if (t2 >= 0 && t2 != t1) result.push_back(t2);
    return result;
}

