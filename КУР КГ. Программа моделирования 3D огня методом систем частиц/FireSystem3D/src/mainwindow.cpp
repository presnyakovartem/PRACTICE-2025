#include "mainwindow.h"
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QTabWidget>
#include <QTableWidget>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), fireCanvas(new FireCanvas(this)),
      animationTimer(new QTimer(this)), selectedSystemIndex(-1) {

  setupUI();
  setupParticleControls();
  setupLightingControls();
  setupCameraControls();

  // Устанавливаем начальные значения для больших систем частиц
  particleCountSpin->setValue(50000);
  particleSizeSlider->setValue(30); // Увеличим размер
  particleSpeedSlider->setValue(30);

  updateSystemsTable();
  if (fireCanvas->getSystemCount() > 0) {
    systemsTable->selectRow(0);
  }

  // Таймер анимации
  connect(animationTimer, &QTimer::timeout, this,
          &MainWindow::onAnimationTimer);
  animationTimer->start(4);

  // Не создаем системы по умолчанию - они уже созданы в FireCanvas
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI() {
  setWindowTitle("Визуализация трёхмерного огня методом систем частиц с учётом "
                 "параметров внешнего освещения. Пресняков А.К. ИУ7-53Б");
  setMinimumSize(1400, 900);

  // Центральный виджет
  QWidget *centralWidget = new QWidget(this);
  setCentralWidget(centralWidget);

  // Основной layout
  QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);

  // Добавляем canvas слева (60% ширины)
  mainLayout->addWidget(fireCanvas, 6);

  // Панель управления справа (40% ширины)
  QTabWidget *controlTabs = new QTabWidget(this);
  controlTabs->setMinimumWidth(400);

  // === ВКЛАДКА СИСТЕМ ЧАСТИЦ ===
  QWidget *particleTab = new QWidget();
  QVBoxLayout *particleLayout = new QVBoxLayout(particleTab);

  // Таблица систем частиц
  QGroupBox *tableGroup = new QGroupBox("Системы частиц");
  QVBoxLayout *tableLayout = new QVBoxLayout(tableGroup);

  systemsTable = new QTableWidget(this);
  systemsTable->setColumnCount(5);
  systemsTable->setHorizontalHeaderLabels(
      {"Номер", "Кол-во частиц", "Размер", "Скорость", "Свет"});
  systemsTable->horizontalHeader()->setStretchLastSection(true);
  systemsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  systemsTable->setSelectionMode(QAbstractItemView::SingleSelection);
  tableLayout->addWidget(systemsTable);

  // Кнопки управления системами
  QHBoxLayout *systemButtonsLayout = new QHBoxLayout();
  addSystemButton = new QPushButton("Добавить систему");
  removeSystemButton = new QPushButton("Удалить систему");
  applyToAllButton = new QPushButton("Применить ко всем");

  systemButtonsLayout->addWidget(addSystemButton);
  systemButtonsLayout->addWidget(removeSystemButton);
  systemButtonsLayout->addWidget(applyToAllButton);
  tableLayout->addLayout(systemButtonsLayout);

  particleLayout->addWidget(tableGroup);

  // Группа управления параметрами
  QGroupBox *paramsGroup = new QGroupBox("Параметры частиц");
  QFormLayout *paramsForm = new QFormLayout(paramsGroup);

  // Количество частиц
  particleCountSpin = new QSpinBox();
  particleCountSpin->setRange(100, 10000000);
  particleCountSpin->setValue(5000);
  paramsForm->addRow("Максимальное кол-во:", particleCountSpin);

  // Размер частиц
  particleSizeSlider = new QSlider(Qt::Horizontal);
  particleSizeSlider->setRange(1, 200);
  particleSizeSlider->setValue(50);
  paramsForm->addRow("Размер:", particleSizeSlider);

  // Скорость частиц
  particleSpeedSlider = new QSlider(Qt::Horizontal);
  particleSpeedSlider->setRange(1, 200);
  particleSpeedSlider->setValue(50);
  paramsForm->addRow("Скорость движения:", particleSpeedSlider);

  // Цвет частиц
  redColorSlider = new QSlider(Qt::Horizontal);
  redColorSlider->setRange(0, 255);
  redColorSlider->setValue(255);
  paramsForm->addRow("Красный:", redColorSlider);

  greenColorSlider = new QSlider(Qt::Horizontal);
  greenColorSlider->setRange(0, 255);
  greenColorSlider->setValue(150);
  paramsForm->addRow("Зелёный:", greenColorSlider);

  blueColorSlider = new QSlider(Qt::Horizontal);
  blueColorSlider->setRange(0, 255);
  blueColorSlider->setValue(0);
  paramsForm->addRow("Синий:", blueColorSlider);

  particleLayout->addWidget(paramsGroup);
  particleLayout->addStretch();

  // === ВКЛАДКА ОСВЕЩЕНИЯ ===
  QWidget *lightingTab = new QWidget();
  QVBoxLayout *lightingLayout = new QVBoxLayout(lightingTab);

  QGroupBox *lightingGroup = new QGroupBox("Внешнее освещение");
  QFormLayout *lightingForm = new QFormLayout(lightingGroup);

  // Интенсивность
  lightIntensitySlider = new QSlider(Qt::Horizontal);
  lightIntensitySlider->setRange(0, 100);
  lightIntensitySlider->setValue(100); // 1.0 по умолчанию – поярче
  lightingForm->addRow("Интенсивность:", lightIntensitySlider);

  // Положение света
  lightXSlider = new QSlider(Qt::Horizontal);
  lightXSlider->setRange(-100, 100);
  lightXSlider->setValue(0);
  lightingForm->addRow("Свет по X:", lightXSlider);

  lightYSlider = new QSlider(Qt::Horizontal);
  lightYSlider->setRange(-100, 100);
  lightYSlider->setValue(8);
  lightingForm->addRow("Свет по Y:", lightYSlider);

  lightZSlider = new QSlider(Qt::Horizontal);
  lightZSlider->setRange(-100, 100);
  lightZSlider->setValue(6);
  lightingForm->addRow("Свет по Z:", lightZSlider);

  // Подпись
  lightPosLabel = new QLabel("Положение: (0, 8, 6)");
  lightingForm->addRow(lightPosLabel);

  lightPosLabel = new QLabel("Положение: (30, 50, 30)");
  lightingForm->addRow(lightPosLabel);

  lightingLayout->addWidget(lightingGroup);
  lightingLayout->addStretch();

  // === ВКЛАДКА КАМЕРЫ ===
  QWidget *cameraTab = new QWidget();
  QVBoxLayout *cameraLayout = new QVBoxLayout(cameraTab);

  QGroupBox *cameraGroup = new QGroupBox("Камера");
  QFormLayout *cameraForm = new QFormLayout(cameraGroup);

  cameraXSlider = new QSlider(Qt::Horizontal);
  cameraXSlider->setRange(-500, 500);
  cameraXSlider->setValue(0);
  cameraForm->addRow("Положение по X:", cameraXSlider);

  cameraYSlider = new QSlider(Qt::Horizontal);
  cameraYSlider->setRange(-500, 500);
  cameraYSlider->setValue(0);
  cameraForm->addRow("Положение по Y:", cameraYSlider);

  cameraZSlider = new QSlider(Qt::Horizontal);
  cameraZSlider->setRange(1, 100); // Ближе к сцене
  cameraZSlider->setValue(10);
  cameraForm->addRow("Положение по Z:", cameraZSlider);

  cameraRotationXSlider = new QSlider(Qt::Horizontal);
  cameraRotationXSlider->setRange(-180, 180);
  cameraRotationXSlider->setValue(0);
  cameraForm->addRow("Поворот по X:", cameraRotationXSlider);

  cameraRotationYSlider = new QSlider(Qt::Horizontal);
  cameraRotationYSlider->setRange(-180, 180);
  cameraRotationYSlider->setValue(0);
  cameraForm->addRow("Поворот по Y:", cameraRotationYSlider);

  fovSlider = new QSlider(Qt::Horizontal);
  fovSlider->setRange(10, 120);
  fovSlider->setValue(60);
  cameraForm->addRow("Фокусное расстояние:", fovSlider);

  cameraPosLabel = new QLabel("Положение: (0, 0, 100)");
  cameraRotLabel = new QLabel("Поворот: (0, 0)");
  cameraForm->addRow(cameraPosLabel);
  cameraForm->addRow(cameraRotLabel);

  cameraLayout->addWidget(cameraGroup);
  cameraLayout->addStretch();

  // === ВКЛАДКА СТАТИСТИКИ ===
  QWidget *statsTab = new QWidget();
  QVBoxLayout *statsLayout = new QVBoxLayout(statsTab);

  QGroupBox *statsGroup = new QGroupBox("Статистика");
  QFormLayout *statsForm = new QFormLayout(statsGroup);

  QLabel *fpsLabel = new QLabel("Текущий FPS: 0");
  QLabel *particlesLabel = new QLabel("Всего частиц: 0");
  QLabel *systemsLabel = new QLabel("Активные системы: 0");

  statsForm->addRow("Текущий FPS:", fpsLabel);
  statsForm->addRow("Всего частиц:", particlesLabel);
  statsForm->addRow("Активные системы:", systemsLabel);

  statsLayout->addWidget(statsGroup);
  statsLayout->addStretch();

  // Добавляем вкладки
  controlTabs->addTab(particleTab, "Частицы");
  controlTabs->addTab(lightingTab, "Свет");
  controlTabs->addTab(cameraTab, "Камера");
  controlTabs->addTab(statsTab, "Статистика");

  mainLayout->addWidget(controlTabs, 4);

  // Подключаем сигналы
  connect(particleCountSpin, QOverload<int>::of(&QSpinBox::valueChanged), this,
          &MainWindow::onParticleCountChanged);
  connect(particleSizeSlider, &QSlider::valueChanged, this,
          &MainWindow::onParticleSizeChanged);
  connect(particleSpeedSlider, &QSlider::valueChanged, this,
          &MainWindow::onParticleSpeedChanged);
  connect(redColorSlider, &QSlider::valueChanged, this,
          &MainWindow::onRedColorChanged);
  connect(greenColorSlider, &QSlider::valueChanged, this,
          &MainWindow::onGreenColorChanged);
  connect(blueColorSlider, &QSlider::valueChanged, this,
          &MainWindow::onBlueColorChanged);

  connect(lightIntensitySlider, &QSlider::valueChanged, this,
          &MainWindow::onLightIntensityChanged);
  connect(lightXSlider, &QSlider::valueChanged, this,
          &MainWindow::onSystemParameterChanged);
  connect(lightYSlider, &QSlider::valueChanged, this,
          &MainWindow::onSystemParameterChanged);
  connect(lightZSlider, &QSlider::valueChanged, this,
          &MainWindow::onSystemParameterChanged);

  connect(cameraXSlider, &QSlider::valueChanged, this,
          &MainWindow::onCameraXChanged);
  connect(cameraYSlider, &QSlider::valueChanged, this,
          &MainWindow::onCameraYChanged);
  connect(cameraZSlider, &QSlider::valueChanged, this,
          &MainWindow::onCameraZChanged);
  connect(cameraRotationXSlider, &QSlider::valueChanged, this,
          &MainWindow::onCameraRotationXChanged);
  connect(cameraRotationYSlider, &QSlider::valueChanged, this,
          &MainWindow::onCameraRotationYChanged);
  connect(fovSlider, &QSlider::valueChanged, this,
          &MainWindow::onSystemParameterChanged);

  connect(addSystemButton, &QPushButton::clicked, this,
          &MainWindow::onAddSystemClicked);
  connect(removeSystemButton, &QPushButton::clicked, this,
          &MainWindow::onRemoveSystemClicked);
  connect(applyToAllButton, &QPushButton::clicked, this,
          &MainWindow::onApplyToAllClicked);

  connect(systemsTable, &QTableWidget::itemSelectionChanged, this,
          &MainWindow::onSystemSelectionChanged);
}

