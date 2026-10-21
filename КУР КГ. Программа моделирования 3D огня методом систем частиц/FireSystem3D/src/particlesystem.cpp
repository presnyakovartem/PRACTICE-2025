#include "particlesystem.h"
#include <cmath>
#include <cstdlib>
#include <ctime>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace {
// Кламп в [0, 1]
float clamp01(float x) {
  if (x < 0.0f)
    return 0.0f;
  if (x > 1.0f)
    return 1.0f;
  return x;
}

// Линейная интерполяция цветов
QColor lerpColor(const QColor &a, const QColor &b, float t) {
  t = clamp01(t);
  float r = a.redF() + (b.redF() - a.redF()) * t;
  float g = a.greenF() + (b.greenF() - a.greenF()) * t;
  float bch = a.blueF() + (b.blueF() - a.blueF()) * t;
  float af = a.alphaF() + (b.alphaF() - a.alphaF()) * t;

  QColor c;
  c.setRgbF(r, g, bch, af);
  return c;
}
} // namespace

ParticleSystem::ParticleSystem()
    : maxParticles(1000000), particleSize(0.3f), particleSpeed(1.0f),
      baseColor(Qt::yellow), emissionRate(600.0f), timeSinceLastEmission(0.0f),
      position(0, 0, 0) {
  std::srand(static_cast<unsigned int>(std::time(nullptr)));
  particles.reserve(maxParticles);

  // Сразу создаем много частиц
  emitParticles(5000);
}

ParticleSystem::ParticleSystem(const Vector3 &position)
    : maxParticles(100000), particleSize(0.3f), particleSpeed(1.0f),
      baseColor(Qt::red), emissionRate(1500.0f), timeSinceLastEmission(0.0f),
      position(position) {
  std::srand(static_cast<unsigned int>(std::time(nullptr)));
  particles.reserve(maxParticles);
  emitParticles(5000);
}

ParticleSystem::~ParticleSystem() {}

void ParticleSystem::update(float deltaTime) {
  timeSinceLastEmission += deltaTime;

  // Генерация новых частиц
  int particlesToEmit = static_cast<int>(emissionRate * deltaTime);
  if (particlesToEmit > 0 &&
      particles.size() < static_cast<size_t>(maxParticles)) {
    emitParticles(particlesToEmit);
    timeSinceLastEmission = 0.0f;
  }

  // Обновление существующих частиц
  for (auto &particle : particles) {
    if (!particle.isAlive())
      continue;

    // Движение относительно позиции системы
    Vector3 relativePos = particle.position - position;
    particle.position =
        position + relativePos + particle.velocity * deltaTime * particleSpeed;

    // Более быстрый и «нормальный» распад жизни (около 1–2 секунд)
    const float decayRate = 0.4f; // ~1.2–1.5 секунды жизни
    particle.life -= decayRate * deltaTime;

    if (!particle.isAlive())
      continue;

    // Турбулентность
    particle.velocity.x += (std::rand() % 100 - 50) / 500.0f * deltaTime;
    particle.velocity.z += (std::rand() % 100 - 50) / 500.0f * deltaTime;

    // Нормализованная жизнь частицы [0..1]
    float life = clamp01(particle.life);

    // ---------- Размер по жизни ----------
    // В начале — крупнее, к концу — поменьше
    // life: 1.0 -> sizeFactor ~ 1.2
    // life: 0.0 -> sizeFactor ~ 0.6
    float sizeFactor = 0.6f + 0.6f * life;
    particle.size = particleSize * sizeFactor;

    // ---------- Цвет по жизни ----------
    // Сначала считаем "огненный" градиент по жизни
    QColor hotTop(255, 255, 0);  // жёлтый
    QColor hotMid(255, 140, 0);  // оранжевый
    QColor hotLow(200, 40, 0);   // тёмно-красный
    QColor smokeTop(80, 80, 80); // светлый дым
    QColor smokeLow(20, 20, 20); // тёмный дым

    QColor gradientColor;

    if (life > 0.7f) {
      // 0.7..1.0 : жёлтый ↔ оранжевый
      float k = (life - 0.7f) / 0.3f; // 0..1
      gradientColor = lerpColor(hotMid, hotTop, k);
    } else if (life > 0.3f) {
      // 0.3..0.7 : оранжевый ↔ тёмно-красный
      float k = (life - 0.3f) / 0.4f; // 0..1
      gradientColor = lerpColor(hotLow, hotMid, k);
    } else {
      // 0.0..0.3 : тёмный красный ↔ дым
      float k = life / 0.3f; // 0..1
      QColor from = smokeLow;
      QColor to = smokeTop;
      gradientColor = lerpColor(from, to, k);
    }

    // --- Подмешиваем baseColor из UI ---
    // 0.0 -> чисто baseColor, 1.0 -> чисто "огненный" градиент
    QColor userBase = baseColor;
    userBase.setAlphaF(1.0f);

    float mix = 0.7f; // 70% огненного градиента, 30% пользовательского цвета
    QColor color = lerpColor(userBase, gradientColor, mix);

    // Яркость отдельно можно не трогать — alpha = life учитывается при рендере
    particle.color = color;
  }

  // Удаление "мёртвых" частиц (происходит очень редко)
  if (std::rand() % 1000 ==
      0) { // Случайная проверка чтобы не делать это каждый кадр
    particles.erase(
        std::remove_if(particles.begin(), particles.end(),
                       [](const Particle &p) { return !p.isAlive(); }),
        particles.end());
  }
}

void ParticleSystem::setParameters(int maxParticles, float particleSize,
                                   float particleSpeed,
                                   const QColor &baseColor) {
  this->maxParticles = maxParticles;
  this->particleSize = particleSize;
  this->particleSpeed = particleSpeed;
  this->baseColor = baseColor;
  particles.reserve(maxParticles);
}

void ParticleSystem::emitParticles(int count) {
  const float emitterRadius = 0.4f; // радиус площадки огня

  for (int i = 0;
       i < count && particles.size() < static_cast<size_t>(maxParticles); ++i) {
    Particle p;

    // Случайная точка в круге (XZ), ближе к центру чаще
    float u = static_cast<float>(std::rand()) / RAND_MAX;
    float v = static_cast<float>(std::rand()) / RAND_MAX;
    float r = emitterRadius * std::sqrt(u);
    float angle2D = 2.0f * M_PI * v;

    float offsetX = r * std::cos(angle2D);
    float offsetZ = r * std::sin(angle2D);
    float offsetY = (std::rand() % 100 - 50) / 500.0f; // лёгкий шум по высоте

    p.position = position + Vector3(offsetX, offsetY, offsetZ);

    // Случайное направление (преимущественно вверх) — можно оставить как было
    float angle = (std::rand() % 360) * M_PI / 180.0f;
    float speed = 0.5f + (std::rand() % 100) / 200.0f;

    p.velocity =
        Vector3(std::cos(angle) * 0.3f, speed * 1.5f, std::sin(angle) * 0.3f);

    p.life = 0.8f + (std::rand() % 100) / 500.0f;
    p.size = particleSize;
    p.color = baseColor;

    particles.push_back(p);
  }
}
