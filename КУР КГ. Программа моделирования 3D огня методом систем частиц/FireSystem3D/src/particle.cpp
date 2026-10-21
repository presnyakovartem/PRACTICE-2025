#include "particle.h"

Particle::Particle()
    : position(0.0f, 0.0f, 0.0f), velocity(0.0f, 0.0f, 0.0f), life(0.0f),
      size(1.0f), color(Qt::white) {}
