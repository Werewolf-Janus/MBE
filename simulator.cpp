#include "simulator.h"
#include "sceneobject.h"
#include <QFile>
#include <QJsonDocument>
#include <QDebug>
#include <QtMath>

double degToRad(double deg) { return deg * M_PI / 180.0; }

Simulator::Simulator()
{

}

Simulator::~Simulator()
{

}

bool Simulator::loadFromFile(const QString& filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    QByteArray jsonData = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << parseError.errorString();
        return false;
    }
    if (!doc.isObject()) {
        return false;
    }
    QJsonObject root = doc.object();

    QJsonArray speedArr = root["sound_speed_profile"].toArray();
    if (!parseSoundSpeedProfile(speedArr))
        return false;

    QJsonObject mountObj = root["mount_angles_deg"].toObject();

    m_mountRollRad = degToRad(mountObj["roll"].toDouble());
    m_mountPitchRad = degToRad(mountObj["pitch"].toDouble());
    m_mountYawRad = degToRad(mountObj["yaw"].toDouble());

    // Заполнение слова
    if (root.contains("relief") && root["relief"].isObject()) {
        if (!parseRelief(root["relief"].toObject()))
            return false;
    }

    return true;
}

bool Simulator::parseSoundSpeedProfile(const QJsonArray& arr)
{
    m_profile.clear();
    for (const QJsonValue& val : arr) {
        QString str = val.toString();
        QStringList parts = str.split(':');
        if (parts.size() != 2) {
            qWarning() << "Invalid sound speed entry:" << str << "- expected 'depth:speed'";
            continue;
        }
        bool ok1, ok2;
        double depth = parts[0].toDouble(&ok1);
        double speed = parts[1].toDouble(&ok2);
        if (ok1 && ok2) {
            m_profile.push_back({ depth, speed });
        }
    }
    if (m_profile.isEmpty()) {
        return false;
    }
    return true;
}

void Simulator::parseSpheres(const QJsonObject &spheresObj){
    m_sphere.clear();
    for(auto spheres = spheresObj.begin(); spheres != spheresObj.end(); ++spheres){
        QJsonObject s = spheres.value().toObject();
        Sphere sphere;
        sphere.xc = s["xc"].toDouble();
        sphere.yc = s["yc"].toDouble();
        sphere.zc = s["zc"].toDouble();
        sphere.r = s["r"].toDouble();
        m_sphere[spheres.key()] = sphere;
    }
}

void Simulator::parsePlanes(const QJsonObject &planeObj){
    m_plane.clear();
    for(auto planes = planeObj.begin(); planes != planeObj.end(); ++planes){
        QString key = planes.key(); // В отличие от сфер здесь нужно заранее инициализировать ключ, так как будет проверка на дно
        if (key == "bottom") continue;
        QJsonObject p = planes.value().toObject();
        Plane plane;
        plane.x0 = p["x0"].toDouble();
        plane.y0 = p["y0"].toDouble();
        plane.z0 = p["z0"].toDouble();
        plane.nx = p["nx"].toDouble();
        plane.ny = p["ny"].toDouble();
        plane.nz = p["nz"].toDouble();
        m_plane[key] = plane;
    }
}

bool Simulator::parseRelief(const QJsonObject& reliefObj)
{
    m_sumObjects.clear();
    m_diffObjects.clear();
    m_sphere.clear();
    m_plane.clear();
    m_isBottom = false;

    if(reliefObj.contains("plain") && reliefObj["plain"].isObject()){
        QJsonObject plainObj = reliefObj["plain"].toObject();

        // Обработка плоскости дна
        if(plainObj.contains("bottom") && plainObj["bottom"].isObject()){
            QJsonObject b = plainObj["bottom"].toObject();
            m_bottom.x0 = b["x0"].toDouble();
            m_bottom.y0 = b["y0"].toDouble();
            m_bottom.z0 = b["z0"].toDouble();
            m_bottom.nx = b["nx"].toDouble();
            m_bottom.ny = b["ny"].toDouble();
            m_bottom.nz = b["nz"].toDouble();
            m_isBottom = true;
        }
        // Обработка сфер
        if (reliefObj.contains("spheres") && reliefObj["spheres"].isObject()){
            parseSpheres(reliefObj["spheres"].toObject());
        }
        parsePlanes(plainObj);

        // Объекты для суммирования
        if(reliefObj.contains("sum")){
            QJsonArray sum = reliefObj["sum"].toArray();
            for (QJsonValue obj : sum){
                m_sumObjects.append(obj.toString());
            }
        }

        // Объекты для вычитания
        // Обрабатывал отдельно ключ ((имя объекта из которого идет вычитание
        if(reliefObj.contains("diff")){
            QJsonObject diff = reliefObj["diff"].toObject();
            for (auto obj = diff.begin(); obj != diff.end(); ++obj){
                QString diffObj = obj.key();
                QStringList subingObjs;
                QJsonArray arr = obj.value().toArray();
                for (auto val : arr){
                    subingObjs.append(val.toString());
                }
                m_diffObjects[diffObj] = subingObjs;
            }
        }
    }

    return true;

}

