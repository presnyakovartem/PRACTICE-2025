#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "firecanvas.h"
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMainWindow>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QTableWidget>
#include <QTimer>
#include <QVBoxLayout>

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private slots:
  void onAddSystemClicked();
  void onRemoveSystemClicked();
  void onApplyToAllClicked();
  void onParticleCountChanged(int value);
  void onParticleSizeChanged(int value);
  void onParticleSpeedChanged(int value);
  void onRedColorChanged(int value);
  void onGreenColorChanged(int value);
  void onBlueColorChanged(int value);
  void onLightIntensityChanged(int value);
  void onAnimationTimer();
  void onSystemSelectionChanged();
  void onCameraXChanged(int value);
  void onCameraYChanged(int value);
  void onCameraZChanged(int value);
  void onCameraRotationXChanged(int value);
  void onCameraRotationYChanged(int value);
  void onSystemParameterChanged();

private:
  void loadStyle();
  void setupUI();
  void setupParticleControls();
  void setupLightingControls();
  void setupCameraControls();
  void updateSystemsTable();
  void updateSelectedSystemParameters();

  FireCanvas *fireCanvas;
  QTimer *animationTimer;

  // Элементы управления частицами
  QSpinBox *particleCountSpin;
  QSlider *particleSizeSlider;
  QSlider *particleSpeedSlider;
  QSlider *redColorSlider;
  QSlider *greenColorSlider;
  QSlider *blueColorSlider;
  QPushButton *addSystemButton;
  QPushButton *removeSystemButton;
  QPushButton *applyToAllButton;

  // Элементы управления освещением
  QSlider *lightIntensitySlider;
  QSlider *lightXSlider;
  QSlider *lightYSlider;
  QSlider *lightZSlider;

  // Элементы управления камерой
  QSlider *cameraXSlider;
  QSlider *cameraYSlider;
  QSlider *cameraZSlider;
  QSlider *cameraRotationXSlider;
  QSlider *cameraRotationYSlider;
  QSlider *fovSlider;

  // Таблица систем частиц
  QTableWidget *systemsTable;
  int selectedSystemIndex;

  // Метки для отображения значений
  QLabel *cameraPosLabel;
  QLabel *cameraRotLabel;
  QLabel *lightPosLabel;
};

#endif // MAINWINDOW_H
