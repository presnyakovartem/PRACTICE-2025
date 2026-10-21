#ifndef SCENE_H
#define SCENE_H

#include "camera.h"
#include "light.h"
#include "particlesystem.h"
#include <vector>

class Scene {
public:
  Scene();
  ~Scene();

  void update(float deltaTime);
  void addParticleSystem();
  void addParticleSystem(const Vector3 &position);
  void removeParticleSystem(int index);

  std::vector<ParticleSystem> &getParticleSystems() { return particleSystems; }
  const std::vector<ParticleSystem> &getParticleSystems() const {
    return particleSystems;
  }
  Light &getLight() { return light; }
  Camera &getCamera() { return camera; }

private:
  std::vector<ParticleSystem> particleSystems;
  Light light;
  Camera camera;
};
#endif // SCENE_H
