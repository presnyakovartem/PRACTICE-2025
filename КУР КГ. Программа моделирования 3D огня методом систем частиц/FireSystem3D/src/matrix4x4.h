#ifndef MATRIX4X4_H
#define MATRIX4X4_H

#include "vector3.h"
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class Matrix4x4 {
public:
  float m[4][4];

  Matrix4x4() { identity(); }

  // Единичная матрица
  void identity() {
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
        m[i][j] = (i == j) ? 1.0f : 0.0f;
      }
    }
  }

  // Умножение матриц
  Matrix4x4 operator*(const Matrix4x4 &other) const {
    Matrix4x4 result;
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
        result.m[i][j] = 0;
        for (int k = 0; k < 4; k++) {
          result.m[i][j] += m[i][k] * other.m[k][j];
        }
      }
    }
    return result;
  }

  Matrix4x4 inverted() const {
    Matrix4x4 result;
    float a[4][8];

    // Формируем расширенную матрицу [M | I]
    for (int i = 0; i < 4; ++i) {
      for (int j = 0; j < 4; ++j) {
        a[i][j] = m[i][j];
        a[i][j + 4] = (i == j) ? 1.0f : 0.0f;
      }
    }

    // Прямой и обратный ход
    for (int col = 0; col < 4; ++col) {
      // Ищем строку с максимальным по модулю элементом в текущем столбце
      int pivotRow = col;
      float maxVal = std::fabs(a[col][col]);
      for (int row = col + 1; row < 4; ++row) {
        float val = std::fabs(a[row][col]);
        if (val > maxVal) {
          maxVal = val;
          pivotRow = row;
        }
      }

      // Если матрица вырожденная – вернём единичную, чтобы избежать NaN
      if (maxVal < 1e-8f) {
        Matrix4x4 id;
        id.identity();
        return id;
      }

      // Меняем местами текущую строку и строку-поворот (если нужно)
      if (pivotRow != col) {
        for (int j = 0; j < 8; ++j) {
          std::swap(a[col][j], a[pivotRow][j]);
        }
      }

      // Нормируем строку так, чтобы на диагонали стояла 1
      float pivot = a[col][col];
      for (int j = 0; j < 8; ++j) {
        a[col][j] /= pivot;
      }

      // Обнуляем остальные элементы в текущем столбце
      for (int row = 0; row < 4; ++row) {
        if (row == col)
          continue;
        float factor = a[row][col];
        for (int j = 0; j < 8; ++j) {
          a[row][j] -= factor * a[col][j];
        }
      }
    }

    // Правая часть теперь содержит M^{-1}
    for (int i = 0; i < 4; ++i) {
      for (int j = 0; j < 4; ++j) {
        result.m[i][j] = a[i][j + 4];
      }
    }

    return result;
  }

  // Умножение матрицы на вектор
  Vector3 operator*(const Vector3 &vec) const {
    float x = m[0][0] * vec.x + m[0][1] * vec.y + m[0][2] * vec.z + m[0][3];
    float y = m[1][0] * vec.x + m[1][1] * vec.y + m[1][2] * vec.z + m[1][3];
    float z = m[2][0] * vec.x + m[2][1] * vec.y + m[2][2] * vec.z + m[2][3];
    float w = m[3][0] * vec.x + m[3][1] * vec.y + m[3][2] * vec.z + m[3][3];

    if (w != 0.0f) {
      return Vector3(x / w, y / w, z / w);
    }
    return Vector3(x, y, z);
  }

  // Статические методы для создания матриц преобразований

  static Matrix4x4 translation(float x, float y, float z) {
    Matrix4x4 mat;
    mat.m[0][3] = x;
    mat.m[1][3] = y;
    mat.m[2][3] = z;
    return mat;
  }

  static Matrix4x4 scale(float sx, float sy, float sz) {
    Matrix4x4 mat;
    mat.m[0][0] = sx;
    mat.m[1][1] = sy;
    mat.m[2][2] = sz;
    return mat;
  }

  static Matrix4x4 rotationX(float angle) {
    float rad = angle * M_PI / 180.0f;
    float c = std::cos(rad);
    float s = std::sin(rad);

    Matrix4x4 mat;
    mat.m[1][1] = c;
    mat.m[1][2] = -s;
    mat.m[2][1] = s;
    mat.m[2][2] = c;
    return mat;
  }

  static Matrix4x4 rotationY(float angle) {
    float rad = angle * M_PI / 180.0f;
    float c = std::cos(rad);
    float s = std::sin(rad);

    Matrix4x4 mat;
    mat.m[0][0] = c;
    mat.m[0][2] = s;
    mat.m[2][0] = -s;
    mat.m[2][2] = c;
    return mat;
  }

  static Matrix4x4 rotationZ(float angle) {
    float rad = angle * M_PI / 180.0f;
    float c = std::cos(rad);
    float s = std::sin(rad);

    Matrix4x4 mat;
    mat.m[0][0] = c;
    mat.m[0][1] = -s;
    mat.m[1][0] = s;
    mat.m[1][1] = c;
    return mat;
  }

  // Матрица перспективной проекции
  static Matrix4x4 perspective(float fov, float aspect, float nearPlane,
                               float farPlane) {
    Matrix4x4 mat;
    float tanHalfFov = std::tan(fov * M_PI / 360.0f);
    float range = farPlane - nearPlane;

    mat.m[0][0] = 1.0f / (aspect * tanHalfFov);
    mat.m[1][1] = 1.0f / tanHalfFov;
    mat.m[2][2] = -(farPlane + nearPlane) / range;
    mat.m[2][3] = -2.0f * farPlane * nearPlane / range;
    mat.m[3][2] = -1.0f;
    mat.m[3][3] = 0.0f;

    return mat;
  }

  // Матрица ортографической проекции
  static Matrix4x4 orthographic(float left, float right, float bottom,
                                float top, float nearPlane, float farPlane) {
    Matrix4x4 mat;
    mat.m[0][0] = 2.0f / (right - left);
    mat.m[1][1] = 2.0f / (top - bottom);
    mat.m[2][2] = -2.0f / (farPlane - nearPlane);
    mat.m[0][3] = -(right + left) / (right - left);
    mat.m[1][3] = -(top + bottom) / (top - bottom);
    mat.m[2][3] = -(farPlane + nearPlane) / (farPlane - nearPlane);
    return mat;
  }
};

#endif // MATRIX4X4_H