void MainWindow::setupParticleControls() {
  onParticleCountChanged(50000);
  onParticleSizeChanged(50);
  onParticleSpeedChanged(50);
}

void MainWindow::setupLightingControls() {
  onLightIntensityChanged(lightIntensitySlider->value());
  onSystemParameterChanged(); // позиция света + FOV
}

void MainWindow::setupCameraControls() {
  onCameraXChanged(cameraXSlider->value());
  onCameraYChanged(cameraYSlider->value());
  onCameraZChanged(cameraZSlider->value());
  onCameraRotationXChanged(cameraRotationXSlider->value());
  onCameraRotationYChanged(cameraRotationYSlider->value());
  fireCanvas->setCameraFOV(fovSlider->value());
}

void MainWindow::updateSystemsTable() {
  systemsTable->setRowCount(fireCanvas->getSystemCount());

  for (int i = 0; i < fireCanvas->getSystemCount(); ++i) {
    const ParticleSystem *system = fireCanvas->getSystem(i);
    if (!system)
      continue;

    systemsTable->setItem(i, 0, new QTableWidgetItem(QString::number(i + 1)));

    systemsTable->setItem(
        i, 1,
        new QTableWidgetItem(QString("%1 / %2")
                                 .arg(system->getActiveParticles())
                                 .arg(system->getMaxParticles())));

    systemsTable->setItem(i, 2,
                          new QTableWidgetItem(QString::number(
                              system->getParticleSize(), 'f', 1)));

    systemsTable->setItem(i, 3,
                          new QTableWidgetItem(QString::number(
                              system->getParticleSpeed(), 'f', 1)));

    QColor color = system->getBaseColor();
    QTableWidgetItem *colorItem =
        new QTableWidgetItem(QString("RGB(%1, %2, %3)")
                                 .arg(color.red())
                                 .arg(color.green())
                                 .arg(color.blue()));
    colorItem->setBackground(color);
    systemsTable->setItem(i, 4, colorItem);
  }
}

