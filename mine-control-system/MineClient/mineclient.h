#ifndef MINECLIENT_H
#define MINECLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <memory>
#include "../common/messages.h"

class MineClient : public QObject {
    Q_OBJECT

public:
    explicit MineClient(QObject* parent = nullptr);
    ~MineClient();

    void connectToServer(const QString& host, quint16 port);
    void disconnectFromServer();
    bool isConnected() const;

public slots:
    void sendStartMiner();
    void sendGetStatus();
    void sendSetLoggerType(const QString& type);

signals:
    void connected();
    void disconnected();
    void statusReceived(int totalMetal, int minersCount);
    void journalEventsReceived(const QJsonArray& events);
    void errorReceived(const QString& error);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError socketError);
    void attemptReconnect();

private:
    void sendMessage(const Message& msg);

    QTcpSocket* m_socket;
    QTimer m_reconnectTimer;
    QString m_host;
    quint16 m_port;
    bool m_manualDisconnect;
};

#endif
