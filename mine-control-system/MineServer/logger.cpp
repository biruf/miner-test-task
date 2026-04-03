#include "logger.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <QSqlError>
#include <QThread>

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

bool DatabaseLogger::testConnection(QSqlDatabase& db, const QString& user, const QString& password) {
    db.setUserName(user);
    db.setPassword(password);

    if (db.open()) {
        return true;
    }

    db.setPassword("");
    if (db.open()) {
        return true;
    }

    return false;
}

void DatabaseLogger::initDatabase() {
    static int connectionCounter = 0;

    qDebug() << "========================================";
    qDebug() << "Initializing PostgreSQL database...";
    qDebug() << "========================================";

    struct UserAttempt {
        QString username;
        QString password;
        QString description;
    };

    QList<UserAttempt> attempts;
    attempts << UserAttempt{"postgres", "", "postgres (no password)"}
             << UserAttempt{"postgres", "postgres", "postgres (password: postgres)"}
             << UserAttempt{QLatin1String(qgetenv("USER")), "", "current user"}
             << UserAttempt{"mine_user", "mine_password", "mine_user"};

    QSqlDatabase defaultDb;
    bool connected = false;
    QString connectedUser;
    QString tempConnName;

    for (const auto& attempt : attempts) {
        tempConnName = "temp_" + QString::number(connectionCounter++);
        defaultDb = QSqlDatabase::addDatabase("QPSQL", tempConnName);
        defaultDb.setHostName("localhost");
        defaultDb.setPort(5432);
        defaultDb.setDatabaseName("postgres");
        defaultDb.setUserName(attempt.username);
        defaultDb.setPassword(attempt.password);

        if (defaultDb.open()) {
            qDebug() << "✓ Connected to PostgreSQL as:" << attempt.username;
            connected = true;
            connectedUser = attempt.username;
            break;
        } else {
            defaultDb.close();
            QSqlDatabase::removeDatabase(tempConnName);
            defaultDb = QSqlDatabase();
        }
    }

    if (!connected) {
        qWarning() << "✗ Cannot connect to PostgreSQL";
        m_dbInitialized = false;
        return;
    }

    {
        QSqlQuery query(defaultDb);

        bool userExists = false;
        if (query.exec("SELECT 1 FROM pg_roles WHERE rolname = 'mine_user'")) {
            if (query.next()) {
                userExists = true;
                qDebug() << "✓ User 'mine_user' already exists";
            }
        }

        if (!userExists) {
            qDebug() << "→ Creating user 'mine_user'...";
            QStringList createCommands;
            createCommands << "CREATE USER mine_user WITH PASSWORD 'mine_password'"
                          << "CREATE USER mine_user WITH PASSWORD 'mine_password' SUPERUSER"
                          << "CREATE USER mine_user";

            for (const QString& cmd : createCommands) {
                if (query.exec(cmd)) {
                    qDebug() << "✓ User 'mine_user' created successfully";
                    break;
                }
            }
        }

        query.exec("ALTER USER mine_user WITH SUPERUSER");

        bool dbExists = false;
        if (query.exec("SELECT 1 FROM pg_database WHERE datname = 'mine_logs'")) {
            if (query.next()) {
                dbExists = true;
                qDebug() << "✓ Database 'mine_logs' already exists";
            }
        }

        if (!dbExists) {
            qDebug() << "→ Creating database 'mine_logs'...";
            QString createDb = QString("CREATE DATABASE mine_logs OWNER %1")
                              .arg(userExists ? "mine_user" : "postgres");

            if (query.exec(createDb)) {
                qDebug() << "✓ Database 'mine_logs' created successfully";
            }
        }
    }

    defaultDb.close();
    QSqlDatabase::removeDatabase(tempConnName);

    QString connName = "main_" + QString::number(connectionCounter++);
    m_db = QSqlDatabase::addDatabase("QPSQL", connName);
    m_db.setHostName("localhost");
    m_db.setPort(5432);
    m_db.setDatabaseName("mine_logs");

    QList<UserAttempt> dbAttempts;
    dbAttempts << UserAttempt{"mine_user", "mine_password", "mine_user with password"}
               << UserAttempt{"mine_user", "", "mine_user without password"}
               << UserAttempt{"postgres", "", "postgres without password"}
               << UserAttempt{"postgres", "postgres", "postgres with password"};

    connected = false;
    for (const auto& attempt : dbAttempts) {
        m_db.setUserName(attempt.username);
        m_db.setPassword(attempt.password);

        if (m_db.open()) {
            connected = true;
            qDebug() << "✓ Connected to mine_logs as:" << attempt.username;
            break;
        }
    }

    if (!connected) {
        qWarning() << "✗ Cannot connect to mine_logs database:" << m_db.lastError().text();
        m_dbInitialized = false;
        return;
    }

    {
        QSqlQuery tableQuery(m_db);
        QString createTable = "CREATE TABLE IF NOT EXISTS mine_events ("
                             "id SERIAL PRIMARY KEY,"
                             "event_data JSONB NOT NULL,"
                             "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP)";

        if (!tableQuery.exec(createTable)) {
            qWarning() << "✗ Failed to create table:" << tableQuery.lastError().text();
            m_dbInitialized = false;
            return;
        }

        tableQuery.exec("CREATE INDEX IF NOT EXISTS idx_mine_events_created_at ON mine_events(created_at)");
    }

    m_dbInitialized = true;

    qDebug() << "========================================";
    qDebug() << "✅ DATABASE LOGGER INITIALIZED SUCCESSFULLY!";
    qDebug() << "   Database: mine_logs";
    qDebug() << "   Table: mine_events";
    qDebug() << "   User: mine_user";
    qDebug() << "   Host: localhost:5432";
    qDebug() << "========================================";
}

void DatabaseLogger::addEvent(const QJsonObject& event) {
    QMutexLocker locker(&m_mutex);

    if (!m_dbInitialized || !m_db.isOpen()) {
        static FileLogger fallbackLogger("database_fallback.log");
        fallbackLogger.addEvent(event);
        return;
    }

    QJsonDocument doc(event);
    QString jsonString = doc.toJson(QJsonDocument::Compact);

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO mine_events (event_data) VALUES (:data)");
    query.bindValue(":data", jsonString);

    if (!query.exec()) {
        static FileLogger fallbackLogger("database_fallback.log");
        fallbackLogger.addEvent(event);
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