void MainWindow::updateSelectedSystemParameters() {
  if (selectedSystemIndex >= 0 &&
      selectedSystemIndex < fireCanvas->getSystemCount()) {
    const ParticleSystem *system = fireCanvas->getSystem(selectedSystemIndex);
    if (system) {
      particleCountSpin->blockSignals(true);
      particleSizeSlider->blockSignals(true);
      particleSpeedSlider->blockSignals(true);
      redColorSlider->blockSignals(true);
      greenColorSlider->blockSignals(true);
      blueColorSlider->blockSignals(true);

      particleCountSpin->setValue(system->getMaxParticles());
      particleSizeSlider->setValue(
          static_cast<int>(system->getParticleSize() * 10));
      particleSpeedSlider->setValue(
          static_cast<int>(system->getParticleSpeed() * 10));

      QColor color = system->getBaseColor();
      redColorSlider->setValue(color.red());
      greenColorSlider->setValue(color.green());
      blueColorSlider->setValue(color.blue());

      particleCountSpin->blockSignals(false);
      particleSizeSlider->blockSignals(false);
      particleSpeedSlider->blockSignals(false);
      redColorSlider->blockSignals(false);
      greenColorSlider->blockSignals(false);
      blueColorSlider->blockSignals(false);
    }
  }
}

