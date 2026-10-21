
#include "firecanvas.h"
#include <QDebug>
#include <QPaintEvent>
#include <QPainter>
#include <QTime>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

FireCanvas::FireCanvas(QWidget *parent)
    : QWidget(parent), width(800), height(600), currentFPS(0.0f),
      lastFrameTime(0), frameCount(0), fpsLastTime(0) {
  setMinimumSize(400, 300);
  setMouseTracking(true); // Включаем отслеживание мыши
  setFocusPolicy(Qt::StrongFocus); // Чтобы получать события мыши
  initializeBuffers();

  fpsLastTime = QTime::currentTime().msecsSinceStartOfDay();
}

FireCanvas::~FireCanvas() {}

void FireCanvas::initializeBuffers() {
  frameBuffer = QImage(width, height, QImage::Format_ARGB32);
  frameBuffer.fill(Qt::black);

  depthBuffer.resize(width * height);
  std::fill(depthBuffer.begin(), depthBuffer.end(), 1.0f);
}

void FireCanvas::addParticleSystem() { addParticleSystemAt(Vector3(0, 0, 0)); }

void FireCanvas::addParticleSystemAt(const Vector3 &position) {
  scene.addParticleSystem(position);
  update();
}

void FireCanvas::removeParticleSystem(int systemIndex) {
  scene.removeParticleSystem(systemIndex);
  update();
}

void FireCanvas::setParticleParameters(int systemIndex, int count, float size,
                                       float speed, const QColor &color) {
  if (systemIndex >= 0 && systemIndex < scene.getParticleSystems().size()) {
    scene.getParticleSystems()[systemIndex].setParameters(count, size, speed,
                                                          color);
  }
  update();
}

const ParticleSystem *FireCanvas::getSystem(int index) const {
  if (index >= 0 && index < scene.getParticleSystems().size()) {
    return &scene.getParticleSystems()[index];
  }
  return nullptr;
}

int FireCanvas::getTotalParticles() const {
  int total = 0;
  for (const auto &system : scene.getParticleSystems()) {
    total += system.getActiveParticles();
  }
  return total;
}

// Управление освещением
void FireCanvas::setLightIntensity(float intensity) {
  scene.getLight().intensity = intensity;
  update();
}

void FireCanvas::setLightPosition(float x, float y, float z) {
  scene.getLight().position = Vector3(x, y, z);
  update();
}

// Управление камерой
void FireCanvas::setCameraPosition(float x, float y, float z) {
  scene.getCamera().position = Vector3(x, y, z);
  update();
}

void FireCanvas::setCameraRotation(float rotX, float rotY) {
  scene.getCamera().rotation = Vector3(rotX, rotY, 0);
  update();
}

void FireCanvas::setCameraFOV(float fov) {
  scene.getCamera().fov = fov;
  update();
}

void FireCanvas::updateFPS() {
  frameCount++;
  qint64 currentTime = QTime::currentTime().msecsSinceStartOfDay();

  if (currentTime - fpsLastTime >= 1000) { // Каждую секунду
    currentFPS = frameCount * 1000.0f / (currentTime - fpsLastTime);
    frameCount = 0;
    fpsLastTime = currentTime;
  }
}

void FireCanvas::paintEvent(QPaintEvent *event) {
  Q_UNUSED(event);

  QPainter painter(this);

  // Обновляем сцену
  scene.update(0.016f);

  // Рендерим кадр
  renderFrame();

  // Отображаем framebuffer
  painter.drawImage(rect(), frameBuffer, frameBuffer.rect());

  // Обновляем FPS
  updateFPS();
}

void FireCanvas::resizeEvent(QResizeEvent *event) {
  width = event->size().width();
  height = event->size().height();
  initializeBuffers();
  QWidget::resizeEvent(event);
}

