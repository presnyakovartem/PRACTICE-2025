#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "udpchatprotocol.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// Класс главного окна приложения
class MainWindow : public QMainWindow
{
    Q_OBJECT  // Макрос для поддержки сигналов и слотов

public:
    // Конструктор и деструктор
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Слоты для обработки пользовательских действий
    void on_sendButton_clicked();  // Обработчик нажатия кнопки отправки
    void on_packetSize_valueChanged(int value);  // Обработчик изменения размера пакета
    void on_messageReceived(const QString &message);  // Обработчик получения сообщения
    void on_applySettings_clicked();  // Обработчик применения сетевых настроек
    void on_clearChatButton_clicked();  // Обработчик очистки истории чата

private:
    Ui::MainWindow *ui;  // Указатель на сгенерированный UI
    UdpChatProtocol *protocol;  // Указатель на сетевой протокол чата

    // Параметры приложения
    int currentPacketSize = 512;  // Текущий размер UDP-пакета (по умолчанию 512 байт)
    quint16 previousPort = 12345;  // Предыдущий порт для восстановления при ошибках
};

#endif // MAINWINDOW_H
