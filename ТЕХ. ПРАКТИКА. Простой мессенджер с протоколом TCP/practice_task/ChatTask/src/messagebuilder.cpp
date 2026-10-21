#include "messagebuilder.h"
#include <QDataStream>

MessageBuilder::MessageBuilder() :
    expectedPackets(0), totalSize(0) {}

void MessageBuilder::processDatagram(const QByteArray &datagram)
{
    // Проверка минимального размера датаграммы (должна содержать заголовок)
    if(datagram.size() < static_cast<int>(sizeof(MessageHeader))) {
        return; // Недостаточно данных для обработки
    }

    QDataStream stream(datagram);
    MessageHeader header;

    // Читаем заголовок из потока данных
    stream >> header.packetSize;
    stream >> header.totalSize;
    stream >> header.index;

    // Если это первый пакет сообщения (индекс 0)
    if(header.index == 0) {
        // Инициализируем сборку нового сообщения
        totalSize = header.totalSize; // Общий размер сообщения
        // Рассчитываем ожидаемое количество пакетов
        expectedPackets = (totalSize + header.packetSize - 1) / header.packetSize;
        fragments.clear(); // Очищаем предыдущие фрагменты
    }

    // Сохраняем фрагмент данных, если индекс в допустимом диапазоне
    if(header.index >= 0 && header.index < expectedPackets) {
        int headerSize = sizeof(MessageHeader);
        // Проверяем, что в датаграмме есть данные после заголовка
        if(datagram.size() > headerSize) {
            // Извлекаем данные после заголовка и сохраняем по индексу
            fragments[header.index] = datagram.mid(headerSize);
        }
    }
}

bool MessageBuilder::isMessageComplete() const
{
    // Проверяем, что собраны все фрагменты сообщения
    return fragments.size() == expectedPackets && expectedPackets > 0;
}

QByteArray MessageBuilder::getMessage() const
{
    QByteArray result; // Результирующее сообщение

    // Собираем фрагменты в правильном порядке
    for(int i = 0; i < expectedPackets; i++) {
        if(fragments.contains(i)) {
            // Добавляем фрагмент к результату
            result.append(fragments[i]);
        }
    }
    return result;
}

void MessageBuilder::reset()
{
    // Сбрасываем состояние сборщика
    fragments.clear();     // Очищаем хранимые фрагменты
    expectedPackets = 0;   // Сбрасываем счетчик пакетов
    totalSize = 0;         // Сбрасываем общий размер
}
