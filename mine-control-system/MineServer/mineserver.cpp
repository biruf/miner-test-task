#include "mineserver.h"
#include "miner.h"
#include "logger.h"
#include <QHostAddress>
#include <QDebug>
#include <QRandomGenerator>
#include <cassert>

MineServer::MineServer(QObject* parent)
    : QTcpServer(parent)
    , m_totalMetal(0)
    , m_nextMinerId(0) {

    m_threadPool.setMaxThreadCount(10);

    m_logger = std::make_shared<FileLogger>();

    m_journalTimer = new QTimer(this);
    connect(m_journalTimer, &QTimer::timeout, this, &MineServer::broadcastJournal);
    m_journalTimer->start(60000);

    qDebug() << "MineServer created";
}

MineServer::~MineServer() {
    stop();
}

bool MineServer::start(quint16 port) {
    bool result = listen(QHostAddress::Any, port);
    if (result) {
        qDebug() << "Server started on port" << port;
    } else {
        qDebug() << "Failed to start server on port" << port;
    }
    return result;
}

void MineServer::stop() {
    close();
    m_threadPool.waitForDone(3000);

    QMutexLocker locker(&m_mutex);
    for (auto client : m_clients) {
        client->disconnectFromHost();
        client->deleteLater();
    }
    m_clients.clear();
}

void MineServer::incomingConnection(qintptr socketDescriptor) {
    QTcpSocket* client = new QTcpSocket(this);
    assert(client != nullptr);

    if (!client->setSocketDescriptor(socketDescriptor)) {
        delete client;
        return;
    }

    connect(client, &QTcpSocket::readyRead, this, &MineServer::onReadyRead);
    connect(client, &QTcpSocket::disconnected, this, &MineServer::onClientDisconnected);

    QMutexLocker locker(&m_mutex);
    m_clients.append(client);

    qDebug() << "New client connected from" << client->peerAddress().toString();
}

void MineServer::onReadyRead() {
    QTcpSocket* client = qobject_cast<QTcpSocket*>(sender());
    assert(client != nullptr);

    while (client->bytesAvailable() > 0) {
        QByteArray data = client->readAll();
        Message msg = Message::fromByteArray(data);

        if (msg.type != MessageType::UNKNOWN) {
            processMessage(client, msg);
        }
    }
}

void MineServer::onClientDisconnected() {
    QTcpSocket* client = qobject_cast<QTcpSocket*>(sender());
    assert(client != nullptr);

    QMutexLocker locker(&m_mutex);
    m_clients.removeOne(client);
    client->deleteLater();

    qDebug() << "Client disconnected";
}

void MineServer::processMessage(QTcpSocket* client, const Message& msg) {
    assert(client != nullptr);

    switch (msg.type) {
        case MessageType::START_MINER: {
            int id = m_nextMinerId++;
            int duration = QRandomGenerator::global()->bounded(3, 11);
            double efficiency = QRandomGenerator::global()->generateDouble();

            Miner* miner = new Miner(id, duration, efficiency);
            connect(miner, &Miner::finished, this, &MineServer::onMinerFinished);

            QJsonObject startEvent;
            startEvent["event"] = "miner_started";
            startEvent["miner_id"] = id;
            startEvent["duration"] = duration;
            startEvent["efficiency"] = efficiency;
            startEvent["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
            m_logger->addEvent(startEvent);

            m_threadPool.start(miner);

            qDebug() << "Started miner" << id << "duration:" << duration << "efficiency:" << efficiency;
            break;
        }

        case MessageType::GET_STATUS: {
            Message response = Message::createStatusResponse(m_totalMetal, m_threadPool.activeThreadCount());
            sendToClient(client, response);
            qDebug() << "Status sent: totalMetal =" << m_totalMetal << "activeMiners =" << m_threadPool.activeThreadCount();
            break;
        }

       case MessageType::SET_LOGGER_TYPE: {
          QString loggerType = msg.data["logger_type"].toString();
            if (loggerType == "database") {
                m_logger = std::make_shared<DatabaseLogger>();
                 // Отправляем как ERROR вместо SUCCESS
                     sendToClient(client, Message::createError("Logger switched to database"));
                      } else {
                       m_logger = std::make_shared<FileLogger>();
                         sendToClient(client, Message::createError("Logger switched to file"));
                          }
                          break;
         }

        default:
            sendToClient(client, Message::createError("Unknown message type"));
            break;
    }
}

void MineServer::onMinerFinished(int metalMined, int minerId) {
    {
        QMutexLocker locker(&m_mutex);
        m_totalMetal += metalMined;
    }

    QJsonObject finishEvent;
    finishEvent["event"] = "miner_finished";
    finishEvent["miner_id"] = minerId;
    finishEvent["metal_mined"] = metalMined;
    finishEvent["total_metal"] = m_totalMetal;
    finishEvent["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    m_logger->addEvent(finishEvent);

    qDebug() << "Miner" << minerId << "finished. Mined:" << metalMined << "Total:" << m_totalMetal;
}

void MineServer::broadcastJournal() {
    QJsonArray events = m_logger->getLastEvents(10);
    if (!events.isEmpty()) {
        broadcastToClients(Message::createJournalEvents(events));
        qDebug() << "Broadcasted" << events.size() << "journal events";
    }
}

void MineServer::sendToClient(QTcpSocket* client, const Message& msg) {
    assert(client != nullptr);

    if (client->state() == QAbstractSocket::ConnectedState) {
        QByteArray data = msg.toByteArray();
        client->write(data);
        client->flush();
    }
}

void MineServer::broadcastToClients(const Message& msg) {
    QMutexLocker locker(&m_mutex);

    QByteArray data = msg.toByteArray();
    for (auto client : m_clients) {
        if (client->state() == QAbstractSocket::ConnectedState) {
            client->write(data);
            client->flush();
        }
    }
}
