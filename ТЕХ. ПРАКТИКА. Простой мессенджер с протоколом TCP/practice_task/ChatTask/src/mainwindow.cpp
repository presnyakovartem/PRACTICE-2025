#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QRegularExpressionValidator>
#include <QMessageBox>
#include <QRegularExpression>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , protocol(new UdpChatProtocol(this))
{
    ui->setupUi(this);

    setWindowFlag(Qt::MSWindowsFixedSizeDialogHint, true);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    // Настройка UI
    ui->packetSize->setValue(currentPacketSize);
    ui->chatHistory->setReadOnly(true);

    // Установка адреса по умолчанию
    ui->destIPLineEdit->setText("127.0.0.1");

    // Валидация IP-адреса с использованием регулярного выражения
    QRegularExpression ipRegex("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    QRegularExpressionValidator *ipValidator = new QRegularExpressionValidator(ipRegex, this);
    ui->destIPLineEdit->setValidator(ipValidator);

    // Подключение сигналов от протокола к слотам главного окна
    connect(protocol, &UdpChatProtocol::messageReceived,
            this, &MainWindow::on_messageReceived);
    connect(protocol, &UdpChatProtocol::errorOccurred,
            this, [this](const QString &error){
                ui->chatHistory->append(tr("Error: %1").arg(error));
            });

    // Автоотправка сообщения при нажатии Enter в поле ввода
    connect(ui->messageInput, &QLineEdit::returnPressed,
            this, &MainWindow::on_sendButton_clicked);

    // Подключение кнопок управления к соответствующим слотам
    connect(ui->applySettingsButton, &QPushButton::clicked,
            this, &MainWindow::on_applySettings_clicked);
    connect(ui->clearChatButton, &QPushButton::clicked,
            this, &MainWindow::on_clearChatButton_clicked);

    // Первоначальная привязка сокета к порту и установка адреса назначения
    protocol->bind(ui->localPortSpinBox->value());
    protocol->setDestination(ui->destIPLineEdit->text(), ui->destPortSpinBox->value());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_applySettings_clicked()
{
    // Получаем значения сетевых настроек из интерфейса
    quint16 localPort = ui->localPortSpinBox->value();
    QString destIP = ui->destIPLineEdit->text();
    quint16 destPort = ui->destPortSpinBox->value();

    // Проверка валидности введенного IP-адреса
    QHostAddress testAddr;
    if(!testAddr.setAddress(destIP)) {
        QMessageBox::warning(this, "Invalid IP", "Please enter a valid IP address");
        return;
    }

    // Сохраняем предыдущее состояние привязки сокета
    bool wasBound = protocol->isBound();

    // Применяем новые настройки сети
    protocol->bind(localPort);
    protocol->setDestination(destIP, destPort);

    // Проверка успешности привязки к новому порту
    if(wasBound && !protocol->isBound()) {
        // Восстанавливаем предыдущий порт при ошибке
        protocol->bind(previousPort);
        QMessageBox::warning(this, "Binding Error",
                             "Failed to bind to new port. Reverting to previous settings.");
    } else {
        // Сохраняем новый порт при успешной привязке
        previousPort = localPort;
    }

    // Отображаем примененные настройки в истории чата
    ui->chatHistory->append(tr("Settings applied: Local port %1, Destination %2:%3")
                                .arg(localPort).arg(destIP).arg(destPort));
}

void MainWindow::on_sendButton_clicked()
{
    // Получаем текст сообщения из поля ввода
    QString message = ui->messageInput->text().trimmed();
    if(message.isEmpty()) return;

    // Отображаем исходящее сообщение в истории чата
    ui->chatHistory->append(tr(">> %1").arg(message));
    ui->messageInput->clear();

    // Отправляем сообщение через сетевой протокол
    protocol->sendMessage(message, currentPacketSize);
}

void MainWindow::on_packetSize_valueChanged(int value)
{
    // Обновляем текущий размер пакета при изменении значения в интерфейсе
    currentPacketSize = value;
    ui->chatHistory->append(tr("Packet size changed to %1 bytes").arg(value));
}

void MainWindow::on_messageReceived(const QString &message)
{
    // Отображаем входящее сообщение в истории чата
    ui->chatHistory->append(tr("<< %1").arg(message));
}

void MainWindow::on_clearChatButton_clicked()
{
    // Очищаем историю сообщений
    ui->chatHistory->clear();
}
