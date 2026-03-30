#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include <QTextStream>
#include <QSocketNotifier>
#include "mineclient.h"

MineClient* g_client = nullptr;

// Функция для вывода списка команд
void showHelp() {
    qDebug() << "\n╔════════════════════════════════════════════════════╗";
    qDebug() << "║  Доступные команды:                                ║";
    qDebug() << "║  status, s    - запросить статус шахты             ║";
    qDebug() << "║  miner, m     - нанять нового шахтера              ║";
    qDebug() << "║  file, f      - переключить логгер на ФАЙЛ         ║";
    qDebug() << "║  db, d        - переключить логгер на PostgreSQL   ║";
    qDebug() << "║  help, h, ?   - показать эту справку               ║";
    qDebug() << "║  exit, q      - выход                              ║";
    qDebug() << "╚════════════════════════════════════════════════════╝";
    qDebug() << "> " << Qt::endl;
}

// Функция обработки ввода
void handleConsoleInput() {
    QTextStream qtin(stdin);
    if (qtin.atEnd()) return;

    QString line = qtin.readLine();
    if (line.isEmpty()) return;

    QString cmd = line.trimmed().toLower();

    if (cmd == "status" || cmd == "s") {
        if (g_client) g_client->sendGetStatus();
        qDebug() << "📤 Запрос статуса отправлен";
    }
    else if (cmd == "miner" || cmd == "m") {
        if (g_client) g_client->sendStartMiner();
        qDebug() << "📤 Команда найма шахтера отправлена";
    }
    else if (cmd == "file" || cmd == "f") {
        if (g_client) g_client->sendSetLoggerType("file");
        qDebug() << "📤 Переключение на ФАЙЛОВЫЙ логгер";
    }
    else if (cmd == "db" || cmd == "d") {
        if (g_client) g_client->sendSetLoggerType("database");
        qDebug() << "📤 Переключение на PostgreSQL логгер";
    }
    else if (cmd == "help" || cmd == "h" || cmd == "?") {
        showHelp();
    }
    else if (cmd == "exit" || cmd == "q" || cmd == "quit") {
        qDebug() << "Завершение работы...";
        QCoreApplication::quit();
    }
    else if (!cmd.isEmpty()) {
        qDebug() << "❌ Неизвестная команда: " << cmd;
        showHelp();
    }
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    qDebug() << "\n╔════════════════════════════════════════════════════╗";
    qDebug() << "║     Mine Client (Офис управления шахтой)           ║";
    qDebug() << "╚════════════════════════════════════════════════════╝";

    // Показываем команды при запуске
    showHelp();

    MineClient client;
    g_client = &client;

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
            client.sendStartMiner();
        }
    });
    autoMinerTimer.start(15000);

    // Обработчики сигналов
    QObject::connect(&client, &MineClient::connected, []() {
        qDebug() << "\n✅ ПОДКЛЮЧЕНО к серверу шахты";
        showHelp();
    });

    QObject::connect(&client, &MineClient::disconnected, []() {
        qDebug() << "\n❌ ОТКЛЮЧЕНО от сервера";
        showHelp();
    });

    QObject::connect(&client, &MineClient::statusReceived,
                     [](int totalMetal, int minersCount) {
        qDebug() << "\n╔════════════════════════════════════════╗";
        qDebug() << "║            СТАТУС ШАХТЫ                ║";
        qDebug() << "╠════════════════════════════════════════╣";
        qDebug() << "║ Время: " << QDateTime::currentDateTime().toString("hh:mm:ss");
        qDebug() << "║ Всего добыто металла: " << totalMetal << " ед.";
        qDebug() << "║ Активных шахтеров: " << minersCount;
        qDebug() << "╚════════════════════════════════════════╝";
        // Показываем команды после статуса
        showHelp();
    });

    QObject::connect(&client, &MineClient::journalEventsReceived,
                     [](const QJsonArray& events) {
        qDebug() << "\n┌────────────────────────────────────────┐";
        qDebug() << "│       ЖУРНАЛ СОБЫТИЙ (последние " << events.size() << ")    │";
        qDebug() << "├────────────────────────────────────────┤";

        for (int i = 0; i < events.size(); ++i) {
            QJsonObject event = events[i].toObject();
            QString eventType = event["event"].toString();

            if (eventType == "miner_started") {
                qDebug() << "│ ⛏️ Шахтер" << QString("%1").arg(event["miner_id"].toInt(), 2)
                        << "начал работу (" << event["duration"].toInt() << " сек)";
            }
            else if (eventType == "miner_finished") {
                qDebug() << "│ 💰 Шахтер" << QString("%1").arg(event["miner_id"].toInt(), 2)
                        << "закончил → +" << QString("%1").arg(event["metal_mined"].toInt(), 4)
                        << " металла";
            }
        }
        qDebug() << "└────────────────────────────────────────┘";
        showHelp();
    });

    QObject::connect(&client, &MineClient::errorReceived,
                     [](const QString& error) {
        qDebug() << "⚠️ Ошибка: " << error;
        showHelp();
    });

    // Настраиваем неблокирующий ввод с консоли
    QSocketNotifier notifier(fileno(stdin), QSocketNotifier::Read);
    QObject::connect(&notifier, &QSocketNotifier::activated, [](int) {
        handleConsoleInput();
    });

    qDebug() << "💡 Вводите команды в любой момент (например: status, miner, db)";
    qDebug() << "💡 Статус обновляется автоматически каждые 10 секунд\n";
    qDebug() << "> " << Qt::endl;

    return app.exec();
}
