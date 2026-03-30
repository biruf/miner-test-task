#include "mineclient.h"
#include <QHostAddress>
#include <QDebug>
#include <cassert>

MineClient::MineClient(QObject* parent)
    : QObject(parent)
    , m_socket(new QTcpSocket(this))
    , m_manualDisconnect(false) {

    connect(m_socket, &QTcpSocket::connected, this, &MineClient::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &MineClient::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &MineClient::onReadyRead);
    connect(m_socket, &QTcpSocket::errorOccurred, this, &MineClient::onError);

    m_reconnectTimer.setInterval(5000);
    connect(&m_reconnectTimer, &QTimer::timeout, this, &MineClient::attemptReconnect);
}

MineClient::~MineClient() {
    m_manualDisconnect = true;
    m_reconnectTimer.stop();

    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->disconnectFromHost();
        m_socket->waitForDisconnected(2000);
    }
}

void MineClient::connectToServer(const QString& host, quint16 port) {
    m_host = host;
    m_port = port;
    m_manualDisconnect = false;

    qDebug() << "Connecting to server" << host << ":" << port;
    m_socket->connectToHost(host, port);
}

void MineClient::disconnectFromServer() {
    m_manualDisconnect = true;
    m_reconnectTimer.stop();

    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->disconnectFromHost();
        m_socket->waitForDisconnected(2000);
    }
}

bool MineClient::isConnected() const {
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

void MineClient::onConnected() {
    qDebug() << "Connected to server";
    m_reconnectTimer.stop();
    emit connected();
}

void MineClient::onDisconnected() {
    qDebug() << "Disconnected from server";
    emit disconnected();

    if (!m_manualDisconnect) {
        qDebug() << "Starting reconnection attempts...";
        m_reconnectTimer.start();
    }
}

void MineClient::onReadyRead() {
    while (m_socket->bytesAvailable() > 0) {
        QByteArray data = m_socket->readAll();
        Message msg = Message::fromByteArray(data);

        switch (msg.type) {
            case MessageType::STATUS_RESPONSE:
                emit statusReceived(
                    msg.data["total_metal"].toInt(),
                    msg.data["miners_count"].toInt()
                );
                break;

            case MessageType::JOURNAL_EVENTS:
                emit journalEventsReceived(msg.data["events"].toArray());
                break;

            case MessageType::ERROR:
                emit errorReceived(msg.data["error"].toString());
                break;

            default:
                qDebug() << "Unknown message type:" << (int)msg.type;
                break;
        }
    }
}

void MineClient::onError(QAbstractSocket::SocketError socketError) {
    Q_UNUSED(socketError);
    qDebug() << "Socket error:" << m_socket->errorString();
    emit errorReceived(m_socket->errorString());
}

void MineClient::attemptReconnect() {
    if (!m_manualDisconnect && m_socket->state() != QAbstractSocket::ConnectedState) {
        qDebug() << "Attempting to reconnect...";
        m_socket->abort();
        m_socket->connectToHost(m_host, m_port);
    }
}

void MineClient::sendMessage(const Message& msg) {
    assert(m_socket != nullptr);

    if (isConnected()) {
        QByteArray data = msg.toByteArray();
        m_socket->write(data);
        m_socket->flush();
    } else {
        qDebug() << "Cannot send message: not connected";
    }
}

void MineClient::sendStartMiner() {
    sendMessage(Message::createStartMiner());
}

void MineClient::sendGetStatus() {
    sendMessage(Message::createGetStatus());
}

void MineClient::sendSetLoggerType(const QString& type) {
    sendMessage(Message::createSetLoggerType(type));
}
