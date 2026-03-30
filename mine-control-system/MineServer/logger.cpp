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

    m_db = QSqlDatabase::addDatabase("QPSQL", connName);
    m_db.setHostName("localhost");
    m_db.setPort(5432);
    m_db.setDatabaseName("mine_logs");
    m_db.setUserName("mine_user");
    m_db.setPassword("mine_password");

    if (!m_db.open()) {
        qWarning() << "Cannot open database:" << m_db.lastError().text();
        m_dbInitialized = false;
        return;
    }

    QSqlQuery query(m_db);

    QString checkTable = "SELECT EXISTS (SELECT FROM information_schema.tables "
                        "WHERE table_name = 'mine_events')";

    if (query.exec(checkTable) && query.next()) {
        bool tableExists = query.value(0).toBool();

        if (!tableExists) {
            qDebug() << "Creating mine_events table...";

            // Создаем таблицу
            QString createTable = "CREATE TABLE IF NOT EXISTS mine_events ("
                                 "id SERIAL PRIMARY KEY,"
                                 "event_data JSONB NOT NULL,"
                                 "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP)";

            if (!query.exec(createTable)) {
                qWarning() << "Failed to create table:" << query.lastError().text();
                m_dbInitialized = false;
                return;
            }

            // Создаем индекс для ускорения запросов
            query.exec("CREATE INDEX IF NOT EXISTS idx_mine_events_created_at ON mine_events(created_at)");

            qDebug() << "Table mine_events created successfully";
        }
    }

    m_dbInitialized = true;
    qDebug() << "Database logger initialized successfully";
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
