#ifndef MINESERVER_H
#define MINESERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>
#include <QMutex>
#include <QThreadPool>
#include <QTimer>
#include <memory>
#include "../common/messages.h"

class Miner;
class Logger;

class MineServer : public QTcpServer {
    Q_OBJECT

public:
    explicit MineServer(QObject* parent = nullptr);
    ~MineServer();

    bool start(quint16 port);
    void stop();

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    void onReadyRead();
    void onClientDisconnected();
    void onMinerFinished(int metalMined, int minerId);
    void broadcastJournal();

private:
    void processMessage(QTcpSocket* client, const Message& msg);
    void sendToClient(QTcpSocket* client, const Message& msg);
    void broadcastToClients(const Message& msg);

    QList<QTcpSocket*> m_clients;
    QMutex m_mutex;
    QThreadPool m_threadPool;

    int m_totalMetal;
    int m_nextMinerId;

    std::shared_ptr<Logger> m_logger;
    QTimer* m_journalTimer;
};

#endif