void MainWindow::onAddSystemClicked() {
  // 1. Создаём новую систему с дефолтной позицией
  fireCanvas->addParticleSystem();

  // 2. Берём её индекс (последняя в списке)
  int newIndex = fireCanvas->getSystemCount() - 1;
  if (newIndex >= 0) {
    // 3. Читаем текущие параметры из интерфейса
    int count = particleCountSpin->value();
    float size = particleSizeSlider->value() / 10.0f;
    float speed = particleSpeedSlider->value() / 10.0f;
    QColor color(redColorSlider->value(), greenColorSlider->value(),
                 blueColorSlider->value());

    // 4. Применяем их к только что созданной системе
    fireCanvas->setParticleParameters(newIndex, count, size, speed, color);
  }

  // 5. Обновляем таблицу и выделяем новую систему
  updateSystemsTable();
  if (newIndex >= 0) {
    systemsTable->selectRow(newIndex);
  }
}

void MainWindow::onRemoveSystemClicked() {
  if (selectedSystemIndex >= 0) {
    fireCanvas->removeParticleSystem(selectedSystemIndex);
    updateSystemsTable();

    if (fireCanvas->getSystemCount() > 0) {
      int newIndex = selectedSystemIndex;
      if (newIndex >= fireCanvas->getSystemCount())
        newIndex = fireCanvas->getSystemCount() - 1;

      // Выбираем ближайшую существующую систему
      systemsTable->selectRow(
          newIndex); // onSystemSelectionChanged обновит selectedSystemIndex
    } else {
      selectedSystemIndex = -1;
    }
  }
}

