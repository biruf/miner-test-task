#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include <QTextStream>
#include <QThread>
#include "mineclient.h"

class ConsoleReader : public QThread {
    Q_OBJECT
public:
    explicit ConsoleReader(MineClient* client) : m_client(client) {}

signals:
    void commandEntered(const QString& cmd);

protected:
    void run() override {
        QTextStream qtin(stdin);
        qDebug() << "Консольный ввод активирован. Введите команду:";
        while (true) {
            QString line = qtin.readLine();
            if (line.isNull()) break;
            emit commandEntered(line);
        }
    }

private:
    MineClient* m_client;
};

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    qDebug() << "\n=== Mine Client (Офис управления шахтой) ===";
    qDebug() << "Доступные команды:";
    qDebug() << "  status    - запросить статус шахты";
    qDebug() << "  miner     - нанять нового шахтера";
    qDebug() << "  file      - переключить логгер на ФАЙЛ";
    qDebug() << "  db        - переключить логгер на PostgreSQL";
    qDebug() << "  exit      - выход";
    qDebug() << "==========================================\n";

    MineClient client;
    client.connectToServer("localhost", 12345);

    QTimer statusTimer;
    QObject::connect(&statusTimer, &QTimer::timeout, [&client]() {
        if (client.isConnected()) {
            client.sendGetStatus();
        }
    });
    statusTimer.start(10000);

    QTimer autoMinerTimer;
    QObject::connect(&autoMinerTimer, &QTimer::timeout, [&client]() {
        if (client.isConnected()) {
            qDebug() << "\n[Автоматический наем] Отправляем команду найма шахтера...";
            client.sendStartMiner();
        }
    });
    autoMinerTimer.start(15000);

    // Обработчики сигналов
    QObject::connect(&client, &MineClient::connected, []() {
        qDebug() << "\n✅ Подключен к серверу шахты\n";
    });

    QObject::connect(&client, &MineClient::disconnected, []() {
        qDebug() << "\n❌ Отключен от сервера. Попытка переподключения...\n";
    });

    QObject::connect(&client, &MineClient::statusReceived,
                     [](int totalMetal, int minersCount) {
        qDebug() << "\n=== СТАТУС ШАХТЫ ===";
        qDebug() << "Время:" << QDateTime::currentDateTime().toString("hh:mm:ss");
        qDebug() << "Всего добыто металла:" << totalMetal << "ед.";
        qDebug() << "Активных шахтеров:" << minersCount;
        qDebug() << "====================\n";
    });

    QObject::connect(&client, &MineClient::journalEventsReceived,
                     [](const QJsonArray& events) {
        qDebug() << "\n=== ЖУРНАЛ СОБЫТИЙ (последние" << events.size() << ") ===";
        for (int i = 0; i < events.size(); ++i) {
            QJsonObject event = events[i].toObject();
            QString eventType = event["event"].toString();
            QString timestamp = event["timestamp"].toString();

            if (eventType == "miner_started") {
                qDebug() << "⛏️ [" << timestamp << "] Шахтер" << event["miner_id"].toInt()
                        << "начал работу на" << event["duration"].toInt() << "сек "
                        << "(эффективность:" << event["efficiency"].toDouble() << ")";
            }
            else if (eventType == "miner_finished") {
                qDebug() << "💰 [" << timestamp << "] Шахтер" << event["miner_id"].toInt()
                        << "закончил работу. Добыто:" << event["metal_mined"].toInt()
                        << "металла. Всего добыто:" << event["total_metal"].toInt();
            }
            else {
                qDebug() << i+1 << ":" << QJsonDocument(event).toJson(QJsonDocument::Compact);
            }
        }
        qDebug() << "================================\n";
    });

    QObject::connect(&client, &MineClient::errorReceived,
                     [](const QString& error) {
        qDebug() << "❌ Ошибка:" << error;
    });

    ConsoleReader reader(&client);
    QObject::connect(&reader, &ConsoleReader::commandEntered,
                     [&](const QString& cmd) {
        QString lowerCmd = cmd.toLower();

        if (lowerCmd == "status" || lowerCmd == "ст статус") {
            client.sendGetStatus();
            qDebug() << "📤 Запрос статуса отправлен";
        }
        else if (lowerCmd == "miner" || lowerCmd == "м шахтер") {
            client.sendStartMiner();
            qDebug() << "📤 Команда найма шахтера отправлена";
        }
        else if (lowerCmd == "file" || lowerCmd == "ф файл") {
            client.sendSetLoggerType("file");
            qDebug() << "📤 Команда переключения на ФАЙЛОВЫЙ логгер отправлена";
        }
        else if (lowerCmd == "db" || lowerCmd == "бд") {
            client.sendSetLoggerType("database");
            qDebug() << "📤 Команда переключения на PostgreSQL логгер отправлена";
        }
        else if (lowerCmd == "exit" || lowerCmd == "в выход") {
            qDebug() << "Завершение работы...";
            app.quit();
        }
        else if (!lowerCmd.isEmpty()) {
            qDebug() << "Неизвестная команда. Доступные: status, miner, file, db, exit";
        }
    });

    reader.start();

    qDebug() << "Клиент запущен. Введите команду или ждите автоматических запросов...";
    qDebug() << "Совет: попробуйте ввести 'db' для переключения на PostgreSQL\n";

    return app.exec();
}

#include "main.moc"