void FireCanvas::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    // Получаем координаты клика
    QPoint pos = event->pos();

    // Преобразуем в мировые координаты
    Vector3 worldPos = screenToWorld(pos.x(), pos.y());

    // Создаем новую систему частиц в этой позиции
    addParticleSystemAt(worldPos);

    qDebug() << "Added particle system at:" << worldPos.x << worldPos.y
             << worldPos.z;
  }

  QWidget::mousePressEvent(event);
}

Vector3 FireCanvas::screenToWorld(int screenX, int screenY) {
  // Экран -> NDC [-1; 1]
  float x = (2.0f * screenX) / width - 1.0f;
  float y = 1.0f - (2.0f * screenY) / height; // инвертируем Y, как в рендере

  Camera &camera = scene.getCamera();
  float aspect = float(width) / float(height);
  Matrix4x4 viewMatrix = camera.getViewMatrix();
  Matrix4x4 projectionMatrix = camera.getProjectionMatrix(aspect);

  // Полная матрица и её обратная
  Matrix4x4 viewProjection = projectionMatrix * viewMatrix;
  Matrix4x4 invViewProjection = viewProjection.inverted();

  // Луч: две точки в NDC на ближней и дальней плоскости
  Vector3 ndcNear(x, y, -1.0f);
  Vector3 ndcFar(x, y, 1.0f);

  Vector3 worldNear = invViewProjection * ndcNear;
  Vector3 worldFar = invViewProjection * ndcFar;

  // Пересечение с плоскостью Z = 0
  float dz = worldFar.z - worldNear.z;
  if (std::fabs(dz) > 1e-6f) {
    float t = -worldNear.z / dz;
    return worldNear + (worldFar - worldNear) * t;
  }

  // На всякий случай – если луч почти параллелен плоскости
  return worldNear;
}

void FireCanvas::renderFrame() {
  // Очищаем буферы
  frameBuffer.fill(Qt::black);
  std::fill(depthBuffer.begin(), depthBuffer.end(), 1.0f);

  // Отладочная информация
  int totalParticles = getTotalParticles();

  // Получаем матрицы камеры
  Camera &camera = scene.getCamera();
  float aspect = float(width) / float(height);
  Matrix4x4 viewMatrix = camera.getViewMatrix();
  Matrix4x4 projectionMatrix = camera.getProjectionMatrix(aspect);
  Matrix4x4 viewProjection = projectionMatrix * viewMatrix;

  // Рендерим все системы частиц
  int renderedParticles = 0;
  for (const auto &system : scene.getParticleSystems()) {
    for (const auto &particle : system.getParticles()) {
      if (!particle.isAlive())
        continue;

      if (renderParticle(particle, viewProjection)) {
        renderedParticles++;
      }
    }
  }

  // Отладочная информация на экране
  QPainter painter(&frameBuffer);
  painter.setPen(Qt::white);
  painter.drawText(
      10, 20,
      QString("Системы: %1 | Частицы: %2 | Отрисовано: %3 | FPS: %4")
          .arg(scene.getParticleSystems().size())
          .arg(totalParticles)
          .arg(renderedParticles)
          .arg(currentFPS, 0, 'f', 1));

  // Инструкция для пользователя
  painter.drawText(10, height - 30,
                   "Нажмите на экран для установки эмиттера новой системы");
}

bool FireCanvas::renderParticle(const Particle &particle,
                                const Matrix4x4 &viewProjection) {
  // Преобразуем позицию частицы в экранные координаты
  Vector3 screenPos = viewProjection * particle.position;

  // Проверяем, находится ли частица перед камерой и в пределах видимой области
  if (screenPos.z < 0 || screenPos.z > 1 || screenPos.x < -1 ||
      screenPos.x > 1 || screenPos.y < -1 || screenPos.y > 1) {
    return false;
  }

  // Преобразуем в пиксельные координаты
  int x = (screenPos.x + 1) * width / 2;
  int y = (1 - screenPos.y) * height / 2;

  // Вычисляем размер частицы на экране
  float screenSize = particle.size * (1.0f - screenPos.z) * 100.0f;
  int sizePixels = std::max(2, static_cast<int>(screenSize));

  // Вычисляем цвет
  QColor finalColor = calculateParticleColor(particle);

  // Рисуем частицу
  drawBillboard(x, y, screenPos.z, sizePixels, finalColor);

  return true;
}

