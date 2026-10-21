#include "udpchatprotocol.h"
#include "messagebuilder.h"
#include <QDebug>

UdpChatProtocol::UdpChatProtocol(QObject *parent) :
    QObject(parent), socket(new QUdpSocket(this)), destPort(0)
{
    // Подключаем сигнал готовности чтения к обработчику
    connect(socket, &QUdpSocket::readyRead,
            this, &UdpChatProtocol::readPendingDatagrams);
}

UdpChatProtocol::~UdpChatProtocol()
{
    // Закрываем сокет при уничтожении объекта
    socket->close();
}

bool UdpChatProtocol::isBound() const {
    // Проверяем, привязан ли сокет к порту
    return socket->state() == QUdpSocket::BoundState;
}

quint16 UdpChatProtocol::currentPort() const {
    // Возвращаем текущий локальный порт сокета
    return socket->localPort();
}

QString UdpChatProtocol::currentAddress() const {
    // Возвращаем текущий адрес назначения
    return destAddress.toString();
}

void UdpChatProtocol::bind(quint16 port)
{
    // Если сокет уже привязан, закрываем его перед перепривязкой
    if(socket->state() == QUdpSocket::BoundState) {
        socket->close();
    }

    // Пытаемся привязаться к указанному порту
    if(!socket->bind(port)) {
        // Генерируем ошибку при неудачной привязке
        emit errorOccurred(tr("Failed to bind to port %1: %2").arg(port).arg(socket->errorString()));
    }
}

void UdpChatProtocol::setDestination(const QString &address, quint16 port)
{
    // Устанавливаем адрес назначения с проверкой валидности
    if(!destAddress.setAddress(address)) {
        emit errorOccurred(tr("Invalid IP address: %1").arg(address));
        return;
    }
    destPort = port; // Устанавливаем порт назначения
}

void UdpChatProtocol::sendMessage(const QString &message, int packetSize)
{
    // Проверяем готовность сокета и наличие адреса назначения
    if(!socket->isValid() || destPort == 0) {
        emit errorOccurred(tr("Socket not ready or destination not set"));
        return;
    }

    // Конвертируем сообщение в байтовый массив
    QByteArray data = message.toUtf8();
    int totalSize = data.size();

    // Рассчитываем размер заголовка (3 int32)
    int headerSize = sizeof(qint32) * 3;
    // Максимальный размер данных в пакете (с учетом заголовка)
    int maxDataSize = qMax(1, packetSize - headerSize);
    // Рассчитываем количество пакетов
    int numPackets = (totalSize + maxDataSize - 1) / maxDataSize;

    // Формируем и отправляем пакеты
    for(int i = 0; i < numPackets; i++) {
        QByteArray packet;
        QDataStream stream(&packet, QIODevice::WriteOnly);

        // Записываем заголовок пакета
        stream << qint32(maxDataSize);   // Макс. размер данных в пакете
        stream << qint32(totalSize);     // Общий размер сообщения
        stream << qint32(i);             // Индекс текущего пакета

        // Вычисляем смещение и длину данных для текущего пакета
        int start = i * maxDataSize;
        int length = qMin(maxDataSize, totalSize - start);
        // Добавляем данные к пакету
        packet.append(data.mid(start, length));

        // Отправляем датаграмму
        qint64 bytesSent = socket->writeDatagram(packet, destAddress, destPort);
        // Обрабатываем ошибки отправки
        if(bytesSent == -1) {
            emit errorOccurred(tr("Failed to send packet: %1").arg(socket->errorString()));
        }
    }
}

void UdpChatProtocol::readPendingDatagrams()
{
    // Обрабатываем все ожидающие датаграммы
    while(socket->hasPendingDatagrams()) {
        // Получаем размер следующей датаграммы
        qint64 datagramSize = socket->pendingDatagramSize();
        QByteArray datagram;
        datagram.resize(static_cast<int>(datagramSize));

        QHostAddress senderAddress;
        quint16 senderPort;

        // Читаем датаграмму
        socket->readDatagram(
            datagram.data(),
            datagram.size(),
            &senderAddress,
            &senderPort
            );

        // Передаем датаграмму сборщику сообщений
        builder.processDatagram(datagram);

        // Если сообщение полностью собрано
        if(builder.isMessageComplete()) {
            // Генерируем сигнал о получении сообщения
            emit messageReceived(QString::fromUtf8(builder.getMessage()));
            // Сбрасываем сборщик для следующего сообщения
            builder.reset();
        }
    }
}
