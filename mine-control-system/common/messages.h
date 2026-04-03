#ifndef MESSAGES_H
#define MESSAGES_H

#include <QByteArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDateTime>

enum class MessageType {
    UNKNOWN = 0,
    START_MINER = 1,
    GET_STATUS = 2,
    STATUS_RESPONSE = 3,
    SET_LOGGER_TYPE = 4,
    JOURNAL_EVENTS = 5,
    ERROR = 6,
    SUCCESS = 7
};

struct Message {
    MessageType type;
    QJsonObject data;

    Message() : type(MessageType::UNKNOWN) {}

    QByteArray toByteArray() const {
        QJsonObject root;
        root["type"] = static_cast<int>(type);
        root["data"] = data;
        root["timestamp"] = QDateTime::currentMSecsSinceEpoch();

        QJsonDocument doc(root);
        return doc.toJson(QJsonDocument::Compact);
    }

    static Message fromByteArray(const QByteArray& bytes) {
        Message msg;

        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(bytes, &error);

        if (error.error == QJsonParseError::NoError && doc.isObject()) {
            QJsonObject root = doc.object();
            msg.type = static_cast<MessageType>(root["type"].toInt());
            msg.data = root["data"].toObject();
        }

        return msg;
    }

    static Message createStartMiner() {
        Message msg;
        msg.type = MessageType::START_MINER;
        return msg;
    }

    static Message createGetStatus() {
        Message msg;
        msg.type = MessageType::GET_STATUS;
        return msg;
    }

    static Message createStatusResponse(int totalMetal, int minersCount) {
        Message msg;
        msg.type = MessageType::STATUS_RESPONSE;
        msg.data["total_metal"] = totalMetal;
        msg.data["miners_count"] = minersCount;
        return msg;
    }

    static Message createSetLoggerType(const QString& type) {
        Message msg;
        msg.type = MessageType::SET_LOGGER_TYPE;
        msg.data["logger_type"] = type;
        return msg;
    }

    static Message createJournalEvents(const QJsonArray& events) {
        Message msg;
        msg.type = MessageType::JOURNAL_EVENTS;
        msg.data["events"] = events;
        return msg;
    }

    static Message createError(const QString& errorText) {
        Message msg;
        msg.type = MessageType::ERROR;
        msg.data["error"] = errorText;
        return msg;
    }

    static Message createSuccess(const QString& message) {
        Message msg;
        msg.type = MessageType::SUCCESS;
        msg.data["message"] = message;
        return msg;
    }
};

#endif // MESSAGES_H
