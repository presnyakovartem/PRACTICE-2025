#include "camera.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Camera::Camera()
    : position(0, 0, 5) // Ближе к началу координат
      ,
      rotation(0, 0, 0), fov(60.0f), nearPlane(0.1f),
      farPlane(100.0f) // Уменьшим дальнюю плоскость
{}

Matrix4x4 Camera::getViewMatrix() const {
  // Сначала повороты, затем перемещение
  Matrix4x4 rotX = Matrix4x4::rotationX(rotation.x);
  Matrix4x4 rotY = Matrix4x4::rotationY(rotation.y);
  Matrix4x4 rotZ = Matrix4x4::rotationZ(rotation.z);
  Matrix4x4 translation =
      Matrix4x4::translation(-position.x, -position.y, -position.z);

  return translation * rotZ * rotY * rotX;
}

Matrix4x4 Camera::getProjectionMatrix(float aspectRatio) const {
  return Matrix4x4::perspective(fov, aspectRatio, nearPlane, farPlane);
}