void MainWindow::onApplyToAllClicked() {
  int systemCount = fireCanvas->getSystemCount();
  if (systemCount == 0)
    return;

  int count = particleCountSpin->value();
  float size = particleSizeSlider->value() / 10.0f;
  float speed = particleSpeedSlider->value() / 10.0f;
  QColor color(redColorSlider->value(), greenColorSlider->value(),
               blueColorSlider->value());

  for (int i = 0; i < systemCount; ++i) {
    fireCanvas->setParticleParameters(i, count, size, speed, color);
  }

  updateSystemsTable();
}

void MainWindow::onParticleCountChanged(int value) {
  if (selectedSystemIndex >= 0) {
    float size = particleSizeSlider->value() / 10.0f;
    float speed = particleSpeedSlider->value() / 10.0f;
    QColor color(redColorSlider->value(), greenColorSlider->value(),
                 blueColorSlider->value());

    fireCanvas->setParticleParameters(selectedSystemIndex, value, size, speed,
                                      color);
    updateSystemsTable();
  }
}

void MainWindow::onParticleSizeChanged(int value) {
  if (selectedSystemIndex >= 0) {
    int count = particleCountSpin->value();
    float speed = particleSpeedSlider->value() / 10.0f;
    QColor color(redColorSlider->value(), greenColorSlider->value(),
                 blueColorSlider->value());

    fireCanvas->setParticleParameters(selectedSystemIndex, count, value / 10.0f,
                                      speed, color);
    updateSystemsTable();
  }
}

void MainWindow::onParticleSpeedChanged(int value) {
  if (selectedSystemIndex >= 0) {
    int count = particleCountSpin->value();
    float size = particleSizeSlider->value() / 10.0f;
    QColor color(redColorSlider->value(), greenColorSlider->value(),
                 blueColorSlider->value());

    fireCanvas->setParticleParameters(selectedSystemIndex, count, size,
                                      value / 10.0f, color);
    updateSystemsTable();
  }
}

void MainWindow::onRedColorChanged(int value) {
  if (selectedSystemIndex >= 0) {
    int count = particleCountSpin->value();
    float size = particleSizeSlider->value() / 10.0f;
    float speed = particleSpeedSlider->value() / 10.0f;
    QColor color(value, greenColorSlider->value(), blueColorSlider->value());

    fireCanvas->setParticleParameters(selectedSystemIndex, count, size, speed,
                                      color);
    updateSystemsTable();
  }
}

void MainWindow::onGreenColorChanged(int value) {
  if (selectedSystemIndex >= 0) {
    int count = particleCountSpin->value();
    float size = particleSizeSlider->value() / 10.0f;
    float speed = particleSpeedSlider->value() / 10.0f;
    QColor color(redColorSlider->value(), value, blueColorSlider->value());

    fireCanvas->setParticleParameters(selectedSystemIndex, count, size, speed,
                                      color);
    updateSystemsTable();
  }
}

void MainWindow::onBlueColorChanged(int value) {
  if (selectedSystemIndex >= 0) {
    int count = particleCountSpin->value();
    float size = particleSizeSlider->value() / 10.0f;
    float speed = particleSpeedSlider->value() / 10.0f;
    QColor color(redColorSlider->value(), greenColorSlider->value(), value);

    fireCanvas->setParticleParameters(selectedSystemIndex, count, size, speed,
                                      color);
    updateSystemsTable();
  }
}

