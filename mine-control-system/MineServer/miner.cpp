#include "miner.h"
#include <QDebug>

Miner::Miner(int id, int duration, double efficiency)
    : m_id(id)
    , m_duration(duration)
    , m_efficiency(efficiency) {

    setAutoDelete(false);
}

Miner::~Miner() {
}

void Miner::run() {
    qDebug() << "Miner" << m_id << "started working for" << m_duration
             << "seconds with efficiency" << m_efficiency;

    for (int i = 0; i < m_duration; i++) {
        QThread::sleep(1);
        if (i % 3 == 0) {
            qDebug() << "Miner" << m_id << "working..." << i+1 << "/" << m_duration;
        }
    }

    int metalMined = static_cast<int>(100 * m_efficiency * m_duration);

    qDebug() << "Miner" << m_id << "finished. Mined:" << metalMined;

    emit finished(metalMined, m_id);

    deleteLater();
}