QColor FireCanvas::calculateParticleColor(const Particle &particle) {
  Light &light = scene.getLight();

  // Базовый цвет частицы (огонь уже "раскрашен" системой частиц)
  QColor baseColor = particle.color;

  // Учитываем время жизни (частицы постепенно растворяются)
  float life = std::max(0.0f, std::min(1.0f, particle.life));
  baseColor.setAlphaF(life);

  // --- Внешний точечный источник света ---
  Vector3 toLight = light.position - particle.position;
  float distanceToLight = toLight.length();

  // Более мягкое затухание по расстоянию (квадратичное)
  float attenuation = 1.0f / (1.0f + 0.01f * distanceToLight * distanceToLight);

  // Собственное свечение огня + вклад внешнего источника
  float emissionFactor = 1.0f; // огонь сам по себе светится
  float externalFactor =
      light.intensity * attenuation; // влияние внешнего источника
  float totalFactor = emissionFactor + externalFactor;

  // Работаем в float [0..1], чтобы аккуратно клампить
  float r = baseColor.redF() * totalFactor;
  float g = baseColor.greenF() * totalFactor;
  float b = baseColor.blueF() * totalFactor;
  float a = baseColor.alphaF();

  r = std::min(1.0f, r);
  g = std::min(1.0f, g);
  b = std::min(1.0f, b);

  QColor result;
  result.setRgbF(r, g, b, a);
  return result;
}

void FireCanvas::drawBillboard(int centerX, int centerY, float depth, int size,
                               const QColor &color) {
  int halfSize = size / 2;
  int startX = std::max(0, centerX - halfSize);
  int endX = std::min(width - 1, centerX + halfSize);
  int startY = std::max(0, centerY - halfSize);
  int endY = std::min(height - 1, centerY + halfSize);

  for (int y = startY; y <= endY; y++) {
    for (int x = startX; x <= endX; x++) {
      int index = y * width + x;

      // Проверка глубины
      if (depth < depthBuffer[index]) {
        depthBuffer[index] = depth;

        // Простое сглаживание - вычисляем прозрачность на основе расстояния от
        // центра
        float dx = std::abs(x - centerX) / float(halfSize + 0.5f);
        float dy = std::abs(y - centerY) / float(halfSize + 0.5f);
        float distance = std::sqrt(dx * dx + dy * dy);

        if (distance <= 1.0f) {
          // Яркий центр, прозрачные края
          float alpha = (1.0f - distance * distance) * color.alphaF();
          QColor finalColor = color;
          finalColor.setAlphaF(alpha);

          // Смешивание цветов (additive blending для огня)
          QColor currentColor = frameBuffer.pixelColor(x, y);
          QColor blended = blendColorsAdditive(currentColor, finalColor);
          frameBuffer.setPixel(x, y, blended.rgba());
        }
      }
    }
  }
}

// Аддитивное смешивание для огня
QColor FireCanvas::blendColorsAdditive(const QColor &background,
                                       const QColor &foreground) {
  float alpha = foreground.alphaF();

  int r = std::min(255, background.red() +
                            static_cast<int>(foreground.red() * alpha));
  int g = std::min(255, background.green() +
                            static_cast<int>(foreground.green() * alpha));
  int b = std::min(255, background.blue() +
                            static_cast<int>(foreground.blue() * alpha));

  return QColor(r, g, b);
}

QColor FireCanvas::blendColors(const QColor &background,
                               const QColor &foreground) {
  float alpha = foreground.alphaF();
  float invAlpha = 1.0f - alpha;

  int r =
      static_cast<int>(foreground.red() * alpha + background.red() * invAlpha);
  int g = static_cast<int>(foreground.green() * alpha +
                           background.green() * invAlpha);
  int b = static_cast<int>(foreground.blue() * alpha +
                           background.blue() * invAlpha);

  return QColor(r, g, b);
}
