#ifndef LIGHT_H
#define LIGHT_H

#include "vector3.h"
#include <QColor>

class Light {
public:
  Light();

  Vector3 position;
  QColor color;
  float intensity;
};

#endif // LIGHT_H
