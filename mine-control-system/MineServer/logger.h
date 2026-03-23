#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QTextStream>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMutex>
#include <QDateTime>
#include <memory>

class Logger {
public:
    virtual ~Logger() = default;
    virtual void addEvent(const QJsonObject& event) = 0;
    virtual QJsonArray getLastEvents(int count) = 0;
};

class FileLogger : public Logger {
public:
    explicit FileLogger(const QString& filename = "mine.log");
    ~FileLogger();

    void addEvent(const QJsonObject& event) override;
    QJsonArray getLastEvents(int count) override;

private:
    QFile m_file;
    QMutex m_mutex;
    QJsonArray m_cache;
};

class DatabaseLogger : public Logger {
public:
    explicit DatabaseLogger(const QString& connectionName = "mine_logger");
    ~DatabaseLogger();

    void addEvent(const QJsonObject& event) override;
    QJsonArray getLastEvents(int count) override;

private:
    void initDatabase();

    QSqlDatabase m_db;
    QMutex m_mutex;
    QString m_connectionName;
    bool m_dbInitialized;
};

#endif
