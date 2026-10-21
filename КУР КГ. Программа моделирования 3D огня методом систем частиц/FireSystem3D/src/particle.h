#ifndef PARTICLE_H
#define PARTICLE_H

#include "vector3.h"
#include <QColor>

class Particle {
public:
  Particle();

  Vector3 position;
  Vector3 velocity;
  float life;   // Оставшееся время жизни (0.0 - 1.0)
  float size;   // Размер частицы
  QColor color; // Цвет частицы

  bool isAlive() const { return life > 0.0f; }
};

#endif // PARTICLE_H
