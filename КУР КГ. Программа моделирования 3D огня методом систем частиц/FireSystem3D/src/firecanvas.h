#ifndef FIRECANVAS_H
#define FIRECANVAS_H

#include "matrix4x4.h"
#include "particlesystem.h"
#include "scene.h"
#include "vector3.h"
#include <QImage>
#include <QMouseEvent>
#include <QWidget>
#include <algorithm>
#include <cmath>
#include <vector>

class FireCanvas : public QWidget {
  Q_OBJECT

public:
  FireCanvas(QWidget *parent = nullptr);
  ~FireCanvas();

  // Управление системами частиц
  void addParticleSystem();
  void addParticleSystemAt(
      const Vector3 &position); // Добавляем систему в конкретной позиции
  void removeParticleSystem(int systemIndex);
  void setParticleParameters(int systemIndex, int count, float size,
                             float speed, const QColor &color);
  int getSystemCount() const { return scene.getParticleSystems().size(); }
  const ParticleSystem *getSystem(int index) const;
  int getTotalParticles() const;

  // Управление освещением
  void setLightIntensity(float intensity);
  void setLightPosition(float x, float y, float z);

  // Управление камерой
  void setCameraPosition(float x, float y, float z);
  void setCameraRotation(float rotX, float rotY);
  void setCameraFOV(float fov);

  // Статистика
  float getCurrentFPS() const { return currentFPS; }

protected:
  void paintEvent(QPaintEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override; // Обработка клика мыши

private:
  void initializeBuffers();
  void renderFrame();
  void updateFPS();
  bool renderParticle(const Particle &particle,
                      const Matrix4x4 &viewProjection);
  QColor calculateParticleColor(const Particle &particle);
  void drawBillboard(int centerX, int centerY, float depth, int size,
                     const QColor &color);
  QColor blendColors(const QColor &background, const QColor &foreground);
  QColor blendColorsAdditive(const QColor &background,
                             const QColor &foreground);
  Vector3
  screenToWorld(int screenX,
                int screenY); // Преобразование экранных координат в мировые

  QImage frameBuffer;
  std::vector<float> depthBuffer;
  Scene scene;
  int width;
  int height;

  // Статистика
  float currentFPS;
  qint64 lastFrameTime;
  int frameCount;
  qint64 fpsLastTime;
};

#endif // FIRECANVAS_H
