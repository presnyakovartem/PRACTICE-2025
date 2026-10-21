#ifndef CAMERA_H
#define CAMERA_H

#include "matrix4x4.h"

class Camera {
public:
  Camera();

  Vector3 position;
  Vector3 rotation; // В градусах
  float fov;
  float nearPlane, farPlane;

  // Получить видовую матрицу
  Matrix4x4 getViewMatrix() const;

  // Получить матрицу проекции
  Matrix4x4 getProjectionMatrix(float aspectRatio) const;
};

#endif // CAMERA_H
