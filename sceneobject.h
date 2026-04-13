/*
Включает geometry.h.

Используется в SoundingCalculator для описания рельефа дна.
*/

#ifndef SCENEOBJECT_H
#define SCENEOBJECT_H

#include "geometry.h"
#include <QVector>

class SceneObject {
public:
    virtual ~SceneObject() = default;
    virtual bool isInside(const Point& p) const = 0;
    virtual QVector<double> intersections(const Ray& ray) const = 0; // Пересечеие с объектом на дне
};

class Plain : public SceneObject {
public:
    Point p0;  // Точка начала координат
    Vector3d n;

    Plain(const Point& p, const Vector3d& normal, bool isBase);
    ~Plain();

    // Добавление объекта (холм, впадина и т.п.) для формирования сложного рельефа
    void addObject(SceneObject* obj);      // объект добавляется к плоскости (объединение)
    void subtractObject(SceneObject* obj); // объект вычитается из плоскости

    bool isInside(const Point& p) const override;
    QVector<double> intersections(const Ray& ray) const override;

private:
   QVector<SceneObject*> m_added;
   QVector<SceneObject*> m_subtracted;
   bool m_isBase; // добавляем флаг наличия плоскости дна (базовой), чтобы можно было сравнивать с добавленными
};

class createSphere : public SceneObject{
public:
    createSphere (const Point& center, double radius);
    ~createSphere() override;

    bool isInside (const Point& p) const override;
    QVector<double> intersections (const Ray& ray) const override;

private:
    Point m_center;
    double m_radius;
};

#endif // SCENEOBJECT_H