SceneObject* Simulator::buildScene()
{
    if (!m_isBottom) return nullptr;
    // Инициализация дна на сцене
    Point pointOnBottom(m_bottom.x0, m_bottom.y0, m_bottom.z0);
    Vector3d normal (m_bottom.nx, m_bottom.ny, m_bottom.nz);
    Plain* plainBottom = new Plain(pointOnBottom, normal, true);

    // Создается QMap для хранения всех объектов
    QMap<QString, SceneObject*> allObjects;
    for (auto spheres = m_sphere.begin(); spheres != m_sphere.end(); ++spheres){
        const Sphere& s = spheres.value(); // Храним в нем все сферы
        allObjects[spheres.key()] = new createSphere(Point(s.xc, s.yc, s.zc), s.r); // Инициализируем сферу через словварь  создается в методе
    }
    for (auto planes = m_plane.begin(); planes != m_plane.end(); ++planes){
        const Plane& p = planes.value(); // Храним в нем все плоскости
        Vector3d norm (p.nx, p.ny, p.nz); // Нормализуем вектора, чтобы при трассировке луча по Закону снеллиуса не уходить в некорректные значения
        allObjects[planes.key()] = new Plain(Point(p.x0, p.y0, p.z0), norm, false); // Инициализирукем плоскости по словарю
    }

    // Добавляем объекты по списку Sum
    for (const QString& name : m_sumObjects){
        if (!allObjects.contains(name)) continue ;

        if (m_diffObjects.contains(name)){
            // Производим инициализацию объекта-обертки для проведения вычитания
            Plain* wrapper = new Plain(Point(0,0,0), Vector3d(0,0,1), false); // Используем нулевые параметры, кроме вектора нормали отвечающего за направление луча в сторону дна
            wrapper->addObject(allObjects[name]);
            const QStringList& subingObjs = m_diffObjects[name]; // Инициализируем вычитаемые объекты
            for (const  QString subName : subingObjs){
                if (allObjects.contains(subName))
                    wrapper->subtractObject(allObjects[subName]);
            }
            plainBottom->addObject(wrapper);
        }
        else {
                plainBottom->addObject(allObjects[name]);
                    }
    }

        // Обработка объектов вне ключа sum
        for (auto objs = m_diffObjects.begin(); objs != m_diffObjects.end(); ++objs){
            const QString& target = objs.key();
            const QStringList& subingObjs = objs.value();

            if (target == "bottom"){
                // Вычитание для дна
                for (const QString& subingName : subingObjs)
                    if (allObjects.contains(subingName)) {
                        plainBottom->subtractObject(allObjects[subingName]);
                    }
            }
            else if (!m_sumObjects.contains(target) && allObjects.contains(target)){ // Если вычитаемый объект не ссумммируемый и не дно
                Plain* wrapper = new Plain(Point(0,0,0), Vector3d(0,0,1), false); // Аналогично для объектов суммирования создаем объект-обертку
                wrapper -> addObject(allObjects[target]);
                for (const QString& subingName :subingObjs){
                    if (allObjects.contains(subingName)) wrapper-> subtractObject(allObjects[subingName]);
                }
                plainBottom->addObject(wrapper);
            }
        }
    return plainBottom;
    }

QVector<double> Simulator::run(
                               double x, double y, double z,
                               double courseDeg, double rollDeg, double pitchDeg,
                               const QVector <double> &beamAnglesDeg
                               )
{
    double courseRad = degToRad(courseDeg);
    double rollRad = degToRad(rollDeg);
    double pitchRad = degToRad(pitchDeg);
    m_beamAnglesDeg = beamAnglesDeg;

    SceneObject* scene = buildScene();


    SoundingCalculator calculator;
    calculator.setScene(scene);
    calculator.setSoundSpeedProfile(m_profile);
    calculator.setMountAngles(m_mountRollRad, m_mountPitchRad, m_mountYawRad);

    QVector<double> beamAnglesRad;
    for (double deg: beamAnglesDeg) beamAnglesRad.append(degToRad(deg));

    return calculator.computeTimes(x, y, z, courseRad, rollRad, pitchRad, beamAnglesRad);

}