void MainWindow::onLightIntensityChanged(int value) {
  float intensity = value / 100.0f;
  fireCanvas->setLightIntensity(intensity);
}

void MainWindow::onCameraXChanged(int value) {
  fireCanvas->setCameraPosition(value, cameraYSlider->value(),
                                cameraZSlider->value());

  cameraPosLabel->setText(QString("Положение: (%1, %2, %3)")
                              .arg(value)
                              .arg(cameraYSlider->value())
                              .arg(cameraZSlider->value()));
}

void MainWindow::onCameraYChanged(int value) {
  fireCanvas->setCameraPosition(cameraXSlider->value(), value,
                                cameraZSlider->value());

  cameraPosLabel->setText(QString("Положение: (%1, %2, %3)")
                              .arg(cameraXSlider->value())
                              .arg(value)
                              .arg(cameraZSlider->value()));
}

void MainWindow::onCameraZChanged(int value) {
  fireCanvas->setCameraPosition(cameraXSlider->value(), cameraYSlider->value(),
                                value);

  cameraPosLabel->setText(QString("Положение: (%1, %2, %3)")
                              .arg(cameraXSlider->value())
                              .arg(cameraYSlider->value())
                              .arg(value));
}

void MainWindow::onCameraRotationXChanged(int value) {
  fireCanvas->setCameraRotation(value, cameraRotationYSlider->value());
  cameraRotLabel->setText(QString("Поворот: (%1, %2)")
                              .arg(value)
                              .arg(cameraRotationYSlider->value()));
}

void MainWindow::onCameraRotationYChanged(int value) {
  fireCanvas->setCameraRotation(cameraRotationXSlider->value(), value);
  cameraRotLabel->setText(QString("Поворот: (%1, %2)")
                              .arg(cameraRotationXSlider->value())
                              .arg(value));
}

void MainWindow::onSystemParameterChanged() {
  fireCanvas->setLightPosition(lightXSlider->value(), lightYSlider->value(),
                               lightZSlider->value());

  lightPosLabel->setText(QString("Положение: (%1, %2, %3)")
                             .arg(lightXSlider->value())
                             .arg(lightYSlider->value())
                             .arg(lightZSlider->value()));

  fireCanvas->setCameraFOV(fovSlider->value());
}

void MainWindow::onSystemSelectionChanged() {
  QList<QTableWidgetItem *> selectedItems = systemsTable->selectedItems();
  if (!selectedItems.isEmpty()) {
    selectedSystemIndex = selectedItems.first()->row();
    updateSelectedSystemParameters();
  } else {
    selectedSystemIndex = -1;
  }
}

void MainWindow::onAnimationTimer() {
  fireCanvas->update();
  updateSystemsTable();

  QWidget *statsTab =
      dynamic_cast<QTabWidget *>(centralWidget()->layout()->itemAt(1)->widget())
          ->widget(3);
  QFormLayout *statsForm = dynamic_cast<QFormLayout *>(
      dynamic_cast<QGroupBox *>(statsTab->layout()->itemAt(0)->widget())
          ->layout());

  if (statsForm) {
    QLabel *fpsLabel = dynamic_cast<QLabel *>(statsForm->itemAt(1)->widget());
    QLabel *particlesLabel =
        dynamic_cast<QLabel *>(statsForm->itemAt(3)->widget());
    QLabel *systemsLabel =
        dynamic_cast<QLabel *>(statsForm->itemAt(5)->widget());

    if (fpsLabel)
      fpsLabel->setText(QString("Текущий FPS: %1")
                            .arg(fireCanvas->getCurrentFPS(), 0, 'f', 1));
    if (particlesLabel)
      particlesLabel->setText(
          QString("Всего частиц: %1").arg(fireCanvas->getTotalParticles()));
    if (systemsLabel)
      systemsLabel->setText(
          QString("Активные системы: %1").arg(fireCanvas->getSystemCount()));
  }
}
