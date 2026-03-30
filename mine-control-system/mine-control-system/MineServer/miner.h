#ifndef MINER_H
#define MINER_H

#include <QRunnable>
#include <QObject>
#include <QThread>
#include <QRandomGenerator>

class Miner : public QObject, public QRunnable {
    Q_OBJECT

public:
    explicit Miner(int id, int duration, double efficiency);
    ~Miner();

    void run() override;

    int getId() const { return m_id; }
    int getDuration() const { return m_duration; }
    double getEfficiency() const { return m_efficiency; }

signals:
    void finished(int metalMined, int minerId);

private:
    int m_id;
    int m_duration;
    double m_efficiency;
};

#endif
