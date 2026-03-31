#include "logger.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <QSqlError>

FileLogger::FileLogger(const QString& filename)
    : m_file(filename) {

    if (!m_file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        qWarning() << "Cannot open log file:" << filename;
    } else {
        qDebug() << "File logger initialized:" << filename;
    }
}

FileLogger::~FileLogger() {
    if (m_file.isOpen()) {
        m_file.close();
    }
}

void FileLogger::addEvent(const QJsonObject& event) {
    QMutexLocker locker(&m_mutex);

    m_cache.append(event);
    if (m_cache.size() > 100) {
        while (m_cache.size() > 100) {
            m_cache.removeAt(0);
        }
    }

    if (m_file.isOpen()) {
        QJsonDocument doc(event);
        QTextStream stream(&m_file);
        stream << doc.toJson(QJsonDocument::Compact) << "\n";
        stream.flush();
    }
}

QJsonArray FileLogger::getLastEvents(int count) {
    QMutexLocker locker(&m_mutex);

    QJsonArray result;
    int start = qMax(0, m_cache.size() - count);

    for (int i = start; i < m_cache.size(); ++i) {
        result.append(m_cache[i]);
    }

    return result;
}

DatabaseLogger::DatabaseLogger(const QString& connectionName)
    : m_connectionName(connectionName)
    , m_dbInitialized(false) {

    initDatabase();
}

DatabaseLogger::~DatabaseLogger() {
    if (m_db.isOpen()) {
        m_db.close();
    }

    QString connection = m_db.connectionName();
    m_db = QSqlDatabase();
    QSqlDatabase::removeDatabase(connection);
}

void DatabaseLogger::initDatabase() {
    static int connectionCounter = 0;
    QString connName = m_connectionName + QString::number(connectionCounter++);

    qDebug() << "Initializing database...";

    QSqlDatabase defaultDb = QSqlDatabase::addDatabase("QPSQL", connName + "_default");
    defaultDb.setHostName("localhost");
    defaultDb.setPort(5432);
    defaultDb.setDatabaseName("postgres");
    defaultDb.setUserName("postgres");
    defaultDb.setPassword("postgres");

    if (!defaultDb.open()) {
        qWarning() << "Cannot connect to default database:" << defaultDb.lastError().text();
        qWarning() << "Make sure PostgreSQL is running: sudo systemctl start postgresql";
        m_dbInitialized = false;
        return;
    }

    qDebug() << "Connected to default database";
    QSqlQuery query(defaultDb);

    bool userExists = false;
    if (query.exec("SELECT 1 FROM pg_roles WHERE rolname = 'mine_user'")) {
        if (query.next()) {
            userExists = true;
            qDebug() << "User mine_user already exists";
        }
    }

    if (!userExists) {
        qDebug() << "Creating user mine_user...";
        if (!query.exec("CREATE USER mine_user WITH PASSWORD 'mine_password'")) {
            qWarning() << "Failed to create user:" << query.lastError().text();
            defaultDb.close();
            m_dbInitialized = false;
            return;
        }
        qDebug() << "User mine_user created successfully";
    }

    bool dbExists = false;
    if (query.exec("SELECT 1 FROM pg_database WHERE datname = 'mine_logs'")) {
        if (query.next()) {
            dbExists = true;
            qDebug() << "Database mine_logs already exists";
        }
    }

    if (!dbExists) {
        qDebug() << "Creating database mine_logs...";
        if (!query.exec("CREATE DATABASE mine_logs OWNER mine_user")) {
            qWarning() << "Failed to create database:" << query.lastError().text();
            defaultDb.close();
            m_dbInitialized = false;
            return;
        }
        qDebug() << "Database mine_logs created successfully";
    }

    defaultDb.close();
    QSqlDatabase::removeDatabase(connName + "_default");

    m_db = QSqlDatabase::addDatabase("QPSQL", connName);
    m_db.setHostName("localhost");
    m_db.setPort(5432);
    m_db.setDatabaseName("mine_logs");
    m_db.setUserName("mine_user");
    m_db.setPassword("mine_password");

    if (!m_db.open()) {
        qWarning() << "Cannot connect to mine_logs database:" << m_db.lastError().text();
        m_dbInitialized = false;
        return;
    }

    qDebug() << "Connected to mine_logs database";

    QSqlQuery tableQuery(m_db);
    QString createTable = "CREATE TABLE IF NOT EXISTS mine_events ("
                         "id SERIAL PRIMARY KEY,"
                         "event_data JSONB NOT NULL,"
                         "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP)";

    if (!tableQuery.exec(createTable)) {
        qWarning() << "Failed to create table:" << tableQuery.lastError().text();
        m_dbInitialized = false;
        return;
    }

    tableQuery.exec("CREATE INDEX IF NOT EXISTS idx_mine_events_created_at ON mine_events(created_at)");

    m_dbInitialized = true;
    qDebug() << "Database logger initialized successfully (auto-created database and user)";
}

void DatabaseLogger::addEvent(const QJsonObject& event) {
    QMutexLocker locker(&m_mutex);

    if (!m_dbInitialized || !m_db.isOpen()) {
        qDebug() << "Database not available, event not logged";
        return;
    }

    QJsonDocument doc(event);
    QString jsonString = doc.toJson(QJsonDocument::Compact);

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO mine_events (event_data) VALUES (:data)");
    query.bindValue(":data", jsonString);

    if (!query.exec()) {
        qWarning() << "Failed to insert event:" << query.lastError().text();
    }
}

QJsonArray DatabaseLogger::getLastEvents(int count) {
    QMutexLocker locker(&m_mutex);

    QJsonArray result;

    if (!m_dbInitialized || !m_db.isOpen()) {
        return result;
    }

    QSqlQuery query(m_db);
    query.prepare("SELECT event_data FROM mine_events ORDER BY id DESC LIMIT :count");
    query.bindValue(":count", count);

    if (query.exec()) {
        while (query.next()) {
            QString jsonString = query.value(0).toString();
            QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8());

            if (doc.isObject()) {
                result.append(doc.object());
            }
        }
    }

    QJsonArray reversed;
    for (int i = result.size() - 1; i >= 0; --i) {
        reversed.append(result[i]);
    }

    return reversed;
}
