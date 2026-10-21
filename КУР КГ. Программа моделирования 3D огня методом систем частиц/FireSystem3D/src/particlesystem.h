#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

#include "particle.h"
#include <vector>

class ParticleSystem {
public:
  ParticleSystem();
  ParticleSystem(const Vector3 &position); // Конструктор с позицией
  ~ParticleSystem();

  void update(float deltaTime);
  void setParameters(int maxParticles, float particleSize, float particleSpeed,
                     const QColor &baseColor);
  void emitParticles(int count);
  void setPosition(const Vector3 &position) { this->position = position; }
  Vector3 getPosition() const { return position; }

  // Геттеры для параметров
  int getMaxParticles() const { return maxParticles; }
  float getParticleSize() const { return particleSize; }
  float getParticleSpeed() const { return particleSpeed; }
  QColor getBaseColor() const { return baseColor; }
  int getActiveParticles() const { return static_cast<int>(particles.size()); }

  const std::vector<Particle> &getParticles() const { return particles; }

private:
  std::vector<Particle> particles;
  int maxParticles;
  float particleSize;
  float particleSpeed;
  QColor baseColor;
  float emissionRate;
  float timeSinceLastEmission;
  Vector3 position; // Позиция системы в мире
};

#endif // PARTICLESYSTEM_H
