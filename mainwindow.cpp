#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QLabel>
#include <QString>
#include <QDebug>

// Qlabel - для отображения
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_label(new QLabel(this))
{
    ui->setupUi(this);
    m_label->setWordWrap(true); // перенос строк
    m_label->setAlignment(Qt::AlignTop | Qt::AlignHCenter); // порядок расположения текста
    m_label->setTextInteractionFlags(Qt::TextSelectableByMouse); // выделение текста
    setCentralWidget(m_label);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setResults(const QVector<double>& beamAngles, const QVector<double>& times)
{
    QString text = "Result:\n";
    for (int i = 0; i < beamAngles.size(); ++i) {
        text += QString("Ray %1 (angle %2 deg) → time %3 sec\n")
            .arg(i)
            .arg(beamAngles[i])
            .arg(times[i]);
    }
    m_label->setText(text);
}
