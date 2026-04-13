#include "mainwindow.h"
#include "simulator.h"
#include <QApplication>
#include <QDebug>

// Дно не надо задавать через main, его требуется указать, как объект сцены
int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    double x = 0.0;
    double y = 0.0;
    double z = -2.0;
    double courseDeg = 90.0;
    double rollDeg = 0.0;
    double pitchDeg = 0.0;
    QVector <double> beamAnglesDeg = {-50, -40, -30, -20, -10, 0, 10, 20, 30, 40, 50};
    //
//    QString filename = "/home/labirint/mleImit/MBE/input.json";
//    QString filename = "/home/oper/projectMBE/input.json";
      QString filename = "/home/werewolf/MBE/input.json";
    if (argc > 1)
        filename = argv[1];
    Simulator sim;
    if (!sim.loadFromFile(filename)) {
        qCritical() << "Failed to load configuration from" << filename;
        return 1;
    }

    QVector<double> times = sim.run(x, y, z, courseDeg, rollDeg, pitchDeg, beamAnglesDeg);
    if (times.isEmpty()) {
        qCritical() << "Simulation failed (no times computed)";
        return 1;
    }

    MainWindow w;
    w.setResults(beamAnglesDeg, times);
    w.show();

    return app.exec();
}

