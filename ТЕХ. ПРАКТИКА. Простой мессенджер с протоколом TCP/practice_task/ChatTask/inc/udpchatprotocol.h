#ifndef UDPCHATPROTOCOL_H
#define UDPCHATPROTOCOL_H

#include <QtCore>
#include <QUdpSocket>
#include <QHostAddress>
#include "messagebuilder.h"

// Класс сетевого протокола для UDP-чата
class UdpChatProtocol : public QObject
{
    Q_OBJECT
public:
    explicit UdpChatProtocol(QObject *parent = nullptr);
    ~UdpChatProtocol();

    // Основные сетевые операции
    void bind(quint16 port);  // Привязка к локальному порту
    void setDestination(const QString &address, quint16 port);  // Установка адреса назначения
    void sendMessage(const QString &message, int packetSize);  // Отправка сообщения

    // Методы получения текущего состояния
    bool isBound() const;        // Проверка привязки к порту
    quint16 currentPort() const; // Получение текущего порта
    QString currentAddress() const;  // Получение текущего адреса назначения

signals:
    void messageReceived(const QString &message);  // Сигнал о получении сообщения
    void errorOccurred(const QString &error);      // Сигнал об ошибке

private slots:
    void readPendingDatagrams();  // Чтение доступных датаграмм

private:
    QUdpSocket *socket;     // UDP-сокет для сетевого взаимодействия
    QHostAddress destAddress;  // Адрес назначения для отправки сообщений
    quint16 destPort;       // Порт назначения
    MessageBuilder builder;  // Сборщик фрагментированных сообщений
};

#endif // UDPCHATPROTOCOL_H
