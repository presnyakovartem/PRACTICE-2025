#ifndef MESSAGEBUILDER_H
#define MESSAGEBUILDER_H

#include <QObject>
#include <QByteArray>
#include <QMap>

// Класс для сборки сообщений из фрагментированных UDP-пакетов
class MessageBuilder
{
public:
    MessageBuilder();
    void processDatagram(const QByteArray &datagram);  // Обработка входящей датаграммы
    bool isMessageComplete() const;  // Проверка полноты собранного сообщения
    QByteArray getMessage() const;   // Получение собранного сообщения
    void reset();                    // Сброс состояния сборщика

private:
    // Структура заголовка пакета
    struct MessageHeader {
        qint32 packetSize;  // Максимальный размер данных в пакете
        qint32 totalSize;   // Общий размер сообщения
        qint32 index;       // Индекс пакета в последовательности
    };

    // Хранилище фрагментов сообщения (индекс -> данные)
    QMap<qint32, QByteArray> fragments;

    // Состояние сборки
    qint32 expectedPackets = 0;  // Ожидаемое количество пакетов
    qint32 totalSize = 0;        // Общий размер собираемого сообщения
};

#endif // MESSAGEBUILDER_H
